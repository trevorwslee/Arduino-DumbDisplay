#include "ddtester.h"


// MbDDTester* CreateMbTester(DumbDisplay& dumbdisplay) {
//   MbDDLayer *pMbLayer = dumbdisplay.createMicrobitLayer(9, 7);
//   return new MbDDTester(pMbLayer);
// }

// TurtleDDTester* CreateTurtleTester(DumbDisplay& dumbdisplay) {
//    TurtleDDLayer *pTurtleLayer = dumbdisplay.createTurtleLayer(215, 215);
//    pTurtleLayer->penColor("blue");
//     return new TurtleDDTester(pTurtleLayer);
// }


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
  }
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

    int shape = stepCount % 12;
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
        pLayer->centeredPolygon(55, 5);
        break;
      case 8:
        pLayer->centeredPolygon(55, 5, true);
        break;
      case 9:
        pLayer->oval(60, 80);
        break;
      case 10:
        pLayer->oval(60, 80, true);
        break;
      case 11:
        for (int size = 3; size <= 10; size++) {
            pLayer->clear();
            pLayer->centeredPolygon(40, size, false);   
            pLayer->circle(40, true);
            delay(500);
        }
        break;
    }
    pLayer->rightTurn(90);
    pLayer->forward(100);
}

int r = random(0, 255);
int g = 128;
int b = 0;
void standardTurtleTestStep(TurtleDDLayer *pLayer, bool firstStep) {
  if (firstStep) {
    pLayer->clear();
    pLayer->setHeading(0);
    pLayer->home(false);
    pLayer->backgroundColor("azure");
    pLayer->fillColor("lemonchiffon");
    pLayer->penFilled(true);
    pLayer->centeredPolygon(73, 6, true);
    pLayer->penFilled(false);
    pLayer->penSize(1);
    pLayer->circle(79, true);
  }
  pLayer->penColor(DD_RGB(r, g, b));
  pLayer->circle(27);
  pLayer->rectangle(90, 20);
  pLayer->rightTurn(10);
  b = b + 20;
  if (b > 255) {
      b = 0;
      r = random(0, 255);
  }
}



MbDDLayer *pMbLayer = NULL;
void MbDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pMbLayer = dumbdisplay.createMicrobitLayer(9, 7);
  }
  debugMbTestStep(pMbLayer, stepCount);
}

TurtleDDLayer *pTurtleLayer = NULL;
void TurtleDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pTurtleLayer = dumbdisplay.createTurtleLayer(215, 215);
    pTurtleLayer->penColor("blue");
  }
  if (stepCount > 0) {
    if (stepCount >= 24) {
      standardTurtleTestStep(pTurtleLayer, stepCount == 24);
    } else {
      shapeTurtleTestStep(pTurtleLayer, stepCount);
    }
  } else {
    debugTurtleTestStep(pTurtleLayer, stepCount);
  }
}

LedGridDDLayer *pLedGridLayer = NULL;
bool hori = false;
void LedGridDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  int init = stepCount % 6;
  if (init == 0) {
    if (pLedGridLayer != NULL)
      dumbdisplay.deleteLayer(pLedGridLayer);
    hori = random(2) == 1;
    int colCount;
    int rowCount;
    int subColCount;
    int subRowCount;
    if (hori) {
      colCount = 10;
      rowCount = 4;
      subColCount = 4;
      subRowCount = 2;
    } else {
      colCount = 4;
      rowCount = 8;
      subColCount = 2;
      subRowCount = 4;
    }
    pLedGridLayer = dumbdisplay.createLedGridLayer(colCount, rowCount, subColCount, subRowCount); 
    pLedGridLayer->offColor("lightgray");
  }
  int c = stepCount % 4;
  if (c == 0)
    pLedGridLayer->toggle(2, 3);
  else if (c == 1)
    pLedGridLayer->turnOn(2, 3);
  else if (c == 2)
    pLedGridLayer->turnOff(2, 3);
  else {
    if (hori)
      pLedGridLayer->horizontalBar(4);
    else
      pLedGridLayer->verticalBar(4, true);
  }
}


void BasicDDTestLoop(bool enableSerial, DumbDisplay& dumbdisplay, bool mb, bool turtle, bool ledGrid) {
  if (!enableSerial) Serial.println("start");
  int stepCount = 0;
  while (true) {
    if (!enableSerial) Serial.println("loop");
    if (mb)
      MbDDTester_testStep(dumbdisplay, stepCount);
    if (turtle)  
      TurtleDDTester_testStep(dumbdisplay, stepCount);
    if (ledGrid)
      LedGridDDTester_testStep(dumbdisplay, stepCount);  
    delay(1000);
    stepCount++;
  }
}
