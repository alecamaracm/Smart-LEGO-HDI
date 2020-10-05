/*
 * outputshiftdriver.c
 *
 *  Created on: Oct 4, 2020
 *      Author: william
 */

#include <outputShiftDriver.h>
#include <ti/display/Display.h>
#include "PIN_HELPER.h"
#include "PIN_HELPER.h"
#include <ti/drivers/PIN.h>
#include <stdint.h>
#include "util.h"
#define NUMBER_OF_IC_OUTPUT 1;

uint8_t outputShiftData[1]={15};






void outputShiftSend(){
    Display_printf(dispHandle, 1, 0, "Sending data to output register in outputShiftDriver ");
    PIN_setOutputEnable(&hStateHui, PIN_O_LATCH, 1);
    PIN_setOutputEnable(&hStateHui, PIN_O_EN, 1);
    PIN_setOutputEnable(&hStateHui, PIN_O_CLK, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_DT, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_EN, 0);
    int numberOfIC=NUMBER_OF_IC_OUTPUT;
    for(int i=0;i< numberOfIC ;i++)
    {
        // Send the first bit
        for(int j=0; j<8; j++)
        {
            PIN_setOutputValue(&hStateHui, PIN_O_CLK, 0);
            if(outputShiftData[i] >>j  & 0x01)
            {
                PIN_setOutputValue(&hStateHui, PIN_O_DT, 1);
            } else
            {
                PIN_setOutputValue(&hStateHui, PIN_O_DT, 0);

            }
            PIN_setOutputValue(&hStateHui, PIN_O_CLK, 1);
            PIN_setOutputValue(&hStateHui, PIN_O_CLK, 0);
            PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 1);
            PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 0);

        }

    }
    PIN_setOutputValue(&hStateHui, PIN_O_DT, 0);
    PIN_setOutputValue(&hStateHui, PIN_O_CLK, 0);
    PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 0);

}
