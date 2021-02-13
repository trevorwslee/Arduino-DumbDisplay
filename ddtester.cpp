#include "Arduino.h"
#include "ddtester.h"


#define ASCII_ONLY false


namespace DDTesterImpl {

void debugMbTestStep(MbDDLayer *pLayer, int stepCount) {
    int count = stepCount % 10;
    if (count == 0) {
      pLayer->clear();

      pLayer->backgroundColor("12-34-56");
      pLayer->ledColor(DD_HEX_COLOR(0xff00ff));

      pLayer->showLeds("|.#.#|####");
      DDDelay(2000);

      pLayer->backgroundColor("lightyellow");
      pLayer->ledColor("green");

      pLayer->toggle(1, 2);
      DDDelay(1000);
      pLayer->unplot(1, 2);
      DDDelay(1000);
      pLayer->plot(1, 2);
      DDDelay(1000);

      pLayer->noBackgroundColor();
      pLayer->showIcon(SmallDiamond);
      DDDelay(1000);
      pLayer->showNumber(8);
      DDDelay(1000);
      pLayer->showString("a,b,c");
      DDDelay(5000);
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
            DDDelay(500);
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
    if (true) {
      for (int i = 0; i < 4; i++) {
        if (i == 0) {
          pLayer->backgroundColor("aliceblue");
          pLayer->drawRect(2, 2, 32, 42, "black");
          pLayer->drawTriangle(4, 4, 32, 32, 8, 42, "blue");
          pLayer->fillTriangle(6, 8, 28, 30, 10, 40, "green");
        } else if (i == 1) {
          pLayer->drawRect(2, 2, 60, 70, "black");
          pLayer->drawRoundRect(4, 4, 56, 66, 18, "blue");
          pLayer->fillRoundRect(6, 6, 52, 62, 20, "green");
        } else if (i == 2) {
          pLayer->drawCircle(11, 11, 9, "green");
          pLayer->fillCircle(11, 11, 7, "blue");
        } else {
          pLayer->drawRect(0, 0, 24, 34, "blue");
          pLayer->fillRect(2, 2, 20, 30, "red");
        }
        DDDelay(2000);
        pLayer->clear();
      }
    }
    pLayer->drawPixel(5, 10, "red");
    pLayer->drawLine(40, 50, 60, 100, "darkgreen");
    pLayer->drawChar(20, 30, '@', "red", "blue", 32);
    DDDelay(2000);
    pLayer->backgroundColor("black");
    pLayer->fillScreen("lightgreen");
    pLayer->write("AA");
    DDDelay(500);
    pLayer->setCursor(20, 0);
    if (ASCII_ONLY) {
      pLayer->println("FRIEND");
    } else {
      pLayer->println("你好!");
    }
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
    pLayer->setTextColor("red", "pink");
    pLayer->print("Hello");
    pLayer->setTextColor("blue");
  }
  if (step-- == 0) {
    pLayer->println(", World!");
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
    pLayer->backgroundColor("beige");
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
            DDDelay(500);
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
SevenSegmentRowDDLayer *p7SegmentRowLayer = NULL;

void DDTester_autoPinLayers(DumbDisplay& dumbdisplay) {
  if (pMbLayer == NULL ||
      pTurtleLayer == NULL ||
      pLedGridLayer == NULL ||
      pLcdLayer == NULL ||
      pGraphicalLayer == NULL || 
      p7SegmentRowLayer == NULL) {
    dumbdisplay.configAutoPin(DD_AP_HORI);
  } else {  
    dumbdisplay.configAutoPin(DD_AP_VERT_4(
                                p7SegmentRowLayer->getLayerId(),
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
        DDTester_autoPinLayers(dumbdisplay);
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
        DDTester_autoPinLayers(dumbdisplay);
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
        DDTester_autoPinLayers(dumbdisplay);
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
        DDTester_autoPinLayers(dumbdisplay);
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
  if (ASCII_ONLY) {
    pLcdLayer->writeLine(((stepCount / 10) % 2 == 0) ? "FRIEND" : "Hi friend", 1, "C");
    pLcdLayer->writeLine("How do you do?", 2, "R");
  } else {  
    pLcdLayer->writeLine(((stepCount / 10) % 2 == 0) ? "朋友" : "Hi friend", 1, "C");
    pLcdLayer->writeLine("Bună Привіт 你好", 2, "R");
  }
}

void SevenSegmentRowDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    p7SegmentRowLayer = dumbdisplay.create7SegmentRowLayer(4);
    if (Pinned) {
      if (AutoPin) {
        DDTester_autoPinLayers(dumbdisplay);
      }
    }
    p7SegmentRowLayer->backgroundColor(DD_HEX_COLOR(0x334455));
    p7SegmentRowLayer->segmentColor("yellow");
  }

  char allSegs[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' }; 
  int maxCount1 = 16;
  int maxCount2 = maxCount1 + 5;
  if (stepCount < maxCount1) {
    if ((stepCount % 2) == 0)
      p7SegmentRowLayer->turnOn("abcdefg.", 2);
    else
      p7SegmentRowLayer->turnOff("abcdefg.", 2);
    if (stepCount < 8)  
      p7SegmentRowLayer->turnOn(String(allSegs[stepCount % 8]));
    else
      p7SegmentRowLayer->turnOff(String(allSegs[stepCount % 8]));
  } else if (stepCount < maxCount2) {
    int counter = stepCount - maxCount1;
    if (counter-- == 0) p7SegmentRowLayer->showHexNumber(0xfb);
    else if (counter-- == 0) {
      p7SegmentRowLayer->showFormatted("00.00");
    } else {
      if (stepCount % 2 == 0)
        p7SegmentRowLayer->showFormatted("----------");
      else
        p7SegmentRowLayer->showFormatted("..........");
    }
  } else {
    int counter = stepCount - maxCount2;
    if (counter-- == 0) p7SegmentRowLayer->showNumber(1);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(12);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(123);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(1234);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(1.00001);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(1.10001);
    else if (counter-- == 0) p7SegmentRowLayer->showNumber(1.234567);
    else {
        float num = random(10000) - random(10000);
        if (random(2) == 0)
          num = num / random(2, 1000);
        p7SegmentRowLayer->showNumber(num);
    }
  }
}

void GraphicalDDTester_testStep(DumbDisplay& dumbdisplay, int stepCount) {
  if (stepCount == 0) {
    pGraphicalLayer = dumbdisplay.createGraphicalLayer(215, 215);
    if (Pinned) {
      if (AutoPin) {
        DDTester_autoPinLayers(dumbdisplay);
      }
    }
    pGraphicalLayer->setTextColor("blue");
    if (true) {
      pGraphicalLayer->setTextFont("monospace");
      pGraphicalLayer->setTextSize(14);
    } else {
      pGraphicalLayer->setTextFont("monospace", 14);
    }
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
  bool sevenSegmentRow = true;

  int testCount = 0;
  if (mb) testCount++;
  if (turtle) testCount++;
  if (ledGrid) testCount++;
  if (lcd) testCount++;
  if (graphical) testCount++;
  if (sevenSegmentRow) testCount++;


  Pinned = testCount > 1;
  AutoPin = true;

  int stepCount = 0;
  while (true) {
    if (graphical)
      GraphicalDDTester_testStep(dumbdisplay, stepCount);
    if (mb)
      MbDDTester_testStep(dumbdisplay, stepCount);
    if (turtle)  
      TurtleDDTester_testStep(dumbdisplay, stepCount);
    if (ledGrid)
      LedGridDDTester_testStep(dumbdisplay, stepCount);  
    if (lcd)
      LcdDDTester_testStep(dumbdisplay, stepCount);
    if (sevenSegmentRow)
      SevenSegmentRowDDTester_testStep(dumbdisplay, stepCount);
    DDDelay(1000);
    stepCount++;
  }
}

