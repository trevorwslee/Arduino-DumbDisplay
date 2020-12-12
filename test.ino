// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"



unsigned long baud = DUMBDISPLAY_BAUD;
boolean enableSerial = true;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), baud, enableSerial));
//DumbDisplay dumbdisplay(new DDInputOutput());


void setup() {
  if (!enableSerial) {
    // if DD not using Serial, setup Serial here
    Serial.begin(115200);
  }

  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    dumbdisplay.writeComment("Good Day!");
  }
}

void loop() {
  if (!enableSerial) {
    // if DD not using Serial, print something to Serial
    Serial.println("hello");
  }
  BasicDDTestLoop(dumbdisplay);
}


