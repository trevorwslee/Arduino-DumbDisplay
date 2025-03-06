//#define WITH_KEYBOARD


#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true, 115200), DD_DEF_SEND_BUFFER_SIZE);
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD), DD_DEF_SEND_BUFFER_SIZE);
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(), DD_DEF_SEND_BUFFER_SIZE);
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

void UpdateCalculatorDisplay();
SevenSegmentRowDDLayer *CreateDisplayLayer();
LcdDDLayer *CreateKeyLayer(int r, int c);

SevenSegmentRowDDLayer *displayLayer;
LcdDDLayer *keyLayers[RowCount][5];

BasicCalculator calculator(DisplayWidth);


#if defined(WITH_KEYBOARD)
#include <Wire.h>
void OnReceivedKey(int param);
#endif






void setup()
{
#if defined(WITH_KEYBOARD) 
  Wire.begin(4);
  Wire.onReceive(OnReceivedKey);
#endif 

  dumbdisplay.setFeedbackSingleClickOnly();

  displayLayer = CreateDisplayLayer();
  for (int r = 0; r < RowCount; r++)
  {
    for (int c = 0; c < ColCount; c++)
    {
      LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
      keyLayers[r][c] = keyLayer;
    }
  }
  UpdateCalculatorDisplay();
  dumbdisplay.configAutoPin(
      DD_AP_VERT_2(
          displayLayer->getLayerId(),
          DD_AP_VERT_5(
              DD_AP_HORI_4(keyLayers[0][0]->getLayerId(), keyLayers[0][1]->getLayerId(), keyLayers[0][2]->getLayerId(), keyLayers[0][3]->getLayerId()),
              DD_AP_HORI_4(keyLayers[1][0]->getLayerId(), keyLayers[1][1]->getLayerId(), keyLayers[1][2]->getLayerId(), keyLayers[1][3]->getLayerId()),
              DD_AP_HORI_4(keyLayers[2][0]->getLayerId(), keyLayers[2][1]->getLayerId(), keyLayers[2][2]->getLayerId(), keyLayers[2][3]->getLayerId()),
              DD_AP_HORI_4(keyLayers[3][0]->getLayerId(), keyLayers[3][1]->getLayerId(), keyLayers[3][2]->getLayerId(), keyLayers[3][3]->getLayerId()),
              DD_AP_HORI_4(keyLayers[4][0]->getLayerId(), keyLayers[4][1]->getLayerId(), keyLayers[4][2]->getLayerId(), keyLayers[4][3]->getLayerId()))));

}

void loop()
{
  DDYield(); // need to call this so that DumbDisplay lib can check for "feedback"
}

void UpdateCalculatorDisplay()
{
  if (calculator.isGrouping())
  {
    displayLayer->backgroundColor(DD_COLOR_cyan);
  }
  else
  {
    displayLayer->backgroundColor(DD_COLOR_azure);
  }
  const char *formatted = calculator.getFormatted();
  displayLayer->showFormatted(formatted, true, DisplayWidth - strlen(formatted));
  if (true)
  {
    dumbdisplay.writeComment(calculator.getFormattedEx());
  }
}

void FeedbackHandler(DDLayer *pLayer, DDFeedbackType type, const DDFeedback &feedback)
{
  if (pLayer == displayLayer)
  {
    // if (type == DDFeedbackType::LONGPRESS)
    // {
    //   displayLayer->flash();
    //   calculator.reset();
    //   dumbdisplay.tone(TONE_CLEAR, 300);
    //   UpdateCalculatorDisplay();
    // }
  }
  else
  {
    if (type == DDFeedbackType::CLICK)
    {
      char key = pLayer->customData.charAt(0);
      if (false) {
        dumbdisplay.writeComment(String(key));
        return;
      }
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
      UpdateCalculatorDisplay();
    }
  }
}

SevenSegmentRowDDLayer *CreateDisplayLayer()
{
  SevenSegmentRowDDLayer *displayLayer = dumbdisplay.create7SegmentRowLayer(DisplayWidth); // .createLcdLayer(DisplayWidth, 1, 28, "sans-serif");
  displayLayer->segmentColor("darkblue");
  displayLayer->border(50, "grey", "raised");
  displayLayer->padding(50);
  //displayLayer->setFeedbackHandler(FeedbackHandler);  // single click only
  return displayLayer;
}
LcdDDLayer *CreateKeyLayer(int r, int c)
{
  const char key = Keys[r][c];
  String dispKey = String(key);
#if defined(__AVR__) || defined(ESP32)  
  if (key == '*')
  {
    dispKey = "×";
  }
  else if (key == '/')
  {
    dispKey = "÷";
  }
  else if (key == '+')
  {
    dispKey = "+";
  }
  else if (key == '-')
  {
    dispKey = "−";
  }
#endif 
  LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32, "sans-serif-black");
  keyLayer->pixelColor("navy");
  keyLayer->border(5, "grey", "raised");
  keyLayer->padding(1);
  keyLayer->writeLine(dispKey);
  keyLayer->setFeedbackHandler(FeedbackHandler, "fl");
  //keyLayer->enableFeedback("fl");
  keyLayer->customData = String(key);
  return keyLayer;
}

#if defined(WITH_KEYBOARD)
void OnReceivedKey(int param) {
  char x = Wire.read();
  dumbdisplay.writeComment(String(x));
}
#endif


