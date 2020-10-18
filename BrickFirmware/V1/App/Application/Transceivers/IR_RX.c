
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



//Message decoder
#define RX_STATE_IDLE 0


//Bit decoder
#define RX_BIT_STATE_IDLE 0
#define RX_BIT_STATE_HIGH 1
#define RX_BIT_STATE_LOW 2
#define RX_BIT_STATE_END 3

struct IR_RX_stud_state{
    //BIT DECODER
    uint8_t debounceCount;
    uint8_t decoderState;
    uint8_t ticksHigh;
    uint8_t ticksLow;

    //BYTE PACKER
    uint8_t currentByte;
    uint8_t currentBitPosition;

    //STREAM DECODER
    uint8_t buffer[MSG_BUFFER_SIZE];
    uint8_t RXstate;
};
struct IR_RX_stud_state studStates[NUMBER_OF_RECEIVING_STUDS];


Task_Struct spTaskIR_RX;
#pragma DATA_ALIGN(spTaskStackIR_RX, 8)
uint8_t spTaskStackIR_RX[512];

Clock_Struct RXclock;


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
        studStates[i].RXstate=RX_STATE_IDLE;
    }


    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = spTaskStackIR_RX;
    taskParams.stackSize = 512;
    taskParams.priority = TRANSCEIVER_TASK_PRIORITY;

    Task_construct(&spTaskIR_RX, IR_RX, &taskParams, NULL);


    Clock_Params clockParams;
    Clock_Params_init(&clockParams);
    clockParams.period = RXTX_TICK_PERIOD;        // 0 is the default, but added for completeness
    clockParams.startFlag = TRUE;
    clockParams.arg = 'A';
    Clock_create(IR_RX_DoClockWork, RXTX_TICK_PERIOD, &clockParams, Error_IGNORE);

}

/**
 * RX clock interrupt, do all necessary non blocking tasks here
 */
void IR_RX_DoClockWork(UArg arg)
{
    inputShiftLoad(); //Load current RX stud data into buffer
    for(int i=0;i<NUMBER_OF_RECEIVING_STUDS;i++){

        struct IR_RX_stud_state  * currentStud=&studStates[i];

        uint8_t studValue=(ReadInputBufferByte(NUMBER_OF_RECEIVING_STUDS>>8)>>(7-i)) & 0x01;

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
            if(currentStud->debounceCount>RX_DEBOUNCE_COUNT) //Change to high detected, end of the bit
            {
                currentStud->decoderState=RX_BIT_STATE_END;
                currentStud->ticksLow-=currentStud->debounceCount; //Add the debounced ticks to the low counter

                //BIT DETECTED, TRY TO PACK AND SEND BYTE
                uint8_t valueDetected=currentStud->ticksHigh > studStates[i].ticksLow;
#ifdef FULL_DEBUG
                Display_printf(dispHandle, 1, 0, "Got new RX bit at stud: %d. Value: %d",i,valueDetected);
#endif
                currentStud->currentByte|=valueDetected<<(7-currentStud->currentBitPosition);
                if(currentStud->currentBitPosition>=7){ //Just wrote the last bit
                    //DO SOMETHING WITH THE NEW BYTE
#ifdef FULL_DEBUG
                Display_printf(dispHandle, 1, 0, "Got new RX byte at stud: %d. Value: %d",i,studStates[i].currentByte);
#endif
                    NewByteReceived(i);
                    currentStud->currentBitPosition=0;
                    currentStud->currentByte=0; //Clean byte after we are done
                }else{
                    currentStud->currentBitPosition++;
                }
                currentStud->debounceCount=0;
            }
            break;
        }
        case RX_BIT_STATE_END:
        {
            if(currentStud->debounceCount>RX_DEBOUNCE_COUNT) //Change to low after bit sent detected
            {
                currentStud->decoderState=RX_BIT_STATE_IDLE;
                currentStud->debounceCount=0;
            }
            break;
        }

        }
    }
}

void NewByteReceived(int studIndex){

}

static void IR_RX()
{
     while(true){
     //    IR_RX_DoWork();
   //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
         Task_sleep(500*100);
  //       Clock_tickPeriod
     }
}


