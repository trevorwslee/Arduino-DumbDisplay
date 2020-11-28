#include "ssdumbdisplay.h"
#include "ddtester.h"

boolean enableSerial = true;
//SoftwareSerial *pSS = new SoftwareSerial(2,3);
DumbDisplay TheDumbDisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), enableSerial));

void setup() {
  if (!enableSerial)
    Serial.begin(115200);
  //pSS->begin(115200);  
}

void loop() {
  if (!enableSerial) {
    Serial.println("hello");
  }
  //pSS->println("SS");
  //delay(1000);
  bool mb = true;
  bool turtle = true;
  bool ledGrid = true;
  BasicDDTestLoop(enableSerial, TheDumbDisplay, mb, turtle, ledGrid);
}


