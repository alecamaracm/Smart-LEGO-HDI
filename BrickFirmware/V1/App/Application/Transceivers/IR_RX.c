
/*
 * IR_RX.c
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
#include "Drivers/inputShiftDriver.h"
#include "GLOBAL_DEFINES.h"


static void IR_RX();





Task_Struct spTaskIR_RX;
#pragma DATA_ALIGN(spTaskStackIR_RX, 8)
uint8_t spTaskStackIR_RX[1024];


void IR_RX_createTask()
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackIR_RX;
  taskParams.stackSize = 1024;
  taskParams.priority = TRANSCEIVER_TASK_PRIORITY;

  Task_construct(&spTaskIR_RX, IR_RX, &taskParams, NULL);
}


static void IR_RX()
{
     while(true){
   //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
         Task_sleep(500*100);
     }
}


