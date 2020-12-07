// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"



unsigned long baud = DUMBDISPLAY_BAUD;
boolean enableSerial = true;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), baud, enableSerial));
//DumbDisplay dumbdisplay(new DDInputOutput());


void setup() {
  if (!enableSerial)
    Serial.begin(115200);
  dumbdisplay.debugSetup(13);
}

void loop() {
  if (!enableSerial) {
    Serial.println("hello");
  }
  bool mb = true;
  bool turtle = true;
  bool ledGrid = true;
  bool lcd = true;
  BasicDDTestLoop(dumbdisplay, mb, turtle, ledGrid, lcd);
}


