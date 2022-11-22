
#if defined(ESP32)
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("LILIGO", true, 115200));
#else
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


#define TONE_YES   1000
#define TONE_NO    1500
#define TONE_ENTER 1200
#define TONE_CLEAR 800

#include "BasicCalculator.h"

const int DisplayWidth = 10;
const char Keys[4][5] = {
    {'7', '8', '9', '%', 'C'},
    {'4', '5', '6', '*', '/'},
    {'1', '2', '3', '+', '-'},
    {'0', '.', '=', '(', ')'}};


LcdDDLayer *CreateDisplayLayer();
LcdDDLayer *CreateKeyLayer(int r, int c);

LcdDDLayer *displayLayer;
LcdDDLayer *keyLayers[4][4];

BasicCalculator calculator(DisplayWidth);

void setup()
{
  displayLayer = CreateDisplayLayer();
  String autoPin("V(");
  for (int r = 0; r < 4; r++)
  {
    if (r > 0)
    {
      autoPin += "+";
    }
    autoPin += "H(";
    for (int c = 0; c < 4; c++)
    {
      LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
      keyLayers[r][c] = keyLayer;
      if (c > 0)
      {
        autoPin += "+";
      }
      autoPin += keyLayer->getLayerId();
    }
    autoPin += ")";
  }
  autoPin += ")";

  dumbdisplay.configAutoPin(DD_AP_VERT_2(displayLayer->getLayerId(), autoPin));
}

void loop()
{
  DDYield(); // need to call this so that DumbDisplay lib can check for "feedback"
}

void UpdateCaculatorDisplay(LcdDDLayer *displayLayer) {
  displayLayer->writeLine(calculator.getFormatted(), 0, "R");
}

void FeedbackHandler(DDLayer *pLayer, DDFeedbackType type, const DDFeedback &feedback)
{
  if (pLayer == displayLayer) 
  {
    if (type == LONGPRESS)
    {
      calculator.reset();
      displayLayer->flash();
      UpdateCaculatorDisplay(displayLayer);
      dumbdisplay.tone(TONE_CLEAR, 300);
    }
  } 
  else 
  {
    if (type == CLICK)
    {
      char key = pLayer->customData.charAt(0);
      if (calculator.push(key)) {
        dumbdisplay.tone(TONE_YES, 100);
        if (key == '=') {
          dumbdisplay.tone(TONE_ENTER, 100);
        }
      } else {
        dumbdisplay.tone(TONE_NO, 200);
      }
      UpdateCaculatorDisplay(displayLayer);
    }
  }
}


LcdDDLayer *CreateDisplayLayer()
{
  LcdDDLayer *displayLayer = dumbdisplay.createLcdLayer(DisplayWidth, 1, 28, "sans-serif");
  displayLayer->backgroundColor("azure");
  displayLayer->pixelColor("black");
  displayLayer->border(5, "grey", "raised");
  displayLayer->padding(5);
  displayLayer->setFeedbackHandler(FeedbackHandler);
  UpdateCaculatorDisplay(displayLayer);
  return displayLayer;
}
LcdDDLayer *CreateKeyLayer(int r, int c)
{
  String key = String(Keys[r][c]);
  LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32, "sans-serif-black");
  keyLayer->pixelColor("navy");
  keyLayer->border(5, "grey", "raised");
  keyLayer->padding(1);
  keyLayer->writeLine(key);
  keyLayer->setFeedbackHandler(FeedbackHandler, "fl");
  keyLayer->customData = key;
  return keyLayer;
}
