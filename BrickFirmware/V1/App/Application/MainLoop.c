
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

#include "Application/Drivers/WS2812Driver.h"
#include "Application/MainLoop.h"
#include "Transceivers/IR_RX.h"
#include "simple_peripheral.h"
#include "Transceivers/IR_TX.h"

#include "Application/Drivers/Accel.h"



static void MainLoop_taskFxn();





Task_Struct spTaskML;
#pragma DATA_ALIGN(spTaskStackML, 8)
uint8_t spTaskStackML[512];

void MainLoop_createTask()
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackML;
  taskParams.stackSize = 512;
  taskParams.priority = DEFAULT_TASK_PRIORITY;

  Task_construct(&spTaskML, MainLoop_taskFxn, &taskParams, NULL);
}


static void MainLoop_taskFxn()
{
     while(true){

         TurnAllOff();
         Display_printf(dispHandle, 1, 0, "RESTARTED!");
         DoAccelSetup();
         while(!AccelMovementPresent()){
             Display_printf(dispHandle, 1, 0, "Accelerometer movement not detected, sleeping to save energy...");
             Task_sleep(ACCEL_WAKEUP_CHECK_INTERVAL_MS*100);
         }

         AccelMovementPresent();

         SimplePeripheral_createTask();
         WS2812Driver_createTask();

         IR_RX_createTask();
         IR_TX_createTask();

         ResetSleepCounter();

         while(true){
             DoAccelWork();
             Task_sleep(ACCEL_UPDATE_INTERVAL_MS*100);
         }

     }
}


