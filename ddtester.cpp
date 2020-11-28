#include "ddtester.h"


MbDDTester* CreateMbTester(DumbDisplay& dumbdisplay) {
  MbDDLayer *pMbLayer = dumbdisplay.createMicrobitLayer(5, 5);;
  return new MbDDTester(pMbLayer);
}
TurtleDDTester* CreateTurtleTester(DumbDisplay& dumbdisplay) {
  TurtleDDLayer *pTurtleLayer = dumbdisplay.createTurtleLayer(251, 201);
  return new TurtleDDTester(pTurtleLayer);
}


void MbDDTester::testStep(int stepCount) {
    int count = stepCount % 10;
    if (count == 0) {
      pLayer->clear();

      pLayer->backgroundColor("12-34-56");
      pLayer->ledColor(0xff00ff);

      pLayer->showLeds("|.#.#|####");
      delay(2000);

      pLayer->backgroundColor("lightyellow");
      pLayer->ledColor("green");

      pLayer->toggle(1, 2);
      delay(1000);
      pLayer->unplot(1, 2);
      delay(1000);
      pLayer->plot(1, 2);
      delay(1000);

      pLayer->noBackgroundColor();
      pLayer->showIcon(SmallDiamond);
      delay(1000);
      pLayer->showNumber(8);
      delay(1000);
      pLayer->showString("a,b,c");
      delay(5000);

    }
    
    pLayer->showNumber(count);

    delay(1000);
}

void TurtleDDTester::testStep(int stepCount) {
  if (stepCount == 0) {
    pLayer->clear();
    pLayer->setHeading(random(360));
    if (random(2) == 0)
      pLayer->home();
    else  
      pLayer->goTo(10, -10);
    pLayer->forward(50);
    pLayer->rightTurn(25);
    pLayer->backward(32);
    pLayer->leftTurn(48);
    pLayer->forward(88);
    pLayer->home(false);
    delay(1000);
  }
  delay(1000);
}


