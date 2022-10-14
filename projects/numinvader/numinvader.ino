#include <Arduino.h>
#include "dumbdisplay.h"


#define BUTTON_LEFT PIN_A1
#define BUTTON_RIGHT PIN_A4

#define LED_LEFT PIN_A2
#define LED_RIGHT PIN_A3


const int INVADER_COUNT = 10;
const long INVADE_INIT_STEP_MILLIS = 1000;
const long INVADE_STEP_FAST_MILLIS = 20;
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
    variantCount = 0;
    variants[INVADER_COUNT] = 0;
    invalidStepMillis = INVADE_INIT_STEP_MILLIS;
    nextInvadeMillis = millis() + invalidStepMillis;
  }
public:
  boolean loop() {
    long nowMillis = millis();
    long diffMillis = nextInvadeMillis - nowMillis;
    if (diffMillis <= 0) {
      char *frontVariant = variants + INVADER_COUNT - variantCount;  // it is not only pointing the the first, it is all the variants as a string
      if (variantCount < INVADER_COUNT) {
        if (variantCount > 0) {
          memmove(frontVariant - 1, frontVariant, variantCount);  // move the front variant (and all that after it) to the left side by 1
        }
        frontVariant -= 1;
        variantCount += 1;
        variants[INVADER_COUNT - 1] = '0' + random(0, 10);  // the right-most now has space for a new invader variant
      }
      int shiftIdx = INVADER_COUNT - variantCount;  // we only have that many variants, so, shift it 
      ss->showFormatted(frontVariant, shiftIdx);  // print the variants; notice of 'shift idx'
      invalidStepMillis -= INVADE_STEP_FAST_MILLIS;
      if (invalidStepMillis < MIN_INVALID_STEP_MILLIS) {
        invalidStepMillis = MIN_INVALID_STEP_MILLIS;
      }
      nextInvadeMillis = nowMillis + invalidStepMillis;
    }
    return variantCount < INVADER_COUNT;
  }
public:
  SevenSegmentRowDDLayer* ss;
  int variantCount;
  char variants[INVADER_COUNT + 1];
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