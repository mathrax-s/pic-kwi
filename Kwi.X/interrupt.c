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
void interrupt InterTimer( void )
{
     if (TMR2IF == 1) {           // タイマー2の割込み発生か？
          count++ ;               // 割込み発生の回数をカウントする
          TMR2IF = 0 ;            // タイマー2割込フラグをリセット
          if (count >= 10) {      // 割込みを125回カウントすると約１秒
               count = 0 ;
               if(wdtOnOff==1){
                   CLRWDT();
               }
          }
     }
}

