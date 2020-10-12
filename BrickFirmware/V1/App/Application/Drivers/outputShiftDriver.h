/*
 * outputshiftdriver.h
 *
 *  Created on: Oct 4, 2020
 *      Author: william
 */

#ifndef APPLICATION_OUTPUTSHIFTDRIVER_H_
#define APPLICATION_OUTPUTSHIFTDRIVER_H_

#include <stdint.h>

extern void outputShiftSend();
extern void LoadOutputBufferByte(int byteIndex,uint8_t value);
extern void EnableOutputLEDs();
extern void DisableOutputLEDs();

#endif /* APPLICATION_OUTPUTSHIFTDRIVER_H_ */

