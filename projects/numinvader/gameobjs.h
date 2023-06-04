#include "tracker.h"  // for ButtonPressTracker  
#include "sound.h" // for playing sound effect


const int INIT_LASER_GUN_COUNT = 3;
const int MAX_INVADER_COUNT = 8;
const long INVADE_INIT_STEP_MILLIS = 2000;
const long INVADE_STEP_FAST_MILLIS = 50;
const long MIN_INVALID_STEP_MILLIS = 500;


class LaserGun {
public:
  void initialize() {
    lifeCount = INIT_LASER_GUN_COUNT;
    variantType = 0;
    life7Seg->showDigit(lifeCount);
    laserGun7Seg->showDigit(variantType);
  }
  // return the fired 'variant type'; or -1
  int loop() {
    int left = digitalRead(BUTTON_LEFT);
    int right = digitalRead(BUTTON_RIGHT);
    bool leftPressed = left == 0;
    bool rightPressed = right == 0;
    int switchVariant = switchVariantTracker.setPressed(leftPressed);
    int fired = fireTracker.setPressed(rightPressed);
    if (switchVariant != -1) {
      if (switchVariant == 1) {
        variantType = (variantType + 1) % 10;
        laserGun7Seg->showDigit(variantType);
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
      if (fired == 1) {
        return variantType;
      }
    }
    return -1;
  }
  bool lostOne() {
    if (lifeCount > 0) {
      lifeCount -= 1;
      life7Seg->showDigit(lifeCount);
      PlayMissLifeSoundEffect();    
      if (lifeCount == 0) {
        PlayEndGameSoundEffect();
      }  
    }
    return lifeCount > 0;
  }
  void reset() {
    lifeCount = INIT_LASER_GUN_COUNT;
    life7Seg->showDigit(lifeCount);
  }
public:
  SevenSegmentRowDDLayer* life7Seg;
  SevenSegmentRowDDLayer* laserGun7Seg;
  ButtonPressTracker switchVariantTracker;
  ButtonPressTracker fireTracker;
  int lifeCount;
  char variantType;  // 0 to 9
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
  // return true if not dead
  bool loop() {
    if (variantCount == -1) {
      return false;
    }
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
      invader7Seg->showFormatted(frontVariant, true, shiftIdx);  // print the variants; notice of 'shift idx'
      nextInvadeMillis = nowMillis + invalidStepMillis;
    }
    return variantCount < MAX_INVADER_COUNT;
  }
  bool firedAt(char variantType) {
    if (variantCount > 0) {
      char *frontVariant = variants + MAX_INVADER_COUNT - variantCount;
      char frontVariantType = *frontVariant - '0';
      if (variantType == frontVariantType) {
        int shiftIdx = MAX_INVADER_COUNT - variantCount;  // we only have that many variants, so, shift it 
        invader7Seg->showFormatted("-", false, shiftIdx);
        PlayHitSoundEffect();
        variantCount -= 1;
        invalidStepMillis -= INVADE_STEP_FAST_MILLIS;
        if (invalidStepMillis < MIN_INVALID_STEP_MILLIS) {
          invalidStepMillis = MIN_INVALID_STEP_MILLIS;
        }
        nextInvadeMillis = millis() + 200;
        return true;
      } else {
        PlayMissSoundEffect();
      }
    }
    return false;
  }
  void reset() {
    variantCount = 0;
    invalidStepMillis = INVADE_INIT_STEP_MILLIS;
    nextInvadeMillis = millis() + invalidStepMillis;
    invader7Seg->clear();    
  }
  void end() {
    if (variantCount > 0) {
      variantCount = -1;  // signal game ended
      for (int i = 0; i < MAX_INVADER_COUNT; i++) {
        invader7Seg->showFormatted("_", false, i);
      }
    }
  }
  inline bool ended() {
    return variantCount == -1;
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
    randomSeed(millis());
    laserGun.initialize();
    invaders.initialize();
  }
  void loop() {
    int firedVariantType = laserGun.loop();
    if (firedVariantType != -1) {
      if (invaders.ended()) {
        laserGun.reset();
        invaders.reset();
      } else {
        invaders.firedAt(firedVariantType);
      }
    }
    if (!invaders.loop()) {
      if (laserGun.lostOne()) {
        invaders.reset();
      } else {
        invaders.end();
      }
    }
  }
private:
  LaserGun& laserGun;
  Invaders& invaders;
};
