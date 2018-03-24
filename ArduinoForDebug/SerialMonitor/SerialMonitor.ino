#include <SoftwareSerial.h>
SoftwareSerial mySerial(13, 12); // RX, TX

int pos;
int buf[100];

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

      if (pos >= 24) {
        pos = 0;
        int p;

        p = 0;
        d = ((buf[p + 0] & 0x7F) << 21) + ((buf[p + 1] & 0x7F) << 14) + ((buf[p + 2] & 0x7F) << 7) + (buf[p + 3] & 0x7F);
        Serial.print(d); Serial.print(",");

        p = 4;
        d = ((buf[p + 0] & 0x7F) << 21) + ((buf[p + 1] & 0x7F) << 14) + ((buf[p + 2] & 0x7F) << 7) + (buf[p + 3] & 0x7F);
        Serial.print(d); Serial.print(",");

        p = 8;
        d = ((buf[p + 0] & 0x7F) << 21) + ((buf[p + 1] & 0x7F) << 14) + ((buf[p + 2] & 0x7F) << 7) + (buf[p + 3] & 0x7F);
        Serial.print(d); Serial.print(",");

        p = 12;
        d = ((buf[p + 0] & 0x7F) << 21) + ((buf[p + 1] & 0x7F) << 14) + ((buf[p + 2] & 0x7F) << 7) + (buf[p + 3] & 0x7F);
        Serial.print(d); Serial.print(",");

        p = 16;
        Serial.print(((buf[p] & 0x7F) << 7) + (buf[p + 1] & 0x7F) * 500);
        Serial.print(",");
        
        p = 18;
        Serial.print(((buf[p] & 0x7F) << 7) + (buf[p + 1] & 0x7F)*400);
        Serial.print(",");
        
        p = 20;
        d = ((buf[p + 0] & 0x7F) << 21) + ((buf[p + 1] & 0x7F) << 14) + ((buf[p + 2] & 0x7F) << 7) + (buf[p + 3] & 0x7F);
        Serial.print(d);
        Serial.println("\t");
      }
    }

  }
}
