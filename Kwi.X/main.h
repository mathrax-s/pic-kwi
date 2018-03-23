
#include <xc.h>
#include <stdint.h>
#include <stddef.h>

#ifndef __MAIN_H__
#define __MAIN_H__

#define _XTAL_FREQ  32000000
#define BUFFER_LENGTH  10

//mp3.c
#define MAX_VOL 25
extern uint8_t send_buf[10];
extern uint8_t recv_buf[10];

extern uint16_t mp3_get_checksum(uint8_t *thebuf);
extern void mp3_fill_checksum();
extern void mp3_send_cmd(uint8_t cmd, uint16_t high_arg, uint16_t low_arg);

//sub.c
extern unsigned long num1, num2;
extern unsigned long tmp1, tmp2;
extern unsigned long num1Buf[BUFFER_LENGTH];
extern unsigned long num2Buf[BUFFER_LENGTH];
extern unsigned int index;
extern float d;
extern unsigned char status;
extern signed char vol;
extern unsigned int timeCount;
extern unsigned int fadeoutCount;
extern float ave1, ave2;
extern float dif;
extern float baseLine;
extern float lastDif;
extern float aveDif;
extern unsigned char toggle;
extern float aveCount;
extern int toggleCount1;
extern int toggleCount2;
extern int toggleTrue;


extern unsigned int adconv(uint8_t ch);
extern unsigned long abval(long val);
extern void setUp();
extern void setSerialDebug();
extern void setSerialDFMp3();
extern void ledOn();
extern void ledOff();
extern void sensing();
extern void serialDebug();

extern unsigned long sorted[BUFFER_LENGTH];
extern unsigned long median(unsigned long samples[], int m);


extern unsigned int count;
extern void interrupt InterTimer(void);

#endif