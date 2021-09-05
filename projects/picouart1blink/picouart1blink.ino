#define DD_4_PICO

#include <picodumbdisplay.h>

/* HC-06 connectivity */
DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));
LedGridDDLayer *led;

void setup() {
    // create a LED layer
    led = dumbdisplay.createLedGridLayer();
}

void loop() {
    led->toggle();
    delay(1000);
}
