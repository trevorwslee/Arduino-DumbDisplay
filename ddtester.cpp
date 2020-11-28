#include "ddtester.h"


MbTester* CreateMbTester(DumbDisplay& dumbdisplay) {
  MicroBitLayer *pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);;
  return new MbTester(pMbLayer);
}
TurtleTester* CreateTurtleTester(DumbDisplay& dumbdisplay) {
  TurtleLayer *pTurtleLayer = dumbdisplay.createTurtleLayer(251, 201);
  return new TurtleTester(pTurtleLayer);
}

void MbTester::testStep(int stepCount) {

}
void TurtleTester::testStep(int stepCount) {

}

void DDTester::mbTestStep(MicroBitLayer* pMbLayer, int stepCount) {
    int count = stepCount % 10;
    if (count == 0) {
      pMbLayer->clear();

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
    
    pMbLayer->showNumber(count);

    delay(1000);
}

void DDTester::turtleTestStep(TurtleLayer* pTurtleLayer, int stepCount) {
  if (stepCount == 0) {
    pTurtleLayer->clear();
    pTurtleLayer->setHeading(random(360));
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
    delay(1000);
  }
  delay(1000);
}


