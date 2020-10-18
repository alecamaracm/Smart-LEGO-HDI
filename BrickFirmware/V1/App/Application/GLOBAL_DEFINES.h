/*
 * GLOBAL_DEFINES.h
 *
 *  Created on: Oct 3, 2020
 *      Author: aleca
 */

#ifndef APPLICATION_GLOBAL_DEFINES_H_
#define APPLICATION_GLOBAL_DEFINES_H_

//Macros
#define DIV_CEIL(n, div) (((n) + (div) - (1)) / (div))

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
//-----------------------------------


//Compile constants
#define EVENT_ENABLE_ADVERTISEMENT 69
#define EVENT_DISABLE_ADVERTISEMENT 70
#define EVENT_UPDATE_BRICKID 71
#define EVENT_READ_ID_TO_FLASH 72
#define EVENT_WRITE_ID_TO_FLASH 74

#define BRICK_ID_FLASH_NV_ID 0x81

#define BRICK_TYPE_4x2 3001
#define BRICK_TYPE_2x2 3003
#define BRICK_TYPE_4x4_CORNER 702
//-----------------------------------


//Brick settings
#include "BrickDefinitions/BRICK_4x2.h"
//-----------------------------------


//Task priorities
#define TRANSCEIVER_TASK_PRIORITY 4
#define DEFAULT_TASK_PRIORITY 3



//RX/TX settings
#define MSG_BUFFER_SIZE 16
#define RXTX_TICK_PERIOD 15

#define RX_DEBOUNCE_COUNT 4
#define TX_DEBOUNCE_COUNT RX_DEBOUNCE_COUNT * 2


#define MSG_HEADER 0x95
#define MSG_TYPE_BRICKID 0x1
#define MSG_RESERVED 0xA
//RX settings

//TX settings
#define TX_INTERVAL_MS 500


//Calculated constants
#define NUMBER_OF_IC_OUTPUT DIV_CEIL(NUMBER_OF_TRANSMITTING_STUDS,8)
#define NUMBER_OF_IC_INPUT DIV_CEIL(NUMBER_OF_RECEIVING_STUDS,8)
//----------------------------------


#endif /* APPLICATION_GLOBAL_DEFINES_H_ */
