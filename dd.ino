#include "ssdumbdisplay.h"
#include "ddtester.h"

boolean enableSerial = true;
DumbDisplay TheDumbDisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), enableSerial));

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
  BasicDDTestLoop(enableSerial, TheDumbDisplay, mb, turtle, ledGrid, lcd);
}


