
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
uint8_t spTaskStackIR_TX[1024];


void IR_TX_createTask()
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackIR_TX;
  taskParams.stackSize = 1024;
  taskParams.priority = TRANSCEIVER_TASK_PRIORITY;

  Task_construct(&spTaskIR_TX, IR_TX, &taskParams, NULL);
}


static void IR_TX()
{
     while(true){
   //      Display_printf(dispHandle, 1, 0, "Button status: %d",PIN_getInputValue(PIN_BUTTON));
         Task_sleep(500*100);
     }
}


