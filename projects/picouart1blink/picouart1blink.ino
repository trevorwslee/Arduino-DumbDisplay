#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9

#include "picodumbdisplay.h"

/* HC-06 connectivity */
DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));
LedGridDDLayer *led;

void setup() {
    // create a LED layer
    led = dumbdisplay.createLedGridLayer();
}

void loop() {
    led->toggle();
    delay(1000);
}
