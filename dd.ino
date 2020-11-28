#include "ssdumbdisplay.h"
#include "ddtester.h"


DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));

void setup() {
}

void loop() {
  bool mb = true;
  bool turtle = true;
  StandardDDTestLoop(dumbdisplay, mb, turtle);
}


