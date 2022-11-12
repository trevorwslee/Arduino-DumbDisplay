#if defined(FOR_LILIGO)

//#define SAVE_IMAGES
#define DOWNLOAD_IMAGES
#define SHOW_SPACE
#define DEBUG_LED_PIN 2
const uint8_t BTN_A = 0; // BOOT
const uint8_t BTN_B = 35;
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("LILIGO", true, 115200));

#else

//#define SAVE_IMAGES
#define DOWNLOAD_IMAGES
#define SHOW_SPACE
#define DEBUG_LED_PIN 2
const uint8_t BTN_A = 0; // BOOT
const uint8_t BTN_B = 35;
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true, 115200));


#endif