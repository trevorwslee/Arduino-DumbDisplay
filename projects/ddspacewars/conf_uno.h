
//#define DOWNLOAD_IMAGES
//#define SHOW_SPACE
#define DEBUG_LED_PIN 13

#define WITH_JOYSTICK
const bool joystickReverseHoriDir = false;
const bool joystickReverseVertDir = false;
const bool joystickAutoTune = true;

const uint8_t BTN_A = 3;
const uint8_t BTN_B = 2;
const uint8_t HORIZONTAL = A0;
const uint8_t VERTICAL = A1;
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
