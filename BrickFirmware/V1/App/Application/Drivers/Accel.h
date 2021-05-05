/*
 * Accel.h
 *
 *  Created on: Apr 20, 2021
 *      Author: william
 */

#ifndef APPLICATION_DRIVERS_ACCEL_H_
#define APPLICATION_DRIVERS_ACCEL_H_

#include "PIN_HELPER.h"

/*
 *  =============================== I2C ===============================
*/
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>

#include <ti/sysbios/knl/Task.h>


void DoAccelWork();
void DoAccelSetup();

bool AccelMovementPresent();
void ResetSleepCounter();

#endif /* APPLICATION_DRIVERS_ACCEL_H_ */
