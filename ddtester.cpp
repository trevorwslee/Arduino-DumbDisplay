#include "ddtester.h"


MbDDTester* CreateMbTester(DumbDisplay& dumbdisplay) {
  MbDDLayer *pMbLayer = dumbdisplay.createMicrobitLayer(5, 5);;
  return new MbDDTester(pMbLayer);
}

TurtleDDTester* CreateTurtleTester(DumbDisplay& dumbdisplay) {
  TurtleDDLayer *pTurtleLayer = dumbdisplay.createTurtleLayer(215, 215);
  pTurtleLayer->penColor("blue");
  return new TurtleDDTester(pTurtleLayer);
}


void debugMbTestStep(MbDDLayer *pLayer, int stepCount) {
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

void debugTurtleTestStep(TurtleDDLayer *pLayer, int stepCount) {
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

void shapeTurtleTestStep(TurtleDDLayer *pLayer, int stepCount) {
    pLayer->clear();
    pLayer->home(false);
    pLayer->setHeading(0);
    pLayer->penSize(2);

    bool filled = stepCount > 10;
    pLayer->penFilled(filled);
    if (filled) {
        pLayer->fillColor("yellow");
    }

    pLayer->leftTurn(20);
    pLayer->forward(20);

    int shape = stepCount % 11;
    switch (shape) {
      case 0:
        pLayer->circle(50);
        break;
      case 1:
        pLayer->rectangle(80, 60);
        break;
      case 2:
        pLayer->triangle(80, 50, 40);
        break;
      case 3:
        pLayer->isoscelesTriangle(50, 65);
        break;
      case 4:
        pLayer->circle(55, true);
        break;
      case 5:
        pLayer->rectangle(60, 70, true);
        break;
      case 6:
        pLayer->polygon(50, 5);
        break;
      case 7:
        pLayer->enclosedPolygon(55, 5);
        break;
      case 8:
        pLayer->oval(60, 80);
        break;
      case 9:
        pLayer->oval(60, 80, true);
        break;
      case 10:
        for (int size = 3; size <= 10; size++) {
            pLayer->clear();
            pLayer->enclosedPolygon(40, size);   
            pLayer->circle(40, true);
            delay(500);
        }
        break;
    }
    pLayer->rightTurn(90);
    pLayer->forward(100);

}




void MbDDTester::testStep(int stepCount) {
  debugMbTestStep(pLayer, stepCount);
}
void TurtleDDTester::testStep(int stepCount) {
  if (stepCount > 1) 
    shapeTurtleTestStep(pLayer, stepCount);
  else
    debugTurtleTestStep(pLayer, stepCount);
}

