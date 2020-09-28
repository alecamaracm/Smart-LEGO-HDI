/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* Example/Board Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

extern void *echoThread(void *arg0);
extern void *blinkThread(void *arg0);


void initializeGPIOS();

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t           threadEcho,threadBlink;
    pthread_attr_t      attrsEcho,attrsBlink;
    struct sched_param  priParamEcho, priParamBlink;
    int                 retc;


    initializeGPIOS();

    pthread_attr_init(&attrsEcho);
    priParamEcho.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrsEcho, &priParamEcho);
    retc |= pthread_attr_setdetachstate(&attrsEcho, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrsEcho, 1024);
    retc = pthread_create(&threadEcho, &attrsEcho, echoThread, NULL);

    pthread_attr_init(&attrsBlink);
    priParamBlink.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrsBlink, &priParamBlink);
    retc |= pthread_attr_setdetachstate(&attrsBlink, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrsBlink, 1024);
    retc = pthread_create(&threadBlink, &attrsBlink, blinkThread, NULL);


    /*if (retc != 0) {
        // pthread_create() failed
        while (1) {}
    }*/

    BIOS_start();

    return (0);
}

void initializeGPIOS(){
    /* Call driver init functions */
    Board_init();

    /* Call driver init functions */
    GPIO_init();
    UART_init();

}
