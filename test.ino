// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "dumbdisplay.h"
#include "ddtester.h"


DumbDisplay dumbdisplay(new DDInputOutput(57600));

void setup() {
  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    DDLogToSerial("=== connected ===");
    dumbdisplay.writeComment("Good Day!");
  }
}

void loop() {
  BasicDDTestLoop(dumbdisplay);
}


