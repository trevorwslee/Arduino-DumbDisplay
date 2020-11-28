#include "ssdumbdisplay.h"
#include "ddtester.h"


DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));

void setup() {
}

void loop() {
  StandardDDTestLoop(dumbdisplay, true, true);
}


