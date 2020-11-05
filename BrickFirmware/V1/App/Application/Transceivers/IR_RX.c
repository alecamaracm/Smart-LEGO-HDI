
/*
 * IR_RX.c
 *
 *  Created on: Sep 30, 2020
 *      Author: aleca
 */

#include <MainLoop.h>
#include "util.h"

#include "Transceivers/IR_RX.h"

#include <stdint.h>
#include <stddef.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>

#include <ti/display/Display.h>

#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/PIN.h>
#include <stdint.h>
#include <xdc/runtime/Error.h>

#include "PIN_HELPER.h"

#include "DataStreamerService.h"
#include "Drivers/inputShiftDriver.h"
#include "GLOBAL_DEFINES.h"
#include <ti/sysbios/knl/Clock.h>

static void IR_RX();


//Bit decoder
#define RX_BIT_STATE_IDLE 0
#define RX_BIT_STATE_HIGH 1
#define RX_BIT_STATE_LOW 2
#define RX_BIT_STATE_END 3

struct IR_RX_stud_state{
    //BIT DECODER
    short debounceCount;
    uint8_t decoderState;
    short ticksHigh;
    short ticksLow;

    //BYTE PACKER
    uint8_t currentByte;
    uint8_t currentBitPosition;

    //STREAM DECODER
    uint8_t buffer[MSG_BUFFER_SIZE];
    uint8_t currentBytePosition;
    uint8_t lastBytePosition;

    //CVS
    bool newDataReady;
    bool disconnectedEvent;
    bool bleChange;

    //STUD CONNECTION DATA
    uint8_t connectedBrickID[6];
    uint8_t connectedBrickStudID[3];
    uint8_t connectedBrickType[3];
    int disconnectionTimeout;
};
struct IR_RX_stud_state studStates[NUMBER_OF_RECEIVING_STUDS];


Task_Struct spTaskIR_RX;
#pragma DATA_ALIGN(spTaskStackIR_RX, 8)
uint8_t spTaskStackIR_RX[512];

bool IR_RX_isSomethingConnected=false;

static Clock_Struct periodicClock;

void IR_RX_createTask()
{
    for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++)
    {
        studStates[i].decoderState=RX_BIT_STATE_IDLE;
        studStates[i].ticksHigh=0;
        studStates[i].ticksLow=0;
        studStates[i].debounceCount=0;
        studStates[i].currentBitPosition=0;
        studStates[i].currentByte=0;
        studStates[i].currentBytePosition=0;
        studStates[i].disconnectionTimeout=0;
        studStates[i].newDataReady=false;
        studStates[i].disconnectedEvent=false;
        studStates[i].bleChange=false;
    }


    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = spTaskStackIR_RX;
    taskParams.stackSize = 512;
    taskParams.priority = TRANSCEIVER_TASK_PRIORITY;
    Task_construct(&spTaskIR_RX, IR_RX_Task, &taskParams, NULL);




    Clock_Params clockParams;
    Clock_Params_init(&clockParams);
    clockParams.period = RXTX_TICK_PERIOD;
    clockParams.startFlag = TRUE;
    clockParams.arg = 'B';
    Clock_create(IR_RX,RXTX_TICK_PERIOD*2 , &clockParams, Error_IGNORE);

}

void IR_RX_Task()
{
    while(true) {

        bool somethingChanged=false;
        bool thereIsBLEPendingData=false;
        // Do message processing outside the critical section
        for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++){
            if(studStates[i].newDataReady) {
                if(MessageReceived(i)) {
                    //If something changed, recalculate lights later and set the ble delta ready flag
                    somethingChanged=true;
                    studStates[i].bleChange=true;
                }
                studStates[i].newDataReady=false;
            }
            if(studStates[i].disconnectedEvent){
                Display_printf(dispHandle, 1, 0,"Stud %d disconnected!",i);
                studStates[i].disconnectedEvent=false;
                somethingChanged=true;
                studStates[i].bleChange=true;
            }
            if(studStates[i].bleChange)
            {
                thereIsBLEPendingData=true;
            }
        }

        if(somethingChanged){
            bool somethingConnected=false;
            for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++){
                if(studStates[i].disconnectionTimeout > 0) {
                    somethingConnected = true;
                    break;
                }
            }
            IR_RX_isSomethingConnected=somethingConnected;
        }

        if(thereIsBLEPendingData && currentBrickIDSet){
            if(dataStreamReady==false) //We are not advertising or sending advertisements
            {
                //No need to call SetNewDataStreamBegin to disable ads bc they are already disabled (streamDataReady=false)
                if(CreateBLEStream()){
                    SetNewDataStreamEnd(); //Enable ads
                    Display_printf(dispHandle, 1, 0,"New BLE stream packet activated from the RX loop.");
                }else{
                    //Should never get here
                    Display_printf(dispHandle, 1, 0,"There is pending data but CreateBLEStream did not return true!");
                }
            }else{
                Display_printf(dispHandle, 1, 0,"New BLE stream deltas are ready but there is a pending stream!");
            }
        }

        Task_sleep(500*100);
    }
}

