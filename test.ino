// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"


unsigned long baud = DUMBDISPLAY_BAUD;
boolean enableSerial = true;
unsigned long serialBaud = DD_SERIAL_BAUD;

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), baud, enableSerial, serialBaud));

void setup() {
  if (!enableSerial) {
    // if DD not using Serial, setup Serial here
    Serial.begin(115200);
  }

  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    DDLogToSerial("=== connected ===");
    dumbdisplay.writeComment("Good Day!");
  }
}

void loop() {
  bool forDebugging = false;
  BasicDDTestLoop(dumbdisplay, forDebugging);
}


