/* 
 * File:   mp3.c
 * Author: MATHRAX
 *
 * Created on March 22, 2018, 7:10 PM
 */

#include <xc.h>
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
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
    int i;
    for (i = 1; i < 7; i++) {
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
    
    setSerialDFMp3();
    
    send_buf[3] = cmd;

    send_buf[5] = high_arg;
    send_buf[6] = low_arg;

    mp3_fill_checksum();

    for (i = 0; i < 10; i++) {
        while (!TRMT);
        TXREG = send_buf[i];
    }

    __delay_ms(50); //50
//    CLRWDT();
}
