/*
 * WS2812Driver.cpp
 *
 *  Created on: Sep 30, 2020
 *      Author: aleca
 */

#include "Drivers/WS2812Driver.h"
#include <ti/display/Display.h>
#include "util.h"

#include <stdint.h>
#include <stddef.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>



#include <ti/sysbios/hal/Hwi.h>
#include "PIN_HELPER.h"

#include <ti/drivers/PIN.h>
#include <stdint.h>
#include "Application/Drivers/DO_NOT_TOUCH.h"
#include "GLOBAL_DEFINES.h"

#include "Transceivers/IR_RX.h"

#include "Accel.h"


void SendWSUpdate();
void RGB_LEDs_setAll(uint8_t r,uint8_t g,uint8_t b);
void UpdateCurrentRGBAnimation();
void UpdateCurrentRGBState();

typedef enum RGB_LEDS_STATE {
    OFF,
    CHARGING,
    CHARGED,
    BRICK_CONNECTED,
    PARTY,
} RGB_LEDS_STATE;

uint8_t RGB_LEDS_ANIMS_INTERVAL_MULTIPLER[] = {
    1,
    1,
    1,
    5,
};





Task_Struct spTaskWS;
#pragma DATA_ALIGN(spTaskStackWS, 8)
uint8_t spTaskStackWS[512];




//Internal use only, do NOT write there
uint8_t WSsendBuffer[(LED_NUMBER*3)];
//Use this buffer, in format GRBGRB,...
uint8_t WSOutputData[(LED_NUMBER*3)];



int animationMultiplierCount=1000;
#define RGB_STATUS_SELECT_INTERVAL_MULTIPLIER 10
int rgbStatusIntervalMultiplierCount=1000;

bool fadeUp=true;

RGB_LEDS_STATE rgbState=PARTY;
float brigthness=0.8f;



void WS2812Driver_createTask()
{


  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = spTaskStackWS;
  taskParams.stackSize = 512;
  taskParams.priority = DEFAULT_TASK_PRIORITY;

  Task_construct(&spTaskWS, WS2812Driver_taskFxn, &taskParams, NULL);
}


int kimball=0;

static void WS2812Driver_taskFxn()
{


     PIN_setOutputEnable(&hStateHui, PIN_WS2812, 1);
     PIN_setOutputValue(&hStateHui, PIN_WS2812, 0);

     RGB_LEDs_setAll(0, 0, 0);

     Display_printf(dispHandle, 1, 0, "WS2812 driver initialization finished!");

     while(true){

         //Check if the current animation mode is correct
         if(rgbStatusIntervalMultiplierCount>RGB_STATUS_SELECT_INTERVAL_MULTIPLIER)
         {
             rgbStatusIntervalMultiplierCount=0;
             UpdateCurrentRGBState();
         }
         rgbStatusIntervalMultiplierCount++;

         //Update the current animation
         if(animationMultiplierCount>RGB_LEDS_ANIMS_INTERVAL_MULTIPLER[rgbState]){
             animationMultiplierCount=0;
             UpdateCurrentRGBAnimation();

             //Display_printf(dispHandle, 1, 0, "Val1: %d",PIN_getInputValue(PIN_CHARGING));
           //  Display_printf(dispHandle, 1, 0, "Val2: %d",PIN_getInputValue(PIN_CHARGED));
         }
         animationMultiplierCount++;

         //Send the update to the LEDs and sleep
         SendWSUpdate();

        // DoAccelWork();

         Task_sleep(RGB_LED_ANIMATION_INTERVAL_MS*100);

     }
}

void UpdateCurrentRGBState(){
    switch(rgbState)
    {
    case OFF:
        rgbState=PARTY;
        break;
    case PARTY:
    {
        if(PIN_getInputValue(PIN_CHARGING)==0){
            Display_printf(dispHandle, 1, 0, "Charging started!");
            rgbState=CHARGING;
        }else if(PIN_getInputValue(PIN_CHARGED)==0){
            Display_printf(dispHandle, 1, 0, "Charging complete!");
            rgbState=CHARGED;
        }
        if(IR_RX_isSomethingConnected) {
            rgbState=BRICK_CONNECTED;
        }
        break;
    }
    case CHARGING:
        if(PIN_getInputValue(PIN_CHARGING)==1){
            Display_printf(dispHandle, 1, 0, "Charging stopped!");
            rgbState=OFF;
        }
        if(IR_RX_isSomethingConnected) {
                    rgbState=BRICK_CONNECTED;
        }
        break;
    case CHARGED:
        if(PIN_getInputValue(PIN_CHARGED)==1){
            Display_printf(dispHandle, 1, 0, "Charging stopped!");
            rgbState=OFF;
        }
        if(IR_RX_isSomethingConnected) {
            rgbState=BRICK_CONNECTED;
        }
        break;
    case BRICK_CONNECTED:
        if(!IR_RX_isSomethingConnected) {
             rgbState=OFF;
        }
        break;
    default:
        break;
    }



}

void UpdateCurrentRGBAnimation()
{
    switch(rgbState){
    case OFF:
        brigthness=0;
        RGB_LEDs_setAll(0,0,0);
        break;
    case CHARGING:
        RGB_LEDs_setAll(30,0,0);
        break;
    case CHARGED:
        RGB_LEDs_setAll(0,30,0);
        break;
    case PARTY:
    {
        /*brigthness=0.1f;

        bool empty=true;
        for(int i=0;i<LED_NUMBER*3;i++)
        {
            if(WSOutputData[i]!=0) empty=false;
        }
        if(empty) WSOutputData[0]=0xFF;

        uint8_t temp=WSOutputData[0];
        for(int i=0;i<LED_NUMBER*3-1;i++)
        {
            WSOutputData[i]= WSOutputData[i+1];
        }
        WSOutputData[LED_NUMBER*3-1]=temp;*/
        RGB_LEDs_setAll(20,20,0);
        brigthness=0.5f;
        break;
    }
    case BRICK_CONNECTED:
    {
        RGB_LEDs_setAll(0,0,150);
        break;
    }

    default:
         break;
    }

    //Apply breath effect
    if(rgbState==CHARGING || rgbState==CHARGED)
    {
        if(fadeUp)
        {
            brigthness+=0.008f;
            if(brigthness>0.15f) fadeUp=false;
        }else{
            brigthness-=0.008f;
            if(brigthness<0.025f) fadeUp=true;
        }
    }

    if(rgbState==BRICK_CONNECTED)
    {
        if(fadeUp)
        {
            brigthness+=0.015f;
            if(brigthness>0.80f) fadeUp=false;
        }else{
            brigthness-=0.015f;
            if(brigthness<0.025f) fadeUp=true;
        }
    }
}

void SendWSUpdate()
{
    for(int i=0;i<(LED_NUMBER*3);i++){
        WSsendBuffer[i]=(uint8_t)(WSOutputData[i]*brigthness);
    }

    Hwi_disable();

    int doSpecial=1;

    int k;
    for(int i=0;i<(LED_NUMBER*3);i++){
        for( k=7;k>=0;k--){ //Shift out byte
             WriteWSValue((WSsendBuffer[i]>>k) & 0x01,&doSpecial);
        }
    }

    Hwi_enable();
}

void RGB_LEDs_setAll(uint8_t r,uint8_t g,uint8_t b)
{
    for(int i=0;i<(LED_NUMBER*3);i+=3){
        WSOutputData[i]=g;
        WSOutputData[i+1]=r;
        WSOutputData[i+2]=b;
    }
}


void TurnAllOff(){
    RGB_LEDs_setAll(0,0,0);
    SendWSUpdate();
}
