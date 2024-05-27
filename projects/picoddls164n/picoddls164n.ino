
// for a desciption of the Shift Register experiment, please watch the YouTube video 
// -- Raspberry Pi Pico Shift Register Arduino Experiment with HC-06 and DumpDisplay, and 8x8 LED Matrix
// -- https://www.youtube.com/watch?v=qNESLsdzsWk

// comment out BLUETOOTH if don't have HC-06 connectivity
// . GP8 => RX of HC-06; GP9 => TX of HC-06; ; HC-06 should be configured to use baud rate of 115200
// if no HC-06 connectivity, will need to use DumbDisplayWifiBridge
#define BLUETOOTH

#define SHIFT_PIN   6
#define CLEAR_PIN   26
#define CLOCK_PIN   27


#define PULSE_MILLIS           2
#define AUTO_DELAY_STEP_MILLIS 2
#define AUTO_DELAY_STEP_FACTOR 2
#define MAX_AUTO_DELAY_STEPS   50





#ifdef BLUETOOTH

// GP8 => RX of HC-06; GP9 => TX of HC-06
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include "picodumbdisplay.h"
DumbDisplay dumbdisplay(new DDPicoSerialIO(115200));

#else

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif


// pins almost continuous from 11; however, 15 is skipped since it is used by the board for other purpose
int GetRegisterPin(int idx) {
  int pin = 11 + idx;
  if (pin >= 15) {
    pin++;
  }
  return pin;
}




LedGridDDLayer *shiftBitLayer;
LedGridDDLayer *registerLayer;

LcdDDLayer *clockLayer;
LcdDDLayer *clearLayer;
LcdDDLayer *autoLayer;

LedGridDDLayer *autoDelayLayer;


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);


void setup() {
  Serial.begin(115200);

  pinMode(SHIFT_PIN, OUTPUT);
  pinMode(CLEAR_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  for (int i = 0; i < 8; i++) {
    pinMode(GetRegisterPin(i), INPUT);
  }

  digitalWrite(CLOCK_PIN, 0);
  digitalWrite(CLEAR_PIN, 1);

  autoDelayLayer = dumbdisplay.createLedGridLayer(MAX_AUTO_DELAY_STEPS, 1, 1, 5);
  autoDelayLayer->border(1, "green", "round");
  autoDelayLayer->padding(1);
  autoDelayLayer->horizontalBar(MAX_AUTO_DELAY_STEPS);
  autoDelayLayer->setFeedbackHandler(FeedbackHandler, "fs:rpt50");


  shiftBitLayer = dumbdisplay.createLedGridLayer();
  shiftBitLayer->border(0.2, "blue");
  shiftBitLayer->offColor("lightgray");
  shiftBitLayer->setFeedbackHandler(FeedbackHandler);

  registerLayer = dumbdisplay.createLedGridLayer(8);
  registerLayer->border(0.2, "gray");
  registerLayer->offColor("lightgray");

  clockLayer = dumbdisplay.createLcdLayer(11, 1);
  clockLayer->writeCenteredLine("Clock");
  clockLayer->border(2, "black", "round");
  clockLayer->setFeedbackHandler(FeedbackHandler, "f");

  clearLayer = dumbdisplay.createLcdLayer(9, 1);
  clearLayer->writeCenteredLine("Clear");
  clearLayer->border(2, "black", "round");
  clearLayer->setFeedbackHandler(FeedbackHandler, "f");

  autoLayer = dumbdisplay.createLcdLayer(8, 1);
  autoLayer->writeCenteredLine("Auto");
  autoLayer->border(2, "black", "round");
  autoLayer->noBackgroundColor();
  autoLayer->setFeedbackHandler(FeedbackHandler);


  dumbdisplay.configAutoPin(DD_AP_VERT_4(
    autoDelayLayer->getLayerId(),
    DD_AP_HORI_2(autoLayer->getLayerId(), clearLayer->getLayerId()),
    DD_AP_HORI_2(shiftBitLayer->getLayerId(), registerLayer->getLayerId()),
    clockLayer->getLayerId()
  ));
}


int shiftBit = 1;  // -1 means auto
int autoDelaySteps = 100;
DDValueStore<int16_t> shiftBitValue(0);
DDValueStore<int16_t> registerValue(0);


void Clock() {
  digitalWrite(CLOCK_PIN, 1);
  delay(PULSE_MILLIS);
  digitalWrite(CLOCK_PIN, 0);
}
void Clear() {
    digitalWrite(CLEAR_PIN, 0);
    delay(PULSE_MILLIS);
    digitalWrite(CLEAR_PIN, 1);
}
void SetShiftBit(int bit) {
  if (shiftBitValue.set(bit)) {
    digitalWrite(SHIFT_PIN, bit);
    shiftBitLayer->bitwise(bit);
  }
}
void ShowRegister() {
  int16_t reg = 0;
  for (int i = 0; i < 8; i++) {
    int pin = GetRegisterPin(i);
    int v = digitalRead(pin);
    reg = reg << 1;
    if (v == 1) {
      reg = reg + 1;
    }
  }
  if (registerValue.set(reg)) {
    Serial.println(reg);
    registerLayer->bitwise(reg);
  }
}


void loop() {
  if (shiftBit != -1) {
    // manual mode
    SetShiftBit(shiftBit);
    ShowRegister();
    DDYield();
  }
  else {
    // auto mode
    int shiftCount = 0;
    while (shiftBit == -1) {
      ShowRegister();
      int bit = (shiftCount == 0) ? 1 : 0;
      SetShiftBit(bit);
      long delayMillis = autoDelaySteps * AUTO_DELAY_STEP_FACTOR * AUTO_DELAY_STEP_MILLIS;
      DDDelay(delayMillis);
      Clock();
      if (shiftCount == 7) {
        shiftCount = 0;
      } else {
        shiftCount = shiftCount + 1;
      }
    }
  }
}

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
  if (shiftBit != -1) {
    // manual mode
    if (pLayer == shiftBitLayer) {
      if (shiftBit == 0) {
        shiftBit = 1;
      } else {
        shiftBit = 0;
      }
    } else if (pLayer == clockLayer) {
      Clock();
    } else if (pLayer == clearLayer) {
      Clear();
    } else if (pLayer == autoLayer) {
      shiftBit = -1;
      Clear();
      autoLayer->backgroundColor("lime");
    }
  } else {
    // auto mode
    if (pLayer == autoDelayLayer) {
      int steps = feedback.x;
      autoDelayLayer->horizontalBar(steps);
      autoDelaySteps = steps;
    } else {
      shiftBit = 1;
      Clear();
      autoLayer->noBackgroundColor();
    }
  }
}
