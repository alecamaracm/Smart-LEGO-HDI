/*
 * PIN_HELPER.c
 *
 *  Created on: Oct 2, 2020
 *      Author: aleca
 */
#include "PIN_HELPER.h"

#include "util.h"
#include <ti/display/Display.h>


PIN_State  hStateHui;

const PIN_Config GpioInitTable[] = {
         PIN_WS2812 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PULLDOWN | PIN_DRVSTR_MAX,
         PIN_CHARGING | PIN_GPIO_OUTPUT_DIS | PIN_PULLUP,
         PIN_CHARGED | PIN_GPIO_OUTPUT_DIS | PIN_PULLUP,
         PIN_BUTTON | PIN_GPIO_OUTPUT_DIS,
         PIN_O_DT | PIN_GPIO_OUTPUT_EN| PIN_GPIO_LOW | PIN_DRVSTR_MAX,
         PIN_O_CLK | PIN_GPIO_OUTPUT_EN| PIN_GPIO_LOW  | PIN_DRVSTR_MAX,
         PIN_O_LATCH | PIN_GPIO_OUTPUT_EN| PIN_GPIO_LOW  | PIN_DRVSTR_MAX,
         PIN_O_EN | PIN_GPIO_OUTPUT_EN| PIN_GPIO_LOW  | PIN_DRVSTR_MAX,
         PIN_TERMINATE
     };


void InitializePins(){
    PIN_init(GpioInitTable);

    if (!PIN_open(&hStateHui, GpioInitTable)) {
        // Create the display so that it can be used (If it is created before initializing the pins it doesnt work for some reason
        dispHandle = Display_open(Display_Type_ANY, NULL);
        Display_printf(dispHandle, 1, 0, "Failed to initialize pins!");
    }
}

