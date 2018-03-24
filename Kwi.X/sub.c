/* 
 * File:   sub.c
 * Author: MATHRAX
 *
 * Created on March 23, 2018, 10:35 AM
 */

#include <xc.h>
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 

// アナログ値の入力処理

unsigned int adconv(uint8_t ch) {
    unsigned int temp;
    ADCON0bits.CHS = ch;
    __delay_us(500);
    GO_nDONE = 1; // PICにアナログ値読取り開始を指示
    __delay_us(500);
    while (GO_nDONE); // PICが読取り完了するまで待つ
    temp = ADRESH; // PICは読取った値をADRESHとADRESLのレジスターにセットする
    temp = (temp << 8) | ADRESL; // 10ビットの分解能力

    return temp;
}

//unsigned long abval(long val) {
//    return (val < 0 ? (-val) : val);
//}

void setUp() {
    OSCCON = 0b01110000;

    // A/Dの設定
    ADCON1 = 0b10010000; // 読取値は右寄せ、A/D変換クロックはFOSC/8、VDDをリファレンスに
    ADCON0 = 0b00000001; // アナログ変換情報設定
    __delay_us(5); // アナログ変換情報が設定されるまでとりあえず待つ

    TRISA = 0b00000000;
    TRISC = 0b00000000;
    ANSELA = 0b00000000;
    ANSELC = 0b00000000;

    //WDT timer0
    OPTION_REGbits.PSA = 0b1;
    OPTION_REGbits.PS = 0b111; // 1:256


    //MP3 Power
    TRISAbits.TRISA4 = 0;
    //    WPUA = 0b00010000;

    //ANALOG
    //C1 AN5 -- RPR220
    TRISCbits.TRISC1 = 1;
    ANSELCbits.ANSC1 = 1;

    //IR-LED
    //A2 -- IR-LED
    TRISAbits.TRISA2 = 0;

    TRISCbits.TRISC4 = 0;
    RC4PPS = 0b10100; // 出力(TXを割当てる)

    //mp3Busy
    TRISCbits.TRISC5 = 1;
    RC5PPS = 0;
    //    WPUC = 0b00100000;
    //    RXPPSbits.RXPPS = 0b10101; // 入力(RC5を割当てる:デフォルト)

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

    T2CON = 0b00000010; // TMR2プリスケーラ値を１６倍に設定
    TMR2 = 0; // タイマー２カウンターを初期化
    PR2 = 124; // PWMの周期を設定（1000Hzで設定）
    TMR2ON = 1; // TMR2(PWM)スタート

    TMR2IE = 1;
    PEIE = 1;
    GIE = 1;
}

void setSerialDebug() {
    TRISCbits.TRISC4 = 0;
    RC4PPS = 0b10100; // 出力(TXを割当てる)
    __delay_ms(2);

    TRISCbits.TRISC3 = 0;
    RC3PPS = 0; // 出力(TXをNO)
    __delay_ms(2);
}

void setSerialDFMp3() {
    TRISCbits.TRISC3 = 0;
    RC3PPS = 0b10100; // 出力(TXを割当てる)
    __delay_ms(2);

    TRISCbits.TRISC4 = 0;
    RC4PPS = 0; // 出力(TXをNO)
    __delay_ms(2);

}

void ledOn() {
    LATAbits.LATA2 = 0;
}

void ledOff() {
    LATAbits.LATA2 = 1;
}

//void sensing() {
//    ledOn();
//    __delay_ms(1);
//    num1Buf[index] = (adconv(5) * adconv(5)) / 10;
//    index = (index + 1) % BUFFER_LENGTH;
//    num1 = (median(num1Buf, BUFFER_LENGTH));
//    ave1 = ave1 * (63.0 / 64.0) + (float) (num1) * (1.0 / 64.0);
//
//    ledOff();
//    __delay_ms(1);
//    num2Buf[index] = (adconv(5)); // * adconv(5) * adconv(5)) / 100;
//    num2 = (median(num2Buf, BUFFER_LENGTH));
//    ave2 = ave2 * (63.0 / 64.0) + (float) (num2) * (1.0 / 64.0);
//}

