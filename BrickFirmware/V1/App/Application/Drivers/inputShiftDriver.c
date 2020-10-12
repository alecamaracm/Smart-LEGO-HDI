/*
 * inputshiftdriver.c
 *
 *  Created on: Oct 4, 2020
 *      Author: william
 */

#include <inputShiftDriver.h>
#include <ti/display/Display.h>
#include "PIN_HELPER.h"
#include "PIN_HELPER.h"
#include <ti/drivers/PIN.h>
#include <stdint.h>
#include "util.h"
#define NUMBER_OF_IC_INPUT 1

uint8_t inputShiftData[NUMBER_OF_IC_INPUT];


void inputShiftLoad(){
    Display_printf(dispHandle, 1, 0, "Reading data to input register in inputShiftDriver ");
    PIN_setOutputEnable(&hStateHui, PIN_I_CLK, 1);
    PIN_setOutputEnable(&hStateHui, PIN_I_LOAD, 1);
    PIN_setOutputValue(&hStateHui, PIN_I_CLK, 0);
    PIN_setOutputValue(&hStateHui, PIN_I_LOAD, 1);

    //Load the data into the shift registers
    PIN_setOutputValue(&hStateHui, PIN_I_LOAD, 0);
    PIN_setOutputValue(&hStateHui, PIN_I_CLK, 1);
    PIN_setOutputValue(&hStateHui, PIN_I_LOAD, 1);
    PIN_setOutputValue(&hStateHui, PIN_I_CLK, 0);

    for(int i=0;i<NUMBER_OF_IC_INPUT ;i++)
    {
        inputShiftData[i]=0;
        // Send the first bit
        for(int j=0; j<8; j++)
        {
            inputShiftData[i]|=(PIN_getInputValue(PIN_I_DT) & 0x01)<<j;

            PIN_setOutputValue(&hStateHui, PIN_I_CLK, 1);
            PIN_setOutputValue(&hStateHui, PIN_I_CLK, 0);
        }

    }

}


uint8_t ReadInputBufferByte(int byteIndex){
    if(byteIndex < sizeof(inputShiftData)){
        return inputShiftData[byteIndex];
    }else{
        Display_printf(dispHandle, 1, 0, "Out of bounds input buffer read!");
        return 0;
    }
}
