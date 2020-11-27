#include <SoftwareSerial.h>
#include "dumbdisplay.h"




DumbDisplay dumbdisplay(new DDInputOutput(new SoftwareSerial(2,3)));
MicroBitLayer* pMbLayer = NULL;

void setup() {
  Serial.begin(115200);

  dumbdisplay.connect();
  pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);

  pMbLayer->showNum(8);
}

void loop() {
  

}
