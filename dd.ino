#include <SoftwareSerial.h>
#include "dumbdisplay.h"



const boolean allowSerial = true;

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), allowSerial));
MicroBitLayer* pMbLayer = NULL;

void setup() {
  if (!allowSerial) {
    Serial.begin(115200);
  }
  
  dumbdisplay.connect();
  pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);

  pMbLayer->showNum(8);
}

void loop() {
  

}
