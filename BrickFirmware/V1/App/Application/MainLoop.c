
/*
 * MainLoopr.cpp
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
#include <outputShiftDriver.h>

#include <ti/display/Display.h>

#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/PIN.h>
#include <stdint.h>

#include "PIN_HELPER.h"

#include "DataStreamerService.h"
#include <outputShiftDriver.h>
#include "inputShiftDriver.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

static void MainLoop_taskFxn();





Task_Struct spTaskML;
#pragma DATA_ALIGN(spTaskStackML, 8)
uint8_t spTaskStackML[1024];

uint8_t counter=1;

void MainLoop_createTask()
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackML;
  taskParams.stackSize = 1024;
  taskParams.priority = 3;

  Task_construct(&spTaskML, MainLoop_taskFxn, &taskParams, NULL);
}


static void MainLoop_taskFxn()
{
     while(true){
   //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
         if(PIN_getInputValue(PIN_BUTTON)==0){

             Display_printf(dispHandle, 1, 0, "Button pushed!");
             SetNewDataStreamBegin();

             for(int i=0;i<counter;i++){
                 dataStreamOutputBuffer[i]=counter;
             }
             dataStreamCurrentLength=counter;
             counter++;

             SetNewDataStreamEnd();

             do{
                 Task_sleep(500*100);
             }while(PIN_getInputValue(PIN_BUTTON)==0);

         }
         outputShiftSend();

         counter*=2;
         if(counter==0){
             counter=1;
         }
         LoadOutputBufferByte(0,0xFF);
         EnableOutputLEDs();

         inputShiftLoad();
         uint8_t data=ReadInputBufferByte(0);
         Display_printf(dispHandle, 1, 0,"Loaded data: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(data));

         Task_sleep(500*100);
     }
}