/**
 * RX clock interrupt, do all necessary non blocking tasks here
 */
void IR_RX_DoWork()
{
    inputShiftLoad(); //Load current RX stud data into buffer.
    uint8_t data=ReadInputBufferByte(0);


    for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++){
        struct IR_RX_stud_state  * currentStud=&studStates[i];

        //Prevents reading new data if the last message still needs to be processed
       // if(currentStud->newDataReady) continue;

        if(currentStud->disconnectionTimeout==1){
            currentStud->disconnectionTimeout=0;
            currentStud->disconnectedEvent=true;
        }else {
            if(currentStud->disconnectionTimeout>1)currentStud->disconnectionTimeout--;
        }

        uint8_t studValue=(ReadInputBufferByte(i/8)>>(7-i)) & 0x01;



        if((studValue==1 && (currentStud->decoderState==RX_BIT_STATE_IDLE || currentStud->decoderState==RX_BIT_STATE_LOW))
                || (studValue==0 && (currentStud->decoderState==RX_BIT_STATE_HIGH || currentStud->decoderState==RX_BIT_STATE_END))){
            //If it is in the right state and value, increase count
            currentStud->debounceCount++;
        }else{ //Something is wrong, just reset the counter
            currentStud->debounceCount=0;
        }

        switch(studStates[i].decoderState){
        case RX_BIT_STATE_IDLE:
        {
            if(currentStud->debounceCount>RX_DEBOUNCE_COUNT) //Already stabilized, signal start
            {
                currentStud->decoderState=RX_BIT_STATE_HIGH;
                currentStud->ticksHigh=currentStud->debounceCount; //Add the debounced ticks to the high counter
                currentStud->debounceCount=0;
            }
            break;
        }

        case RX_BIT_STATE_HIGH:
        {
            currentStud->ticksHigh+=1;
            if(studStates[i].debounceCount>RX_DEBOUNCE_COUNT) //Change to low detected
            {
                currentStud->decoderState=RX_BIT_STATE_LOW;
                currentStud->ticksHigh-=currentStud->debounceCount;
                currentStud->ticksLow=currentStud->debounceCount; //Add the debounced ticks to the low counter
                currentStud->debounceCount=0;
            }

            break;
        }
        case RX_BIT_STATE_LOW:
        {
            currentStud->ticksLow+=1;
            if(currentStud->ticksLow>(currentStud->ticksHigh)*RX_MAXIMUM_HIGHLOW_TIMES_DIFFERENCE_VALID){
                currentStud->newDataReady=true;
                currentStud->decoderState=RX_BIT_STATE_IDLE;
                currentStud->currentBitPosition=0;
                currentStud->debounceCount=0;
                currentStud->currentByte=0;
                currentStud->lastBytePosition=currentStud->currentBytePosition;
                currentStud->currentBytePosition=0;
                break;
            }

            if(currentStud->debounceCount>RX_DEBOUNCE_COUNT) //Change to high detected, end of the bit
            {
                currentStud->decoderState=RX_BIT_STATE_IDLE;
                currentStud->ticksLow-=currentStud->debounceCount; //Add the debounced ticks to the low counter

                //BIT DETECTED, TRY TO PACK AND SEND BYTE
                uint8_t valueDetected=currentStud->ticksHigh > studStates[i].ticksLow;
                currentStud->ticksHigh=0;
                currentStud->ticksLow=0;


                currentStud->currentByte|=valueDetected<<(7-currentStud->currentBitPosition);
                if(currentStud->currentBitPosition>=7){ //Just wrote the last bit
                    //DO SOMETHING WITH THE NEW BYTE
                    currentStud->currentBitPosition=0;

                    if(currentStud->currentBytePosition < MSG_BUFFER_SIZE) {
                        currentStud->buffer[currentStud->currentBytePosition]=currentStud->currentByte;
                        currentStud->currentBytePosition++; //Clean byte after we are done
                    }

                    currentStud->currentByte=0; //Clean byte after we are done
                }else{
                    currentStud->currentBitPosition++;
                }
                //currentStud->debounceCount=0;
            }
            break;
        }
        }
    }

   // PIN_setOutputValue(&hStateHui, PIN_BUTTON, 0);
}

