#include "Arduino.h"
#include "ddtester.h"


namespace DDTesterImpl {

void debugMbTestStep(MbDDLayer *pLayer, int stepCount) {
    int count = stepCount % 10;
    if (count == 0) {
      pLayer->clear();

      pLayer->backgroundColor("12-34-56");
      pLayer->ledColor(DD_HEX_COLOR(0xff00ff));

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
  pLayer->penColor(DD_RGB_COLOR(r, g, b));
  pLayer->circle(27);
  pLayer->rectangle(90, 20);
  pLayer->rightTurn(10);
  b = b + 20;
  if (b > 255) {
      b = 0;
      r = random(0, 255);
  }
}

void standardGraphicalTestStep(GraphicalDDLayer *pLayer, int stepCount) {
  int step = stepCount;
  if (step-- == 0) {
    pLayer->println("你好!");
  }
  if (step-- == 0) {
    pLayer->println();
  }
  if (step-- == 0) {
    pLayer->print("This is a very very long long text line.");
  }
  if (step-- == 0) {
    pLayer->println(" Yes, it is a very very long long text line!"); 
  }
  if (step-- == 0) {
    pLayer->println();
  }
  if (step-- == 0) {
    pLayer->print("Hello, ");
  }
  if (step-- == 0) {
    pLayer->println(" World!");
  }
  if (step-- == 0) {
    pLayer->println();
  }
  if (step-- == 0) {
    pLayer->println("How are you?");
  }
  if (step-- == 0) {
    pLayer->println();
  }
  if (step-- == 0) {
    pLayer->print("Good");
  }
  if (step-- == 0) {
    pLayer->println(" Day!");
  }
}

void shapeGraphicalTestStep(GraphicalDDLayer *pLayer, int stepCount) {
    pLayer->clear();
    pLayer->setCursor(100,100);
    pLayer->setHeading(0);
    pLayer->penColor("darkred");
    pLayer->penSize(2);

    bool filled = stepCount > 10;
    if (filled) {
        pLayer->fillColor("green");
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


bool Pinned = false;
bool AutoPin = true;


MbDDLayer *pMbLayer = NULL;
TurtleDDLayer *pTurtleLayer = NULL;
LedGridDDLayer *pLedGridLayer = NULL;
LcdDDLayer *pLcdLayer = NULL;
GraphicalDDLayer *pGraphicalLayer = NULL;

void LcdDDTester_autoPinLayers(DumbDisplay& dumbdisplay) {
  if (pMbLayer == NULL || pTurtleLayer == NULL || pLedGridLayer == NULL || pLcdLayer == NULL || pGraphicalLayer == NULL) {
    dumbdisplay.configAutoPin(DD_AP_HORI);
  } else {  
    dumbdisplay.configAutoPin(DD_AP_VERT_3(
                                DD_AP_HORI_2(pMbLayer->getLayerId(), pTurtleLayer->getLayerId()),
                                pLcdLayer->getLayerId(),
                                DD_AP_HORI_2(pLedGridLayer->getLayerId(), pGraphicalLayer->getLayerId())
                              ));
  }
}




void MbDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pMbLayer = dumbdisplay.createMicrobitLayer(9, 7);
    if (Pinned) {
      if (AutoPin) {
        LcdDDTester_autoPinLayers(dumbdisplay);
      } else {
        dumbdisplay.pinLayer(pMbLayer, 50, 50, 50, 50);
      }
    }
  }
  debugMbTestStep(pMbLayer, stepCount);
}


void TurtleDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pTurtleLayer = dumbdisplay.createTurtleLayer(215, 215);
    if (Pinned) { 
      if (AutoPin) {
        LcdDDTester_autoPinLayers(dumbdisplay);
      } else {
        dumbdisplay.pinLayer(pTurtleLayer, 0, 50, 50, 50);
      }
    }
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
    if (Pinned) {
      if (AutoPin) {
        LcdDDTester_autoPinLayers(dumbdisplay);
      }
    }
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
    bool reverseSense = random(2) == 0;
    if (hori)
      pLedGridLayer->horizontalBar(4, reverseSense);
    else
      pLedGridLayer->verticalBar(4, reverseSense);
  }
}

void LcdDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pLcdLayer = dumbdisplay.createLcdLayer(18, 3, 16, "Courier");
    if (Pinned) {
      if (AutoPin) {
        LcdDDTester_autoPinLayers(dumbdisplay);
      } else {
        dumbdisplay.pinLayer(pLcdLayer, 0, 0, 100, 50);
      }
    }
    pLcdLayer->pixelColor("red");
    pLcdLayer->bgPixelColor(DD_RGB_COLOR(200, 200, 200));
    pLcdLayer->backgroundColor(DD_HEX_COLOR(0x111111));
  }
  pLcdLayer->setCursor(0, 0);
  pLcdLayer->print("stepCount:" + String(stepCount));
  pLcdLayer->writeLine(((stepCount / 10) % 2 == 0) ? "朋友" : "Hi friend", 1, "C");
  pLcdLayer->writeLine("Bună Привіт 你好", 2, "R");
}

void GraphicalDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pGraphicalLayer = dumbdisplay.createGraphicalLayer(215, 215);
    if (Pinned) {
      if (AutoPin) {
        LcdDDTester_autoPinLayers(dumbdisplay);
      }
    }
    pGraphicalLayer->penColor("blue");
    pGraphicalLayer->setTextFont("monospace", 14);
  }
  if (stepCount <= 14) {
    standardGraphicalTestStep(pGraphicalLayer, stepCount);
  } else {
    shapeGraphicalTestStep(pGraphicalLayer, stepCount);
  }
}


}

using namespace DDTesterImpl;



void BasicDDTestLoop(DumbDisplay& dumbdisplay) {

  bool mb = true;
  bool turtle = true;
  bool ledGrid = true;
  bool lcd = true;
  bool graphical = true;

  int testCount = 0;
  if (mb) testCount++;
  if (turtle) testCount++;
  if (ledGrid) testCount++;
  if (lcd) testCount++;
  if (graphical) testCount++;


  Pinned = testCount > 1;
  AutoPin = true;

  int stepCount = 0;
  while (true) {
    if (mb)
      MbDDTester_testStep(dumbdisplay, stepCount);
    if (turtle)  
      TurtleDDTester_testStep(dumbdisplay, stepCount);
    if (ledGrid)
      LedGridDDTester_testStep(dumbdisplay, stepCount);  
    if (lcd)
      LcdDDTester_testStep(dumbdisplay, stepCount);
    if (graphical)
      GraphicalDDTester_testStep(dumbdisplay, stepCount);
    delay(1000);
    stepCount++;
  }
}

