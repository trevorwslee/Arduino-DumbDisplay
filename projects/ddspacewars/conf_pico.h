

#if defined(FOR_PICOW)

//#define SAVE_IMAGES
#define DOWNLOAD_IMAGES
#define SHOW_SPACE
#define DEBUG_LED_PIN 1
#define JOYSTICK_H_MIN_READING 1000
#define JOYSTICK_H_MAX_READING 50
#define JOYSTICK_V_MIN_READING 50
#define JOYSTICK_V_MAX_READING 1000
const uint8_t BTN_A = 21;
const uint8_t BTN_B = 18;
const uint8_t HORIZONTAL = 26;
const uint8_t VERTICAL = 27;
// GP8 => RX of HC06; GP9 => TX of HC06
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include "picodumbdisplay.h"
DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));

#else

//#define SAVE_IMAGES
#define DOWNLOAD_IMAGES
#define SHOW_SPACE
#define DEBUG_LED_PIN 25
const uint8_t BTN_A = 16;
const uint8_t BTN_B = 15;
// const uint8_t ENTER = 14;
// GP8 => RX of HC06; GP9 => TX of HC06
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include "picodumbdisplay.h"
DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));


#endif