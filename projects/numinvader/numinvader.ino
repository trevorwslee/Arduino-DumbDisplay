#include <Arduino.h>
#include "dumbdisplay.h"


#define BUTTON_LEFT PIN_A1
#define BUTTON_RIGHT PIN_A4

#define LED_LEFT PIN_A2
#define LED_RIGHT PIN_A3


const int INIT_LASER_GUN_COUNT = 3;
const int MAX_INVADER_COUNT = 8;
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
    lifeCount = INIT_LASER_GUN_COUNT;
    life7Seg->showDigit(lifeCount);
    laserGun7Seg->showDigit(variant);
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
        laserGun7Seg->showDigit(variant);
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
  SevenSegmentRowDDLayer* life7Seg;
  SevenSegmentRowDDLayer* laserGun7Seg;
  ButtonPressTracker switchVariantTracker;
  ButtonPressTracker fireTracker;
  int lifeCount;
  char variant;  // 0 to 9
};

class Invaders {
public:
  void initialize() {
    variantCount = 0;
    variants[MAX_INVADER_COUNT] = 0;
    invalidStepMillis = INVADE_INIT_STEP_MILLIS;
    nextInvadeMillis = millis() + invalidStepMillis;
  }
public:
  boolean loop() {
    long nowMillis = millis();
    long diffMillis = nextInvadeMillis - nowMillis;
    if (diffMillis <= 0) {
      char *frontVariant = variants + MAX_INVADER_COUNT - variantCount;  // it is not only pointing the the first, it is all the variants as a string
      if (variantCount < MAX_INVADER_COUNT) {
        if (variantCount > 0) {
          memmove(frontVariant - 1, frontVariant, variantCount);  // move the front variant (and all that after it) to the left side by 1
        }
        frontVariant -= 1;
        variantCount += 1;
        variants[MAX_INVADER_COUNT - 1] = '0' + random(0, 10);  // the right-most now has space for a new invader variant
      }
      int shiftIdx = MAX_INVADER_COUNT - variantCount;  // we only have that many variants, so, shift it 
      invader7Seg->showFormatted(frontVariant, shiftIdx);  // print the variants; notice of 'shift idx'
      invalidStepMillis -= INVADE_STEP_FAST_MILLIS;
      if (invalidStepMillis < MIN_INVALID_STEP_MILLIS) {
        invalidStepMillis = MIN_INVALID_STEP_MILLIS;
      }
      nextInvadeMillis = nowMillis + invalidStepMillis;
    }
    return variantCount < MAX_INVADER_COUNT;
  }
public:
  SevenSegmentRowDDLayer* invader7Seg;
  int variantCount;
  char variants[MAX_INVADER_COUNT + 1];
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


  laserGun.life7Seg = dumbdisplay.create7SegmentRowLayer(1);
  laserGun.life7Seg->segmentColor("gold");
  laserGun.life7Seg->border(5, "gold", "round");
  laserGun.life7Seg->padding(5);

  laserGun.laserGun7Seg = dumbdisplay.create7SegmentRowLayer(1);
  laserGun.laserGun7Seg->segmentColor("blue");
  laserGun.laserGun7Seg->backgroundColor("yellow");
  laserGun.laserGun7Seg->border(5, "blue", "round");
  laserGun.laserGun7Seg->padding(5);

  invaders.invader7Seg = dumbdisplay.create7SegmentRowLayer(MAX_INVADER_COUNT);
  invaders.invader7Seg->segmentColor("red");
  invaders.invader7Seg->border(5, "red", "round");
  invaders.invader7Seg->padding(5);

  dumbdisplay.configAutoPin(DD_AP_HORI);

  controller.initialize();
}


void loop() {
  controller.loop();
}