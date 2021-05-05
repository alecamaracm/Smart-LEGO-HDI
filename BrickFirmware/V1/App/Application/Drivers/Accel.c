/*
 * Accel.c
 *
 *  Created on: Apr 20, 2021
 *      Author: william
 */

#include "Accel.h"
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include "WS2812Driver.h"
#include "GLOBAL_DEFINES.h"
#include "util.h"


#define ACCEL_ADDRESS 0x12
#define ACCEL_CHIPID 0xE7
#define ACCEL_MODE_REGISTER_ON 0xC0
#define ACCEL_INTEN2_REGISTER_VALUE 0x1F
#define ACCEL_MOTCONFIG0_REGISTER_VALUE 0x02
#define ACCEL_MOTCONFIG2_REGISTER_VALUE 0x07

#define HAL_SYSTEM_RESET() *((uint32_t *)0x40082270) = 1;


#define REGISTER_CHIP_ID 0x00
#define REGISTER_ACCEL_DATA 0x01
#define REGISTER_ACCEL_MODE 0x11
#define REGISTER_ACCEL_INTEN2 0x18
#define REGISTER_ACCEL_MOTCONFIG0 0x2c
#define REGISTER_ACCEL_MOTCONFIG2 0x2e
#define REGISTER_ACCEL_INTSTATE 0x09

struct ACCEL_DATA_STRUCT{
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
} accelData;

I2CCC26XX_Object i2cCC26xxObjects[1];

const I2CCC26XX_HWAttrsV1 i2cCC26xxHWAttrs[1] = {
                                                 {
                                                  .baseAddr    = I2C0_BASE,
                                                  .powerMngrId = 7,
                                                  .intNum      = INT_I2C_IRQ,
                                                  .intPriority = ~0,
                                                  .swiPriority = 0,
                                                  .sdaPin      = PIN_SDA,
                                                  .sclPin      = PIN_SCL,
                                                 }
};

const I2C_Config I2C_config[1] = {
                                  {
                                   .fxnTablePtr = &I2CCC26XX_fxnTable,
                                   .object      = &i2cCC26xxObjects[0],
                                   .hwAttrs     = &i2cCC26xxHWAttrs[0]
                                  },
};

I2C_Handle i2cHandle;
I2C_Params i2cParams;


uint32_t sleepInactivityCounter=0;
bool flag=false;

bool AccelMovementPresent(){
    uint8_t interruptState=0;
    uint8_t registerNumber=REGISTER_ACCEL_INTSTATE;
    I2C_Transaction i2cTransaction;
    i2cTransaction.slaveAddress = ACCEL_ADDRESS;
    i2cTransaction.writeBuf = &registerNumber;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = &interruptState;
    i2cTransaction.readCount = 1;
    bool status = I2C_transfer(i2cHandle, &i2cTransaction);
    if(status==true){
        interruptState &= 0x07;
        return interruptState>0; //There is movement
    }else{
        Display_printf(dispHandle, 1, 0, "I2C error on DoAccelWork!");
        return false;

    }
}

void ResetSleepCounter(){
    sleepInactivityCounter=0;
}

void DoAccelWork(){

    sleepInactivityCounter++;

    if(AccelMovementPresent()){
        sleepInactivityCounter=0;
    }else{
        if(sleepInactivityCounter>INACTIVITY_SLEEP_TIME_MS/ACCEL_UPDATE_INTERVAL_MS){
            Display_printf(dispHandle, 1, 0, "No movement, starting sleep...");
            TurnAllOff();
            Task_sleep(500*100);
            HAL_SYSTEM_RESET();
        }
    }
}

bool IsAccelPresent(){
    uint8_t registerNumber=REGISTER_CHIP_ID;
    uint8_t dataRead;
    I2C_Transaction i2cTransaction;
    i2cTransaction.slaveAddress = ACCEL_ADDRESS;
    i2cTransaction.writeBuf = &registerNumber;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = &dataRead;
    i2cTransaction.readCount = 1;
    bool status = I2C_transfer(i2cHandle, &i2cTransaction);
    if(status==true){
        if(dataRead==ACCEL_CHIPID)
        {
            return true;
        }else{
            Display_printf(dispHandle, 1, 0, "I2C Accelerometer returned the wrong chip ID! %d",dataRead);
            return false;
        }
    }else{
        Display_printf(dispHandle, 1, 0, "I2C error on IsAccelPresent!");
        return false;
    }
}


bool WriteToRegister(uint8_t registerNumber, uint8_t data){
    uint8_t writeBuffer[2]={registerNumber,data};
    I2C_Transaction i2cTransaction;
    i2cTransaction.slaveAddress = ACCEL_ADDRESS;
    i2cTransaction.writeBuf = writeBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;
    return I2C_transfer(i2cHandle, &i2cTransaction);
}


bool SetupRegisters(){
    if(!WriteToRegister(REGISTER_ACCEL_MODE,ACCEL_MODE_REGISTER_ON)){
        Display_printf(dispHandle, 1, 0, "I2C error on SetupRegisters_MODE!");
        return false;
    }

    if(!WriteToRegister(REGISTER_ACCEL_INTEN2,ACCEL_INTEN2_REGISTER_VALUE)){
        Display_printf(dispHandle, 1, 0, "I2C error on SetupRegisters_INTEN2!");
        return false;
    }

    if(!WriteToRegister(REGISTER_ACCEL_MOTCONFIG0,ACCEL_MOTCONFIG0_REGISTER_VALUE)){
        Display_printf(dispHandle, 1, 0, "I2C error on SetupRegisters_MOTCONFIG0!");
        return false;
    }

    if(!WriteToRegister(REGISTER_ACCEL_MOTCONFIG2,ACCEL_MOTCONFIG2_REGISTER_VALUE)){
        Display_printf(dispHandle, 1, 0, "I2C error on SetupRegisters_MOTCONFIG2!");
        return false;
    }
    return true;
}



void DoAccelSetup(){
    Display_printf(dispHandle, 1, 0, "Initializing KB accelerometer!");
    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cParams.transferCallbackFxn = NULL;
    i2cParams.bitRate = I2C_400kHz;

    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        // Error opening I2C
        Display_printf(dispHandle, 1, 0, "I2C open error!");
    }
    Display_printf(dispHandle, 1, 0, "I2C driver initialized! GOFYKB");

    if(!IsAccelPresent()) return;
    if(!SetupRegisters()) return;

    Display_printf(dispHandle, 1, 0, "Accelerometer initialized successfully!");

}

int I2C_count=1;
