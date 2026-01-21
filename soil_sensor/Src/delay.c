/*
 * delay.c
 *
 *  Created on: Nov 27, 2025
 *      Author: sunbeam
 */


#include "delay.h"

void DelayMs(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 4000; i++) {
        __asm("nop");
    }
}
