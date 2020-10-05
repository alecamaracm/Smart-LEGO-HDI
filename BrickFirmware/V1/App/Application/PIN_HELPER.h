/*
 * PIN_HELPER.h
 *
 *  Created on: Oct 2, 2020
 *      Author: aleca
 */

#ifndef APPLICATION_PIN_HELPER_H_
#define APPLICATION_PIN_HELPER_H_

#include <ti/drivers/PIN.h>

#define PIN_WS2812 7
#define PIN_CHARGING 4
#define PIN_CHARGED 3
#define PIN_BUTTON 2
#define  PIN_O_DT 10
#define  PIN_O_CLK 9
#define  PIN_O_LATCH 8
#define PIN_O_EN 6

extern PIN_State  hStateHui;



void InitializePins();


#endif /* APPLICATION_PIN_HELPER_H_ */
