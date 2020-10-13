/*
 * IR_RX.h
 *
 *  Created on: Oct 2, 2020
 *      Author: aleca
 */

#ifndef IR_RX_H_
#define IR_RX_H_

void IR_RX_createTask();
static void IR_RX_DoClockWork(UArg arg);
void NewByteReceived(int studIndex);

#endif /* APPLICATION_DRIVERS_MAINLOOP_H_ */
