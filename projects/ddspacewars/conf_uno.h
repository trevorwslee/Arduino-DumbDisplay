
//#define DOWNLOAD_IMAGES
//#define SHOW_SPACE
#define JOYSTICK_H_MIN_READING 10
#define JOYSTICK_H_MAX_READING 1013
#define JOYSTICK_V_MIN_READING 10
#define JOYSTICK_V_MAX_READING 1013
#define DEBUG_LED_PIN 13
const uint8_t BTN_A = 3;
const uint8_t BTN_B = 2;
// const uint8_t ENTER = 5;
const uint8_t HORIZONTAL = A0;
const uint8_t VERTICAL = A1;
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
