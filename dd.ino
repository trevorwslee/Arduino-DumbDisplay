#include "ssdumbdisplay.h"


#define MB
#define TURTLE

const bool enableSerial = true;

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), enableSerial));


#ifdef MB
MicroBitLayer* pMbLayer = NULL;
#endif
#ifdef TURTLE
TurtleLayer* pTurtleLayer = NULL;
#endif

void setup() {
  if (!enableSerial)
    Serial.begin(115200);
#ifdef TURTLE
  pTurtleLayer = dumbdisplay.createTurtleLayer(251, 201);
#endif
#ifdef MB
  pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);
#endif
}

int loopCount = 0;
void loop() {
  if (loopCount == 10) {
#ifdef TURTLE
    pTurtleLayer->clear();
#endif
#ifdef MB
    pMbLayer->clear();
#endif   
    loopCount = 0;
  }
  if (loopCount == 0)
    basic();
#ifdef MB
  pMbLayer->showNumber(loopCount);
#endif  
#ifdef TURTLE
  pTurtleLayer->circle(33);
  pTurtleLayer->rightTurn(13);
#endif
  loopCount++;
  delay(1000);
}

void basic() {
#ifdef TURTLE
  if (random(2) == 0)
    pTurtleLayer->home();
  else  
    pTurtleLayer->goTo(10, -10);
  pTurtleLayer->forward(50);
  pTurtleLayer->rightTurn(25);
  pTurtleLayer->backward(32);
  pTurtleLayer->leftTurn(48);
  pTurtleLayer->forward(88);
  pTurtleLayer->home(false);
  delay(2000);
#endif

#ifdef MB
  //pMbLayer->opacity(200);
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
#endif
}
