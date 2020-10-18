
/*
 * outputshiftdriver.c
 *
 *  Created on: Oct 4, 2020
 *      Author: william
 */

#include "Drivers/outputShiftDriver.h"
#include <ti/display/Display.h>
#include "PIN_HELPER.h"
#include "PIN_HELPER.h"
#include <ti/drivers/PIN.h>
#include <stdint.h>
#include "util.h"
#include "GLOBAL_DEFINES.h"

uint8_t outputShiftData[NUMBER_OF_IC_OUTPUT];


void outputShiftSend(){
#ifdef FULL_DEBUG
    Display_printf(dispHandle, 1, 0, "Sending data to output register in outputShiftDriver");
#endif

    PIN_setOutputEnable(&hStateHui, PIN_O_LATCH, 1);
    PIN_setOutputEnable(&hStateHui, PIN_O_CLK, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_DT, 0);
    PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 0);
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
        }

    }
    PIN_setOutputValue(&hStateHui, PIN_O_DT, 0);
    PIN_setOutputValue(&hStateHui, PIN_O_CLK, 0);
    PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_LATCH, 0);

}


void LoadOutputBufferByte(int byteIndex,uint8_t value){
    if(byteIndex < sizeof(outputShiftData)){
        outputShiftData[byteIndex]=value;
    }else{
        Display_printf(dispHandle, 1, 0, "Out of bounds output buffer write!");
    }
}

void EnableOutputLEDs(){
    PIN_setOutputEnable(&hStateHui, PIN_O_EN, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_EN, 0);
}
void DisableOutputLEDs(){
    PIN_setOutputEnable(&hStateHui, PIN_O_EN, 1);
    PIN_setOutputValue(&hStateHui, PIN_O_EN, 1);
}
