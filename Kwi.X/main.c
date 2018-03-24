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
#pragma config BORV = 1        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
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
unsigned long timeCount = 0;
unsigned int fadeoutCount;
float ave1, ave2;
float dif;
float baseLine;
float lastDif;
float aveDif;
unsigned char toggle;

float aveCount;

int toggleCount1;
int toggleCount2;
int toggleTrue;
unsigned int mp3Busy;
unsigned long mp3ResetCount;
unsigned int rcvData;
unsigned int rcvBuf[20];
unsigned int rcvPos;

unsigned int wdtOnOff = 1;
unsigned int timeOut;

void main(void) {
    setUp();

    LATAbits.LATA4 = 0;
    __delay_ms(1000);
    LATAbits.LATA4 = 1;
    __delay_ms(500);
    
    setSerialDFMp3();
    //MP3RESET
    mp3_send_cmd(0x0C, 0, 0);
    __delay_ms(1000);
    __delay_ms(1000);
    
    //PLAY '01/001.mp3'
    mp3_send_cmd(0x0F, 1, 1);
    //Play
    mp3_send_cmd(0x0D, 0, 0);
    //Repeat Play
    mp3_send_cmd(0x11, 0, 0);

    wdtOnOff = 0;
    do {
        __delay_ms(1);
        mp3Busy = PORTCbits.RC5;
        serialDebug();
    } while (mp3Busy != 0);
    wdtOnOff = 1;

    for (unsigned int i = 0; i < 20; i++) {
        //Volume 0
        mp3_send_cmd(6, 0, i);
        serialDebug();
        __delay_ms(50);
    }
    for (unsigned int i = 0; i < 20; i++) {
        //Volume 0
        mp3_send_cmd(6, 0, 20 - i);
        serialDebug();
        __delay_ms(50);
    }

    //Pause
    mp3_send_cmd(0x0E, 0, 0);
    __delay_ms(50);

    status = 0;
    vol = 1;
    ave1 = 0;
    ave2 = 0;
    baseLine = 0;

    
    while (1) {

        aveCount = 2.0;

        ledOn();
        __delay_ms(5);
        num1Buf[index] = (adconv(5) * adconv(5)) / 10;
        index = (index + 1) % BUFFER_LENGTH;
        num1 = (median(num1Buf, BUFFER_LENGTH));
        ave1 = ave1 * ((aveCount - 1.0) / aveCount) + (float) (num1) * (1.0 / aveCount);

        ledOff();
        __delay_ms(5);
        num2Buf[index] = (adconv(5));
        num2 = (median(num2Buf, BUFFER_LENGTH));
        ave2 = ave2 * ((aveCount - 1.0) / aveCount) + (float) (num2) * (1.0 / aveCount);


        aveCount = 2.0;
        //Light > IR
        if ((ave1 > ave2)) {
            baseLine = baseLine * ((aveCount - 1.0) / aveCount) + (ave1 - ave2) * (1.0 / aveCount); // + (ave2) * (5.0 / aveCount);

        } else if ((ave2 > ave1)) {
            baseLine *= 0.5; //baseLine * ((aveCount - 1.0) / aveCount) + (0) * (1.0 / aveCount); // + (ave2) * (5.0 / aveCount);
        }

        int hystheresis = 50;

        if (baseLine > (100 + hystheresis)) {
            toggle = 1;
        } else if (baseLine < (100 - hystheresis)) {
            toggle = 0;
        }


        mp3Busy = PORTCbits.RC5;
        serialDebug();

        //Toggle DeBounce
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
                mp3ResetCount = 0;

                mp3Busy = PORTCbits.RC5;
                //play
                if (mp3Busy == 1) {
                    //PLAY '01/001.mp3'
                    mp3_send_cmd(0x0F, 1, 1);
                    //Play
                    mp3_send_cmd(0x0D, 0, 0);
                }
            }

            //20sec count
            if (status == 1) {
                fadeoutCount = 0;
                if (mp3Busy == 0) {
                    mp3ResetCount = 0;
                }

                if (timeCount < 160) {

                    timeCount++;

                    mp3Busy = PORTCbits.RC5;
                    //loop
                    if (mp3Busy == 1) {
                        //PLAY '01/001.mp3'
                        mp3_send_cmd(0x0F, 1, 1);
                        //Play
                        mp3_send_cmd(0x0D, 0, 0);
                    }
                    if (mp3ResetCount < 50) {
                        mp3ResetCount++;
                    } else {
                        wdtOnOff = 0;
                    }

                    //Fadein
                    vol += (MAX_VOL / 4);
                    if (vol > MAX_VOL) {
                        vol = MAX_VOL;
                    }
                    //Volume Fadein
                    mp3_send_cmd(6, 0, (unsigned int)vol);

                } else {
                    //Fadeout
                    vol -= 2;

                    if (vol < 1) {
                        vol = 1;
                        //Pause
                        mp3_send_cmd(0x0E, 0, 0);
                    } else {
                        //Volume Fadeout
                        mp3_send_cmd(6, 0, (unsigned int)vol);

                        if (mp3ResetCount < 50) {
                            mp3ResetCount++;
                        }
                    }
                }
            }

        } else {

            //Fadeout
            vol -= 2;

            if (fadeoutCount > 10) {
                timeCount = 0;
                mp3ResetCount = 0;
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
                mp3_send_cmd(6, 0, (unsigned int)vol);
            }


        }

    }
    return;
}