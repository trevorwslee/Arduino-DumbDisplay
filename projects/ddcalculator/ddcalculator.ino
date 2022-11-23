
#if defined(ESP32)
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("LILIGO", true, 115200));
#else
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif

#define TONE_YES 1000
#define TONE_NO 1500
#define TONE_ENTER 1200
#define TONE_CLEAR 800

#include "BasicCalculator.h"

const int DisplayWidth = 10;
const int RowCount = 5;
const int ColCount = 4;
const char Keys[RowCount][ColCount] = {
    {'%', '(', ')', 'C'},
    {'7', '8', '9', '/'},
    {'4', '5', '6', '*'},
    {'1', '2', '3', '-'},
    {'0', '.', '=', '+'}};

void UpdateCaculatorDisplay();
SevenSegmentRowDDLayer *CreateSevenSegDisplayLayer();
LcdDDLayer *CreateDisplayLayer();
LcdDDLayer *CreateKeyLayer(int r, int c);

SevenSegmentRowDDLayer *sevenSegLayer;
LcdDDLayer *displayLayer;
LcdDDLayer *keyLayers[RowCount][5];

BasicCalculator calculator(DisplayWidth);

void setup()
{
  sevenSegLayer = CreateSevenSegDisplayLayer();
  displayLayer = CreateDisplayLayer();
  for (int r = 0; r < RowCount; r++)
  {
    for (int c = 0; c < ColCount; c++)
    {
      LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
      keyLayers[r][c] = keyLayer;
    }
  }
  UpdateCaculatorDisplay();
  dumbdisplay.configAutoPin(
    DD_AP_VERT_3(
      sevenSegLayer->getLayerId(),
      displayLayer->getLayerId(), 
      DD_AP_VERT_5(
        DD_AP_HORI_4(keyLayers[0][0]->getLayerId(), keyLayers[0][1]->getLayerId(), keyLayers[0][2]->getLayerId(), keyLayers[0][3]->getLayerId()),
        DD_AP_HORI_4(keyLayers[1][0]->getLayerId(), keyLayers[1][1]->getLayerId(), keyLayers[1][2]->getLayerId(), keyLayers[1][3]->getLayerId()),
        DD_AP_HORI_4(keyLayers[2][0]->getLayerId(), keyLayers[2][1]->getLayerId(), keyLayers[2][2]->getLayerId(), keyLayers[2][3]->getLayerId()),
        DD_AP_HORI_4(keyLayers[3][0]->getLayerId(), keyLayers[3][1]->getLayerId(), keyLayers[3][2]->getLayerId(), keyLayers[3][3]->getLayerId()),
        DD_AP_HORI_4(keyLayers[4][0]->getLayerId(), keyLayers[4][1]->getLayerId(), keyLayers[4][2]->getLayerId(), keyLayers[4][3]->getLayerId()))
      ));
}

void loop()
{
  DDYield(); // need to call this so that DumbDisplay lib can check for "feedback"
}

void UpdateCaculatorDisplay()
{
  if (calculator.isGrouing()) {
    sevenSegLayer->backgroundColor("beige");
  } else {
    sevenSegLayer->backgroundColor("azure");
  }
  const char* formatted = calculator.getFormatted();
  sevenSegLayer->showFormatted(formatted, true, DisplayWidth - strlen(formatted));
  displayLayer->writeLine(calculator.getFormattedEx(), 0, "R");
}

void FeedbackHandler(DDLayer *pLayer, DDFeedbackType type, const DDFeedback &feedback)
{
  if (pLayer == displayLayer)
  {
    if (type == LONGPRESS)
    {
      displayLayer->flash();
      calculator.reset();
      dumbdisplay.tone(TONE_CLEAR, 300);
      UpdateCaculatorDisplay();
    }
  }
  else
  {
    if (type == CLICK)
    {
      char key = pLayer->customData.charAt(0);
      if (key == 'C')
      {
        calculator.reset();
        dumbdisplay.tone(TONE_CLEAR, 300);
      }
      else if (calculator.push(key))
      {
        dumbdisplay.tone(TONE_YES, 100);
        if (key == '=')
        {
          dumbdisplay.tone(TONE_ENTER, 100);
        }
      }
      else
      {
        dumbdisplay.tone(TONE_NO, 200);
      }
      // if (true) {
      //   double num = calculator.getNum();
      //   const char* formatted = calculator.getFormatted();
      //   dumbdisplay.writeComment(String(num) + " ==> " + formatted);
      // }
      UpdateCaculatorDisplay();
    }
  }
}

SevenSegmentRowDDLayer *CreateSevenSegDisplayLayer() {
  SevenSegmentRowDDLayer *displayLayer = dumbdisplay.create7SegmentRowLayer(DisplayWidth);// .createLcdLayer(DisplayWidth, 1, 28, "sans-serif");
  //displayLayer->backgroundColor("azure");
  displayLayer->segmentColor("darkblue");
  //displayLayer->pixelColor("black");
  displayLayer->border(50, "grey", "raised");
  displayLayer->padding(50);
  displayLayer->setFeedbackHandler(FeedbackHandler);
  return displayLayer;
}

LcdDDLayer *CreateDisplayLayer()
{
  LcdDDLayer *displayLayer = dumbdisplay.createLcdLayer(DisplayWidth, 1, 28, "sans-serif");
  displayLayer->backgroundColor("azure");
  displayLayer->pixelColor("black");
  displayLayer->border(5, "grey", "raised");
  displayLayer->padding(5);
  displayLayer->setFeedbackHandler(FeedbackHandler);
  return displayLayer;
}
LcdDDLayer *CreateKeyLayer(int r, int c)
{
  const char key = Keys[r][c];
  String dispKey;
  if (key == '*') {
    dispKey = "×";
  } else if (key == '/') {
    dispKey = "÷";
  } else if (key == '+') {
    dispKey = "+";
  } else if (key == '-') {
    dispKey = "−";
  } else {
    dispKey = key;
  }
  LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32, "sans-serif-black");
  keyLayer->pixelColor("navy");
  keyLayer->border(5, "grey", "raised");
  keyLayer->padding(1);
  keyLayer->writeLine(dispKey);
  keyLayer->setFeedbackHandler(FeedbackHandler, "fl");
  keyLayer->customData = key;
  return keyLayer;
}
