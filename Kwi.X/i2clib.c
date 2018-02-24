/*
 * File:   i2clib.c
 * Author: shozo
 *
 * Created on February 16, 2018, 6:30 PM
 */


#include <xc.h>
#include "i2clib.h"
#include "main.h"

int AckCheck ;
// アイドル状態のチェック
// ACKEN RCEN PEN RSEN SEN R/W BF が全て0ならOK
void I2C_IdleCheck(char mask)
{
     while (( SSPCON2 & 0x1F ) | (SSPSTAT & mask)) ;
}

/*******************************************************************************
*  InterI2C( void )                                                            *
*    I2C関連の割り込み処理                                                  *
*     この関数はメインプログラムの割込み関数で呼びます                         *
*******************************************************************************/
void InterI2C( void )
{
     if (SSPIF == 1) {        // SSP(I2C)割り込み発生か?
      if (AckCheck == 1) AckCheck = 0 ;
          SSPIF = 0 ;         // フラグクリア
     }
     if (BCLIF == 1) {        // MSSP(I2C)バス衝突割り込み発生か?
          BCLIF = 0 ;         // 今回はフラグのみクリアする(無処理)
     }
}
/*******************************************************************************
*  InitI2C_Master()                                                            *
*    I2C通信のマスターモードで初期化を行う処理                              *
*                                                                              *
*    注)クロック8MHzでの設定です、他のクロックはSSPADDを変更する必要が有ります *
*******************************************************************************/
void InitI2C_Master()
{
     __delay_ms(40);
     SSPSTAT= 0b10000000 ;    // 標準速度モードに設定する(100kHz)
     SSPCON1= 0b00101000 ;    // SDA/SCLピンはI2Cで使用し、マスターモードとする
     SSPADD = MySSPADD ;      // クロック=FOSC/((SSPADD + 1)*4) 8MHz/((0x13+1)*4)=0.1(100KHz)
     SSPIE = 1 ;              // SSP(I2C)割り込みを許可する
     BCLIE = 1 ;              // MSSP(I2C)バス衝突割り込みを許可する
     PEIE   = 1 ;             // 周辺装置割り込みを許可する
     GIE    = 1 ;             // 全割り込み処理を許可する 
     SSPIF = 0 ;              // SSP(I2C)割り込みフラグをクリアする
     BCLIF = 0 ;              // MSSP(I2C)バス衝突割り込みフラグをクリアする
}
/*******************************************************************************
*  ans = I2C_Start(adrs,rw)                                                    *
*    スレーブにスタートコンディションを発行する処理                            *
*                                                                              *
*    adrs : スレーブのアドレスを指定します                                     *
*    rw   : スレーブに対する動作の指定をします                                 *
*           0=スレーブに書込みなさい要求 1=スレーブに送信しなさい要求         *
*    ans  : 0=正常 1=異常(相手からACKが返ってこない)                          *
*******************************************************************************/
int I2C_Start(int adrs,int rw)
{
     // スタート(START CONDITION)
     I2C_IdleCheck(0x5) ;
     SSPCON2bits.SEN = 1 ;
     // [スレーブのアドレス]を送信する
     I2C_IdleCheck(0x5) ;
     AckCheck = 1 ;
     SSPBUF = (char)((adrs<<1)+rw) ;    // アドレス + R/Wを送信
     while (AckCheck) ;                 // 相手からのACK返答を待つ
     return SSPCON2bits.ACKSTAT ;
}
/*******************************************************************************
*  ans = I2C_rStart(adrs,rw)                                                   *
*    スレーブにリピート・スタートコンディションを発行する処理                  *
*                                                                              *
*    adrs : スレーブのアドレスを指定します                                     *
*    rw   : スレーブに対する動作の指定をします                                 *
*           0=スレーブに書込みなさい要求 1=スレーブに送信しなさい要求         *
*    ans  : 0=正常 1:異常(相手からACKが返ってこない)                          *
*******************************************************************************/
int I2C_rStart(int adrs,int rw)
{
     // リピート・スタート(REPEATED START CONDITION)
     I2C_IdleCheck(0x5) ;
     SSPCON2bits.RSEN = 1 ;
     // [スレーブのアドレス]を送信する
     I2C_IdleCheck(0x5) ;
     AckCheck = 1 ;
     SSPBUF = (char)((adrs<<1)+rw) ;    // アドレス + R/Wを送信
     while (AckCheck) ;                 // 相手からのACK返答を待つ
     return SSPCON2bits.ACKSTAT ;
}
/*******************************************************************************
*  I2C_Stop()                                                                  *
*    スレーブにストップコンディションを発行する処理                            *
*******************************************************************************/
void I2C_Stop()
{
     // ストップ(STOP CONDITION)
     I2C_IdleCheck(0x5) ;
     SSPCON2bits.PEN = 1 ;
}
/*******************************************************************************
*  ans = I2C_Send(dt)                                                          *
*    スレーブにデータを1バイト送信する処理                                    *
*                                                                              *
*    dt  : 送信するデータを指定します                                          *
*    ans  : 0=正常 1=異常(相手からACKが返ってこない又はNOACKを返した)         *
*******************************************************************************/
int I2C_Send(char dt)
{
     I2C_IdleCheck(0x5) ;
     AckCheck = 1 ;
     SSPBUF = dt ;                      // データを送信
     while (AckCheck) ;                 // 相手からのACK返答を待つ
     return SSPCON2bits.ACKSTAT ;
}
/*******************************************************************************
*  ans = I2C_Receive(ack)                                                      *
*    スレーブからデータを1バイト受信する処理                                  *
*                                                                              *
*    ack  : スレーブへの返答データを指定します                                 *
*           0:ACKを返す 1:NOACKを返す(受信データが最後なら1)                  *
*    ans  : 受信したデータを返す                                               *
*******************************************************************************/
char I2C_Receive(int ack)
{
 char dt ;
 
     I2C_IdleCheck(0x5) ;
     SSPCON2bits.RCEN = 1 ;        // 受信を許可する
     I2C_IdleCheck(0x4) ;
     dt = SSPBUF ;                 // データの受信
     I2C_IdleCheck(0x5) ;
     SSPCON2bits.ACKDT = ack ;     // ACKデータのセット
     SSPCON2bits.ACKEN = 1 ;       // ACKデータを返す
     return dt ;
}