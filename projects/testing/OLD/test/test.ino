
// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"


DumbDisplay dumbdisplay(new DDInputOutput(115200));
//DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(11, 10), 115200, true, 115200));


void IdleCallback(long idleForMillis) {
  Serial.print("* IDLE: ");
  Serial.println(idleForMillis / 1000);
}


void setup() {
  // Serial.begin(57600);
  // Serial.println("*****");
  dumbdisplay.setIdleCalback(IdleCallback);
  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    dumbdisplay.logToSerial("=== connected ===");
    dumbdisplay.writeComment("Good Day!");
  }
}

void loop() {
  BasicDDTestLoop(dumbdisplay);
}
