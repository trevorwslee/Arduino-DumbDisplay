#include "ssdumbdisplay.h"



const boolean allowSerial = true;

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), allowSerial));
MicroBitLayer* pMbLayer = NULL;

void setup() {
  if (!allowSerial) {
    Serial.begin(115200);
  }
  
  pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);

}

int loopCount = 0;
void loop() {
  if (loopCount == 10) {
    pMbLayer->clearScreen();
    loopCount = 0;
  }
  if (loopCount == 0)
    basic();
  pMbLayer->showNumber(loopCount);
  loopCount++;
  delay(1000);
}

void basic() {
  pMbLayer->backgroundColor("12-34-56");
  pMbLayer->ledColor(0xff00ff);

  pMbLayer->showLeds("|.#.#|####");
  delay(2000);

  pMbLayer->backgroundColor("lightyellow");
  pMbLayer->ledColor("green");

  pMbLayer->toggle(1, 2);
  delay(1000);
  pMbLayer->unplot(1, 2);
  delay(1000);
  pMbLayer->plot(1, 2);
  delay(1000);

  pMbLayer->noBackgroundColor();
  pMbLayer->showIcon(SmallDiamond);
  delay(1000);
  pMbLayer->showNumber(8);
  delay(1000);
  pMbLayer->showString("a,b,c");
  delay(5000);
}
