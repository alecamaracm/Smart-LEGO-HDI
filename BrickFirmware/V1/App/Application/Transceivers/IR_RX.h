/*
 * IR_RX.h
 *
 *  Created on: Oct 2, 2020
 *      Author: aleca
 */

#ifndef IR_RX_H_
#define IR_RX_H_

void IR_RX_createTask();
extern void IR_RX_DoWork();
void MessageReceived(int studIndex);
void IR_RX_Task();
extern bool IR_RX_isSomethingConnected;

#endif /* APPLICATION_DRIVERS_MAINLOOP_H_ */
