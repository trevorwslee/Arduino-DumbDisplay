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

  pMbLayer->backgroundcolor("12-34-56");

  pMbLayer->ledColor(0xff00ff);

  pMbLayer->showLeds("|.#.#|####");
  delay(2000);

  pMbLayer->ledColor("green");

  pMbLayer->toggle(1, 2);
  delay(1000);
  pMbLayer->unplot(1, 2);
  delay(1000);
  pMbLayer->plot(1, 2);
  delay(1000);

  pMbLayer->showIcon(SmallDiamond);
  delay(1000);
  pMbLayer->showNumber(8);
  delay(1000);
  pMbLayer->showString("a,b,c");
  delay(5000);
}

int loopCount = 0;
void loop() {
  delay(1000);
  if (loopCount == 10) {
    pMbLayer->clearScreen();
    loopCount = 0;
  }
  pMbLayer->showNumber(loopCount);
  loopCount++;
}