//void serialDFMp3Debug() {
//    setSerialDebug();
//    while (!TRMT);
//    TXREG = 255;
//
//
//}

void serialDebug() {

    setSerialDebug();

    //RPR220 on
    while (!TRMT);
    TXREG = 255;
    while (!TRMT);
    TXREG = (((timeCount >> 21)) & 0x7F);
    while (!TRMT);
    TXREG = (((timeCount >> 14)) & 0x7F);
    while (!TRMT);
    TXREG = (((timeCount >> 7)) & 0x7F);
    while (!TRMT);
    TXREG = (((timeCount >> 0)) & 0x7F);

    while (!TRMT);
    TXREG = ((mp3ResetCount >> 21) & 0x7F);
    while (!TRMT);
    TXREG = ((mp3ResetCount >> 14) & 0x7F);
    while (!TRMT);
    TXREG = ((mp3ResetCount >> 7) & 0x7F);
    while (!TRMT);
    TXREG = (mp3ResetCount & 0x7F);

    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave1) >> 21) & 0x7F);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave1) >> 14) & 0x7F);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave1) >> 7) & 0x7F);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave1)) & 0x7F);


    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave2) >> 21) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave2) >> 14) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave2) >> 7) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (ave2)) & 0b01111111);


    while (!TRMT);
    TXREG = (unsigned int) toggleTrue >> 7;
    while (!TRMT);
    TXREG = (unsigned int) toggleTrue & 0b01111111;

    while (!TRMT);
    TXREG = (unsigned int) (unsigned int) mp3Busy >> 7;
    while (!TRMT);
    TXREG = (unsigned int) (unsigned int) mp3Busy & 0b01111111;

    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (baseLine) >> 21) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (baseLine) >> 14) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (baseLine) >> 7) & 0b01111111);
    while (!TRMT);
    TXREG = (unsigned int) (((unsigned int) (baseLine) & 0b01111111));

    //        while (!TRMT);
    //        TXREG = (int) (abval(((int) (abval(dif - aveDif)) >> 21) & 0b01111111));
    //        while (!TRMT);
    //        TXREG = (int) (abval(((int) (abval(dif - aveDif)) >> 14) & 0b01111111));
    //        while (!TRMT);
    //        TXREG = (int) (abval(((int) (abval(dif - aveDif)) >> 7) & 0b01111111));
    //        while (!TRMT);
    //        TXREG = (int) (abval(((int) (abval(dif - aveDif))) & 0b01111111));

}

/******************************
GlobalInt
 *******************************/
//#define UART_BUFFER_SIZE   64
//unsigned char UART_Buffer[UART_BUFFER_SIZE]; // 受信したデータを格納するバッファ
//unsigned char UART_Buffer_inptr;
//unsigned char UART_Buffer_outptr;

//void interrupt isr(void) {
//    int x;
//    unsigned char dt;
//
//    if (RCIF == 1) {
//        dt = RCREG;
//        x = (UART_Buffer_outptr + 1) % UART_BUFFER_SIZE;
//        if (x != UART_Buffer_inptr) {
//            UART_Buffer[UART_Buffer_outptr] = dt;
//            UART_Buffer_outptr = x;
//        }
//
//        if (OERR == 1) {
//            CREN = 0;
//            while (OERR);
//            CREN = 1;
//        }
//        RCIF = 0;
//    }
//}

//unsigned char UART_Read() {
//    unsigned int ans;
//    if (UART_Buffer_inptr == UART_Buffer_outptr) return 0xffff;
//    ans = UART_Buffer[UART_Buffer_inptr];
//    UART_Buffer_inptr = (UART_Buffer_inptr + 1) % UART_BUFFER_SIZE;
//
//    return ans;
//}
//
//int UART_Available() {
//    return (UART_Buffer_outptr + UART_BUFFER_SIZE - UART_Buffer_inptr) % UART_BUFFER_SIZE;
//}
