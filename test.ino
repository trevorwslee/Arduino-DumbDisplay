#include "ssdumbdisplay.h"
#include "ddtester.h"


boolean enableSerial = true;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), enableSerial));
//DumbDisplay dumbdisplay(new DDInputOutput());


void setup() {
  if (!enableSerial)
    Serial.begin(115200);
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


