/*
 * WS2812Driver.cpp
 *
 *  Created on: Sep 30, 2020
 *      Author: aleca
 */

#include <Drivers/WS2812Driver.h>
#include "util.h"

#include <stdint.h>
#include <stddef.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>

#include <ti/display/Display.h>

Task_Struct spTaskWS;

#pragma DATA_ALIGN(spTaskStackWS, 8)
uint8_t spTaskStackWS[1024];

void WS2812Driver_createTask()
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackWS;
  taskParams.stackSize = 1024;
  taskParams.priority = 3;

  Task_construct(&spTaskWS, WS2812Driver_taskFxn, &taskParams, NULL);
}


static void WS2812Driver_taskFxn()
{
    while(true){
        Display_printf(dispHandle, 1, 0, "WS2812Task");
        Task_sleep(350*100);
    }
}
