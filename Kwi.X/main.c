/*
 * File:   main.c
 * Author: shozo
 *
 * Created on February 5, 2018, 3:43 PM
 */

#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = ON       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config ZCDDIS = OFF     // Zero-cross detect disable (Zero-cross detect circuit is enabled at POR)
#pragma config PLLEN = ON       // Phase Lock Loop enable (4x PLL is always enabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = ON      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)


#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include "main.h"

unsigned long num1, num2;
unsigned long tmp1, tmp2;
unsigned long num1Buf[BUFFER_LENGTH];
unsigned long num2Buf[BUFFER_LENGTH];
unsigned int index = 0;

float d;
unsigned char status;
signed char vol = 0;
unsigned int timeCount = 0;
unsigned int fadeoutCount;
float ave1, ave2;
float dif;
float baseLine;
float lastDif;
float aveDif;
unsigned char toggle;

float aveCount;
#define __DEBUG__

int toggleCount1;
int toggleCount2;
int toggleTrue;

void main(void) {
    setUp();

    for (int i = 0; i < 10; i++) {
        ledOn();
        __delay_ms(50);
        ledOff();
        __delay_ms(50);
        CLRWDT();
    }
    __delay_ms(500);
    CLRWDT();

    setSerialDFMp3();
    //MP3RESET
    mp3_send_cmd(0x0C, 0, 0);
    __delay_ms(500);

    //PLAY '01/001.mp3'
    mp3_send_cmd(0x0F, 1, 1);
    //Repeat Play
    mp3_send_cmd(0x11, 0, 1);
    //Volume 10
    mp3_send_cmd(6, 0, 10);

    __delay_ms(500);

    //Volume 0
    mp3_send_cmd(6, 0, 1);

    //Pause
    mp3_send_cmd(0x0E, 0, 0);
    status = 0;
    vol = 1;


    while (1) {

        aveCount = 4.0;

        ledOn();
        __delay_ms(5);
        num1Buf[index] = (adconv(5) * adconv(5)) / 10; // * adconv(5) * adconv(5)) / 10;
        index = (index + 1) % BUFFER_LENGTH;
        num1 = (median(num1Buf, BUFFER_LENGTH));
        if (num1 > 3000)num1 = 3000;
        ave1 = ave1 * ((aveCount - 1.0) / aveCount) + (float) (num1) * (1.0 / aveCount);

        ledOff();
        __delay_ms(5);
        num2Buf[index] = (adconv(5));
        num2 = (median(num2Buf, BUFFER_LENGTH));
        ave2 = ave2 * ((aveCount - 1.0) / aveCount) + (float) (num2) * (1.0 / aveCount);


        aveCount = 40.0;
        baseLine = baseLine * ((aveCount - 1.0) / aveCount) + (ave1) * (1.0 / aveCount);
        if (baseLine > 500)baseLine = 500;
        if (baseLine > ave1) baseLine *= 0.9;


        if ((baseLine >= 500) && (ave1 > baseLine)) {
            if ((ave1 - baseLine) > 300) {
                toggle = 1;
            }
        } else if (baseLine < 500) {
            toggle = 0;
        }

        serialDebug();

        setSerialDFMp3();

        //Toggle Debounce
        if (toggle == 1) {
            toggleCount1++;
            if (toggleCount1 > 0) {
                toggleCount1 = 0;
                toggleTrue = 1;
            }
            toggleCount2 = 0;
        } else {
            toggleCount2++;
            if (toggleCount2 > 5) {
                toggleCount2 = 0;
                toggleTrue = 0;
                status = 0;
            }
            toggleCount1 = 0;
        }

        if (toggleTrue == 1) {
            if (status == 0) {
                status = 1;
                timeCount = 0;
                //PLAY '01/001.mp3'
                mp3_send_cmd(0x0F, 1, 1);
                //Repeat Play
                mp3_send_cmd(0x11, 0, 1);
                //Play
                mp3_send_cmd(0x0D, 0, 0);
            }

            //20sec count
            if (status == 1) {
                fadeoutCount = 0;

                if (timeCount < 2400) {
                    
                    timeCount++;

                    //Fadein
                    vol += 4;
                    if (vol > MAX_VOL) {
                        vol = MAX_VOL;
                    }
                    //Volume Fadein
                    mp3_send_cmd(6, 0, vol);

                } else {

                    //Fadeout
                    vol -= 2;

                    if (vol < 1) {
                        vol = 1;
                        //Pause
                        mp3_send_cmd(0x0E, 0, 0);
                    } else {
                        //Volume Fadeout
                        mp3_send_cmd(6, 0, vol);
                    }

                }
            }

        } else {

            //Fadeout
            vol -= 2;

            if (fadeoutCount > 10) {
                timeCount = 0;
            } else {
                fadeoutCount++;
            }

            if (vol < 1) {
                status = 0;
                vol = 1;
                //Pause
                mp3_send_cmd(0x0E, 0, 0);
            } else {
                //Volume Fadeout
                mp3_send_cmd(6, 0, vol);
            }

        }

    }


    return;
}