//Returns true if something changed
bool Message_BRICKID_Received(int studIndex, uint8_t * startOfData, uint8_t length){

    struct IR_RX_stud_state  * currentStud=&studStates[studIndex];
    //  Display_printf(dispHandle, 1, 0, "BrickID message received at stud %d",studIndex);

    if(length!=12) { //Check for the message length
        Display_printf(dispHandle, 1, 0, "Wrong BRICK_ID message length %d",studIndex);
        return false;
    }

    int changed=memcmp(currentStud->connectedBrickID,startOfData,6);
    int changed2=memcmp(currentStud->connectedBrickStudID,startOfData+9,3);
    //There is no point in checking the birck type since 2 bricks can't have the same id

    if(changed!=0 || changed2!=0 || currentStud->disconnectionTimeout==0) {



#ifdef RXTX_DEBUG

        long newID=0;
        int newBrickType=0, newStudID=0;

        for(int i=0;i<6;i++){
            newID|=startOfData[i]<< ((5-i)*8);
        }

        newBrickType=(startOfData[6]<<16) + (startOfData[6+1]<<8)+(startOfData[6+2]);
        newStudID=(startOfData[9]<<16) + (startOfData[9+1]<<8)+(startOfData[9+2]);

        if(currentStud->disconnectionTimeout==0){ // Brick just placed
            Display_printf(dispHandle, 1, 0, "A new wild brick just appeared at stud %d. ID=%u, Type=%d, stud=%d",studIndex,newID,newBrickType,newStudID);
        }else{ //Old brick replaced with new one without disconnecting
            Display_printf(dispHandle, 1, 0, "A new wild brick just flash replaced the old one at stud %d. ID=%u, Type=%d, stud=%d",studIndex,newID,newBrickType,newStudID);
        }
#endif

        //Actually copy the values becasue they have weird lengths
        memcpy(currentStud->connectedBrickID,startOfData,6);
        memcpy(currentStud->connectedBrickType,startOfData+6,3);
        memcpy(currentStud->connectedBrickStudID,startOfData+9,3);

        currentStud->disconnectionTimeout=TICKS_TO_TIMEOUT;
        return true;
    }else{
        currentStud->disconnectionTimeout=TICKS_TO_TIMEOUT;
        return false;
    }


}

//Returns true if something changed
bool MessageReceived(int studIndex){
    struct IR_RX_stud_state  * currentStud=&studStates[studIndex];

    //Check header
    if(currentStud->buffer[0]!=MSG_HEADER) {
        // Display_printf(dispHandle, 1, 0, "Wrong message header at stud %d",studIndex);
        return false;
    }

    //Check the checksum
    uint8_t myChecksum = 0;
    for(int i=0;i<currentStud->lastBytePosition - 1;i++){
        myChecksum+=currentStud->buffer[i];
    }
    uint8_t gotChecksum=currentStud->buffer[currentStud->lastBytePosition-1];
    if(myChecksum!=gotChecksum){
        //  Display_printf(dispHandle, 1, 0, "Wrong checksum at stud %d. Calculated: %d, Received: %d",studIndex,myChecksum,gotChecksum);
        return false;
    }

    //Check the reserved bits
    if(currentStud->buffer[1] & 0x0F != MSG_RESERVED){
        // Display_printf(dispHandle, 1, 0, "Wrong reserved part of byte 1 at stud %d",studIndex);
        return false;
    }

    //Call the right helper method depending on the message type
    uint8_t messageType=(currentStud->buffer[1] >> 4);
    switch(messageType){
    case MSG_TYPE_BRICKID:
        return Message_BRICKID_Received(studIndex, currentStud->buffer+2, currentStud->lastBytePosition-3);
        break;
    default:
        //Display_printf(dispHandle, 1, 0, "Unknown message type at stud %d",studIndex);
        break;
    }

}

static void IR_RX(UArg arg1)
{

    IR_RX_DoWork();
    //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
    //Task_sleep(RXTX_TICK_PERIOD);
    //       Clock_tickPeriod

}

bool CreateBLEStream(){

    dataStreamCurrentLength=0;

    for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++){

        if(studStates[i].bleChange==true)
        {
            //This is not completely efficient in terms of data per BLE stream, but it might help save CPU cycles with a long # of studs.

            if(studStates[i].disconnectionTimeout==0) {

                //If the current packet won't fit in the stream anymore, just break and don´t send any more that
                if(dataStreamCurrentLength+4 >=DATASTREAM_MAX_LENGTH) break;


                //Stud is disconnected
                dataStreamOutputBuffer[dataStreamCurrentLength]=BLE_DELTA_TYPE_BASIC_DISCONNECTED;

               //Local stud
               for(int k=0;k<3;k++) {
                   dataStreamOutputBuffer[dataStreamCurrentLength+1+k]=(uint8_t)(i>>((2-k)*8));
               }

               dataStreamCurrentLength+=4;
            }else{

                //If the current packet won't fit in the stream anymore, just break and don´t send any more that
                if(dataStreamCurrentLength+13 >=DATASTREAM_MAX_LENGTH) break;

                //Stud connnected or changed
                dataStreamOutputBuffer[dataStreamCurrentLength]=BLE_DELTA_TYPE_BASIC_CONNECTED;

               //Remote MAC
               memcpy(dataStreamOutputBuffer+dataStreamCurrentLength+1, studStates[i].connectedBrickID, 6);

               //Local stud
               for(int k=0;k<3;k++) {
                   dataStreamOutputBuffer[dataStreamCurrentLength+1+6+k]=(uint8_t)(i>>((2-k)*8));
               }

               //Remote stud
               memcpy(dataStreamOutputBuffer+dataStreamCurrentLength+1+6+3, studStates[i].connectedBrickStudID, 3);

               dataStreamCurrentLength+=13;
            }

            studStates[i].bleChange=false; //We processed this stud change
        }
    }

    return dataStreamCurrentLength>0;

}


