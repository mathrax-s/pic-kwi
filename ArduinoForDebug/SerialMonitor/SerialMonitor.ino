#include <SoftwareSerial.h>
SoftwareSerial mySerial(13, 12); // RX, TX

int pos;
int buf[20];

unsigned long d;

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
}

void loop() {

  if (mySerial.available()) {

    int rcvdata = mySerial.read();
    //    Serial.print(rcvdata);
    if (rcvdata == 255) {
      pos = 0;
    } else {
      buf[pos] = rcvdata;
      pos++;

      if (pos >= 16) {
        pos = 0;
        int p;
        
        p=0;
        d = ((buf[p+0] & 0x7F) << 21) + ((buf[p+1] & 0x7F) << 14) + ((buf[p+2] & 0x7F) << 7) + (buf[p+3] & 0x7F);
        Serial.print("nm1:\t");
        Serial.print(d);
        
//        p=4;
//        d = ((buf[p+0] & 0x7F) << 21) + ((buf[p+1] & 0x7F) << 14) + ((buf[p+2] & 0x7F) << 7) + (buf[p+3] & 0x7F);
//        Serial.print("\t\tnm2:\t");
//        Serial.print(d);
        
        p=8;
        d = ((buf[p+0] & 0x7F) << 21) + ((buf[p+1] & 0x7F) << 14) + ((buf[p+2] & 0x7F) << 7) + (buf[p+3] & 0x7F);
        Serial.print("\t\tave:\t");
        Serial.print(d);
        
        p=12;
        Serial.print("\t\ttgl:\t");
        Serial.print(((buf[p] & 0x7F) << 7) + (buf[p+1] & 0x7F));
       
        p=14;
        Serial.print("\t\tdif:\t");
        Serial.print(((buf[p] & 0x7F) << 7) + (buf[p+1] & 0x7F));
        Serial.println("\t");
      }
    }

  }
}
