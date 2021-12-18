#include "dumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));
LedGridDDLayer *led;

void setup() {
    // create a LED layer
    led = dumbdisplay.createLedGridLayer();
}

void loop() {
    led->toggle();
    delay(1000);
}
