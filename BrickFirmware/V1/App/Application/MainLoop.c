
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

#include <ti/display/Display.h>

#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/PIN.h>
#include <stdint.h>

#include "PIN_HELPER.h"

#include "DataStreamerService.h"

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

         }
         Task_sleep(1000*100);
     }
}


