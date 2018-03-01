/*
 * File:   main.c
 * Author: shozo
 *
 * Created on February 5, 2018, 3:43 PM
 */

#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
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
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)


#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include "main.h"

uint8_t send_buf[10] = {
    0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF
};
uint8_t recv_buf[10];

static void fill_uint16_bigend(uint8_t *thebuf, uint16_t data) {
    *thebuf = (uint8_t) (data >> 8);
    *(thebuf + 1) = (uint8_t) data;
}

//calc checksum (1~6 byte)

uint16_t mp3_get_checksum(uint8_t *thebuf) {
    uint16_t sum = 0;
    for (int i = 1; i < 7; i++) {
        sum += thebuf[i];
    }
    return -sum;
}
//fill checksum to send_buf (7~8 byte)

void mp3_fill_checksum() {
    uint16_t checksum = mp3_get_checksum(send_buf);
    fill_uint16_bigend(send_buf + 7, checksum);
}

void mp3_send_cmd(uint8_t cmd, uint16_t high_arg, uint16_t low_arg) {
    unsigned int i;

    send_buf[3] = cmd;

    send_buf[5] = high_arg;
    send_buf[6] = low_arg;

    mp3_fill_checksum();

    for (i = 0; i < 10; i++) {
        while (!TRMT);
        TXREG = send_buf[i];
    }

    __delay_ms(50);
}

// アナログ値の入力処理

unsigned int adconv(uint8_t ch) {
    unsigned int temp;
    ADCON0bits.CHS = ch;
    GO_nDONE = 1; // PICにアナログ値読取り開始を指示
    while (GO_nDONE); // PICが読取り完了するまで待つ
    temp = ADRESH; // PICは読取った値をADRESHとADRESLのレジスターにセットする
    temp = (temp << 8) | ADRESL; // 10ビットの分解能力
    __delay_us(100);
    return temp;
}

int abval(int val) {
    return (val < 0 ? (-val) : val);
}

unsigned int num1, num2;
unsigned char status;
signed char vol = 0;
unsigned int timeCount = 0;
unsigned int fadeoutCount;
//#define __DEBUG__

void main(void) {
    OSCCON = 0b01110000;

    // A/Dの設定
    ADCON1 = 0b10010000; // 読取値は右寄せ、A/D変換クロックはFOSC/8、VDDをリファレンスに
    ADCON0 = 0b00000001; // アナログ変換情報設定(AN6から読込む)
    __delay_us(5); // アナログ変換情報が設定されるまでとりあえず待つ

    TRISA = 0b00000000;
    TRISC = 0b00000000;
    ANSELA = 0b00000000;
    ANSELC = 0b00000000;

    //ANALOG
    //C1 AN5 -- RPR220
    TRISCbits.TRISC1 = 1;
    ANSELCbits.ANSC1 = 1;

    //IR-LED
    //A2 -- IR-LED
    TRISAbits.TRISA2 = 0;

#ifdef __DEBUG__
    TRISCbits.TRISC4 = 0;
    RC4PPS = 0b10100; // 出力(TXを割当てる)
#else
    //TX C3,RX C0
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC0 = 1;
    RC3PPS = 0b10100; // 出力(TXを割当てる)
    RXPPS = 0b10000; // 入力(RC0を割当てる:デフォルト)
#endif

    SYNC = 0;
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 51; //51;

    //TXSTA
    //CSRC , TX9 , TXEN , SYNC , SENDB , BRGH , TRMT , TX9D
    CSRC = 1;
    TX9 = 0;
    TXEN = 1;
    SYNC = 0;
    //SENDB = 
    BRGH = 0;
    //RCSTA
    //SPEN , RX9 , SREN , CREN , ADDEN , FERR , OERR , RX9D
    SPEN = 1;
    RX9 = 0;
    //SREN = 
    CREN = 1;
    ADDEN = 0;

    unsigned int i;
    for (i = 0; i < 10; i++) {
        LATAbits.LATA2 = 0;
        __delay_ms(50);
        LATAbits.LATA2 = 1;
        __delay_ms(50);
    }
    __delay_ms(500);
    //PLAY '01/001.mp3'
    mp3_send_cmd(0x0F, 1, 1);
    //Repeat Play
    mp3_send_cmd(0x11, 0, 1);

    //Volume 0
    mp3_send_cmd(6, 0, 1);

    //Pause
    mp3_send_cmd(0x0E, 0, 0);
    status = 0;
    vol = 1;

    while (1) {

        LATAbits.LATA2 = 0;
        num1 = adconv(5);
        __delay_ms(50);

        LATAbits.LATA2 = 1;
        num2 = adconv(5);
        __delay_ms(50);

#ifdef __DEBUG__
        //RPR220 on
        while (!TRMT);
        TXREG = 255;
        while (!TRMT);
        TXREG = abval(num1 >> 7);
        while (!TRMT);
        TXREG = abval(num1 & 0b01111111);

        while (!TRMT);
        TXREG = abval(num2 >> 7);
        while (!TRMT);
        TXREG = abval(num2 & 0b01111111);

        while (!TRMT);
        TXREG = abval(num1 - num2) >> 7;
        while (!TRMT);
        TXREG = abval(num1 - num2) & 0b01111111;

#else
        
        
        if (abval(num1) < 1000) {

            if (status == 0) {
                status = 1;
                timeCount = 0;
                //Play
                mp3_send_cmd(0x0D, 0, 0);
            }

            //20sec count
            if (status == 1) {
                fadeoutCount = 0;

                timeCount++;
                if (timeCount < 125) {

                    //Fadein
                    vol += 5;
                    if (vol > 30) {
                        vol = 30;
                    }
                    //Volume Fadein
                    mp3_send_cmd(6, 0, vol);

                } else {
                    timeCount = 200;

                    //Fadeout
                    vol -= 4;

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
                //                timeCount = 0;
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

#endif
    }


    return;
}
