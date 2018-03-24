/* 
 * File:   interrupt.c
 * Author: MATHRAX
 *
 * Created on March 23, 2018, 12:03 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include "main.h"

/*
 * 
 */
unsigned int count;

void interrupt InterTimer(void) {
    if (TMR2IF == 1) {
        count++;
        TMR2IF = 0;
        if (count >= 10) {
            count = 0;
            if (wdtOnOff == 1) {
                CLRWDT();
            }
        }
    }
}

