
/*
 * IR_TX.c
 *
 *  Created on: Sep 30, 2020
 *      Author: aleca
 */

#include <MainLoop.h>
#include "util.h"

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

#include "PIN_HELPER.h"

#include "DataStreamerService.h"
#include "Drivers/outputShiftDriver.h"
#include "GLOBAL_DEFINES.h"


static void IR_TX();



Task_Struct spTaskIR_TX;
#pragma DATA_ALIGN(spTaskStackIR_TX, 8)
uint8_t spTaskStackIR_TX[512];

uint8_t studsSendingByteBuffer[NUMBER_OF_TRANSMITTING_STUDS];
uint8_t studsCRCByteBuffer[NUMBER_OF_TRANSMITTING_STUDS];


void IR_TX_createTask()
{

  Task_Params taskParams;

   //Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackIR_TX;
  taskParams.stackSize = 512;
  taskParams.priority = TRANSCEIVER_TASK_PRIORITY;


  Task_construct(&spTaskIR_TX, IR_TX, &taskParams, NULL);
}

uint8_t SendShared(uint8_t toSend);
void SendIndividually(uint8_t * sutdValueBuffer,uint8_t * studCRCs);


static void IR_TX()
{

     while(true){
    //     Display_printf(dispHandle, 1, 0, "Starting to send IR datass");
//#ifdef FULL_DEBUG
        // Display_printf(dispHandle, 1, 0, "Starting to send IR data");
//#endif

         //Set TX LEDs to 0 and enable them
         for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
             LoadOutputBufferByte(k,0x00);
         }
         outputShiftSend();
         EnableOutputLEDs();

         for(int i=0;i<NUMBER_OF_TRANSMITTING_STUDS;i++){
             studsCRCByteBuffer[i]=0;
         }

         uint8_t tempCRC=studsCRCByteBuffer[0];

         //Packet start
         tempCRC+=SendShared(MSG_HEADER);
         tempCRC+=SendShared(MSG_TYPE_BRICKID<<4 | MSG_RESERVED);

         //Data bytes
         for(int i=0;i<6;i++){
             tempCRC+=SendShared(currentBrickID[i]);
         }
         //Data bytes
         int brickTypeToSend=BRICK_TYPE;
         for(int i=0;i<3;i++){
             tempCRC+=SendShared((brickTypeToSend >> ((2-i)*8)) & 0xFF);
         }

         for(int i=0;i<NUMBER_OF_TRANSMITTING_STUDS;i++){
              studsCRCByteBuffer[i]=tempCRC;
         }

         for(int i=0;i<3;i++){
             for(int k=0;k<NUMBER_OF_TRANSMITTING_STUDS;k++){
                studsSendingByteBuffer[k]=(k>>((2-i)*8));
             }

             SendIndividually(studsSendingByteBuffer, studsCRCByteBuffer);
         }

         SendIndividually(studsCRCByteBuffer, NULL);


         for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
              LoadOutputBufferByte(k,0xFF);
          }
          outputShiftSend();
          Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);
          for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
              LoadOutputBufferByte(k,0x00);
          }
          outputShiftSend();
          DisableOutputLEDs();

#ifdef FULL_DEBUG
         Display_printf(dispHandle, 1, 0, "Finished to send IR data");
#endif

         Task_sleep(TX_INTERVAL_MS*100);
     }
}

uint8_t SendShared(uint8_t toSend){
    for(int i=0;i<8;i++){
        uint8_t val=(toSend >> (7-i)) & 0x01;

        //Sending all ones
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            LoadOutputBufferByte(k,0xFF);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);

        //Sending all zeroes
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            LoadOutputBufferByte(k, (val==0) ? 0x00 : 0xFF);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);

        //Sending all zeroes
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            LoadOutputBufferByte(k,0x00);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);
    }


    //Long delay after a byte if you want to analyze this in a logic analyzer
    //Task_sleep(50 * 100);
    return toSend;
}

void SendIndividually(uint8_t * sutdValueBuffer,uint8_t * studCRCs){
    for(int i=0;i<8;i++){
        //Sending all ones
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            LoadOutputBufferByte(k,0xFF);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);


        //Send the data for each stud at this current bit
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            uint8_t byteToSend=0;
            for(int j=0;j<8;j++){
                //Check for studs that we don't have
                if((k*8)+j >= NUMBER_OF_TRANSMITTING_STUDS) continue;

                uint8_t studCurrentValue=(sutdValueBuffer[(k*8)+j] >> (7-i)) & 0x01;
                byteToSend|=(studCurrentValue<<(7-j));

            }
            LoadOutputBufferByte(k, byteToSend);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);

        //Sending all zeroes
        for(int k=0;k<NUMBER_OF_IC_OUTPUT;k++){
            LoadOutputBufferByte(k,0x00);
        }
        outputShiftSend();
        Task_sleep(TX_DEBOUNCE_COUNT * RXTX_TICK_PERIOD);
    }

    if(studCRCs!=NULL){
        for(int k=0;k<NUMBER_OF_TRANSMITTING_STUDS;k++){
            studCRCs[k]+=sutdValueBuffer[k];
        }
    }

    //Long delay after a byte if you want to analyze this in a logic analyzer
    //Task_sleep(80 * 100);
}


