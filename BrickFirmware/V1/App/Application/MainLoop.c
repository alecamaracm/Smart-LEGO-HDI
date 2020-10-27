
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
#include "Drivers/outputShiftDriver.h"

#include <ti/display/Display.h>

#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/PIN.h>
#include <stdint.h>

#include "PIN_HELPER.h"

#include "DataStreamerService.h"
#include "Drivers/outputShiftDriver.h"
#include "Drivers/inputShiftDriver.h"
#include "GLOBAL_DEFINES.h"



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
  taskParams.priority = DEFAULT_TASK_PRIORITY;

  Task_construct(&spTaskML, MainLoop_taskFxn, &taskParams, NULL);
}


static void MainLoop_taskFxn()
{
     while(true){
   //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
       /*  if(PIN_getInputValue(PIN_BUTTON)==0){

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

         }*/
       //  outputShiftSend();

       //  counter*=2;
       //  if(counter==0){
       //      counter=1;
       //  }
       //  LoadOutputBufferByte(0,0xFF);
       //  outputShiftSend();
        // EnableOutputLEDs();

   //      inputShiftLoad();
    //     uint8_t data=ReadInputBufferByte(0);
    //     Display_printf(dispHandle, 1, 0,"Loaded data: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(data));

         Task_sleep(500*100);
     }
}


