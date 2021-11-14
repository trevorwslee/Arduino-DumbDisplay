#define DD_4_ESP32
#include "esp32dumbdisplay.h"


#define NOISE_PIN 15
#define NOISE_PIN_NUM GPIO_NUM_15

#define TOUCH_PIN 4 
#define TOUCH_PIN_NUM T0
#define TOUCH_THRESHOLD 40

// uncommend out if want DumbDisplay to go to sleep when 'idle'
//#define AUTO_SLEEP_WHEN_IDEL_MILLIS 10000




DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));


LedGridDDLayer* detected;
PlotterDDLayer* plotter;

LcdDDLayer* just;
LcdDDLayer* inFive;
LcdDDLayer* touch;
LcdDDLayer* noise;

#ifdef AUTO_SLEEP_WHEN_IDEL_MILLIS
void IdleCallback(long idleForMillis) {
  if (idleForMillis > AUTO_SLEEP_WHEN_IDEL_MILLIS) {
    esp_sleep_enable_ext0_wakeup(NOISE_PIN_NUM, 0);
    esp_deep_sleep_start();
  }
}
#endif


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
  if (pLayer == just) {
    esp_deep_sleep_start();
  } else if (pLayer == inFive) {
    esp_sleep_enable_timer_wakeup(5 * 1000 * 1000);  // wake up in 5 seconds
    esp_deep_sleep_start();
  } else if (pLayer == touch) {
    esp_sleep_enable_touchpad_wakeup();
    esp_deep_sleep_start();
  } else if (pLayer == noise) {
    esp_sleep_enable_ext0_wakeup(NOISE_PIN_NUM, 0);
    esp_deep_sleep_start();
  }
}

LcdDDLayer* CreateButton(const String& label) {
  LcdDDLayer* button = dumbdisplay.createLcdLayer(16, 1);
  button->border(3, "blue", "raised");
  button->pixelColor("darkred");
  button->noBackgroundColor();
  button->padding(1);
  button->writeLine(label, 0, "C");
  button->setFeedbackHandler(FeedbackHandler, "fl");
  return button;
}


volatile long noisyMillis = 0;

void DetectedNoise() {
  noisyMillis = millis();
}



void setup() {
  pinMode(NOISE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(NOISE_PIN), DetectedNoise, RISING);
  touchAttachInterrupt(TOUCH_PIN, [](){}, TOUCH_THRESHOLD);

#ifdef AUTO_SLEEP_WHEN_IDEL_MILLIS
  dumbdisplay.setIdleCalback(IdleCallback);
#endif

  detected = dumbdisplay.createLedGridLayer(2, 1);
  detected->onColor("navy");
  detected->offColor("lightgray");

  plotter = dumbdisplay.createPlotterLayer(300, 60);
  plotter->border(2, "blue", "round");

  just = CreateButton("just sleep");
  inFive = CreateButton("wake in 5s");
  touch = CreateButton("wake by touch");
  noise = CreateButton("wake by noise");


  dumbdisplay.configAutoPin(
    DD_AP_VERT_5(
      DD_AP_HORI_2(detected->getLayerId(), plotter->getLayerId()),
      just->getLayerId(),
      inFive->getLayerId(),
      touch->getLayerId(),
      noise->getLayerId()
    ));

  dumbdisplay.writeComment("");
  dumbdisplay.writeComment("Good Day! It is a new start!");
  dumbdisplay.writeComment("");
}


int counter = 0;
long lastMillis = -1;

bool wasTouched = false;
bool wasNoisy = false;



void loop() {
  if (lastMillis == -1 || (millis() - lastMillis) >= 1000) {
    int count = ++counter;
    plotter->set(count);
    if (count % 2 == 0) {
      dumbdisplay.backgroundColor("lightyellow");
    } else {
      dumbdisplay.backgroundColor("lightgreen");
    }
    lastMillis = millis();
  }

  int touchValue = touchRead(TOUCH_PIN);
  bool touched = touchValue < TOUCH_THRESHOLD;
  if (touched != wasTouched) {
    if (touched) {
      detected->turnOn(0);
    } else {
      detected->turnOff(0);
    }
    wasTouched = touched;
  }

  bool noisy = (millis() - noisyMillis) < 200;
  if (noisy != wasNoisy) {
    if (noisy) {
      detected->turnOn(1);
    } else {
      detected->turnOff(1);
    }
    wasNoisy = noisy;
  }


  DDYield();
}

