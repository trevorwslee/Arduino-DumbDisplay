// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"


//#define USE_BLUETOOTH

unsigned long serialBaud = 9600/*DD_SERIAL_BAUD*/;


#ifdef USE_BLUETOOTH
unsigned long baud = DUMBDISPLAY_BAUD;
boolean enableSerial = true;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), baud, enableSerial, serialBaud));
#else
boolean enableSerial = true;
DumbDisplay dumbdisplay(new DDInputOutput(serialBaud));
#endif


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


