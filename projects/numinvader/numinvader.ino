#include <Arduino.h>
#include "dumbdisplay.h"


#define BUTTON_LEFT PIN_A1
#define BUTTON_RIGHT PIN_A4

#define LED_LEFT PIN_A2
#define LED_RIGHT PIN_A3


const int INVADER_COUNT = 10;
const long INVADE_INIT_STEP_MILLIS = 1000;
const long INVADE_STEP_FAST_MILLIS = 50;
const long MIN_INVALID_STEP_MILLIS = 500;

class ButtonPressTracker {
public:
  ButtonPressTracker() {
    this->pressed = false;  // assume initially not pressed
    this->blackOutMillis = 0;
  }
  int setPressed(bool pressed) {
    long nowMillis = millis();
    if (blackOutMillis != 0) {
      long diff = blackOutMillis - nowMillis;
      if (diff < 0) {
        blackOutMillis = 0;
      }
    }
    if (blackOutMillis == 0) {
      if (pressed != this->pressed) {
        this->pressed = pressed;
        blackOutMillis = nowMillis + 50;
        return this->pressed ? 1 : 0;
      }
    }
    return -1;
  }
private:
  bool pressed;
  long blackOutMillis;
};


class LaserGun {
public:
  void initialize() {
    ss->showNumber(variant);
  }
  void loop() {
    int left = digitalRead(BUTTON_LEFT);
    int right = digitalRead(BUTTON_RIGHT);
    bool leftPressed = left == 0;
    bool rightPressed = right == 0;
    int switchVariant = switchVariantTracker.setPressed(leftPressed);
    int fired = fireTracker.setPressed(rightPressed);
    if (switchVariant != -1) {
      if (switchVariant == 1) {
        variant = (variant + 1) % 10;
        ss->showNumber(variant);
        digitalWrite(LED_LEFT, 1);
      } else {
        digitalWrite(LED_LEFT, 0);
      }
    }
    if (fired != -1) {
      if (fired == 1) {
        digitalWrite(LED_RIGHT, 1);
      } else {
        digitalWrite(LED_RIGHT, 0);
      }
    }
  }
public:
  SevenSegmentRowDDLayer* ss;
  ButtonPressTracker switchVariantTracker;
  ButtonPressTracker fireTracker;
  char variant;  // 0 to 9
};

class Invaders {
public:
  void initialize() {
    activeInvaderCount = 0;
    activeInvaders[0] = 0;
    invalidStepMillis = INVADE_INIT_STEP_MILLIS;
    nextInvadeMillis = millis() + invalidStepMillis;
  }
public:
  void loop() {
    long nowMillis = millis();
    long diffMillis = nextInvadeMillis - nowMillis;
    if (diffMillis <= 0) {
      if (activeInvaderCount < INVADER_COUNT) {
        activeInvaders[activeInvaderCount++] = '.';
      }
      ss->showFormatted(activeInvaders);
      invalidStepMillis -= INVADE_STEP_FAST_MILLIS;
      if (invalidStepMillis < MIN_INVALID_STEP_MILLIS) {
        invalidStepMillis = MIN_INVALID_STEP_MILLIS;
      }
      nextInvadeMillis = nowMillis + invalidStepMillis;
    }
  }
public:
  SevenSegmentRowDDLayer* ss;
  int activeInvaderCount;
  char activeInvaders[INVADER_COUNT + 1];
  long invalidStepMillis;
  long nextInvadeMillis;
};

class Controller {
public:
  Controller(LaserGun& laserGun, Invaders& invaders)
    : laserGun(laserGun), invaders(invaders) {
  }
public:
  void initialize() {
    laserGun.initialize();
    invaders.initialize();
  }
  void loop() {
    laserGun.loop();
    invaders.loop();
  }
private:
  LaserGun& laserGun;
  Invaders& invaders;
};

DumbDisplay dumbdisplay(new DDInputOutput(115200));

LaserGun laserGun;
Invaders invaders;
Controller controller(laserGun, invaders);


void setup() {
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);

  laserGun.ss = dumbdisplay.create7SegmentRowLayer(1);
  laserGun.ss->border(5, "black", "round");
  laserGun.ss->padding(5);
  laserGun.ss->resetSegmentOffColor(DD_RGB_COLOR(0xff, 0xee, 0xff));

  invaders.ss = dumbdisplay.create7SegmentRowLayer(INVADER_COUNT);
  invaders.ss->border(5, "black", "round");
  invaders.ss->padding(5);
  invaders.ss->resetSegmentOffColor(DD_RGB_COLOR(0xff, 0xee, 0xff));

  dumbdisplay.configAutoPin(DD_AP_HORI);

  controller.initialize();
}


void loop() {
  controller.loop();
}