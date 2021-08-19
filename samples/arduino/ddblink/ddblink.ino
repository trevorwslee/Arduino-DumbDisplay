#include <dumbdisplay.h>

/* please use DumbDisplayWiFiBridge for connection */
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
