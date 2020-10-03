/*
 * DO_NOT_TOUCH.h
 *
 *  Created on: Sep 30, 2020
 *      Author: william
 */

#ifndef PROFILES_DO_NOT_TOUCH_H_
#define PROFILES_DO_NOT_TOUCH_H_

#define ASM_NOP asm(" NOP ")

inline void WSCpuShortDelay0First(void) {
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
}
inline void WSCpuShortDelay0Last(void) {
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
}


inline void WSCpuShortDelay1First(void) {
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
    ASM_NOP;
}
inline void WSCpuShortDelay1Last(void) {
    ASM_NOP;
}


#define REGISTER_WS2812 0x40022007

volatile void WSToggleHigh(){
    *(uint32_t*)((void*)REGISTER_WS2812) |= 0x01;
}

volatile inline void WSInlineToggleHigh(){
    *(uint32_t*)((void*)REGISTER_WS2812) |= 0x01;
}


volatile void WSToggleLow(){
    *(uint32_t*)((void*)REGISTER_WS2812) &= ~0x01;
}

volatile inline void WSInlineToggleLow(){
    *(uint32_t*)((void*)REGISTER_WS2812) &= ~0x01;
}


volatile inline void WriteWSValue(int value,int * fix){
    if(value==0){
        WSInlineToggleHigh();
        if(!*fix)
        {
            ASM_NOP;
            ASM_NOP;
        }
        WSCpuShortDelay0First();
        WSInlineToggleLow();
        if(!*fix)
        {
            ASM_NOP;
            ASM_NOP;
            ASM_NOP;
        }
        WSCpuShortDelay0Last();
        if(*fix) *fix=false;
    }else {
        WSInlineToggleHigh();
        if(!*fix)
        {
            ASM_NOP;
            ASM_NOP;
            ASM_NOP;
            ASM_NOP;
            ASM_NOP;
        }
        WSCpuShortDelay1First();
        if(!*fix)
        {
            ASM_NOP;
        }
        if(*fix) *fix=false;
        WSInlineToggleLow();
    }
}




#endif /* PROFILES_DO_NOT_TOUCH_H_ */
