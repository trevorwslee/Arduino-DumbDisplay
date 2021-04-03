#define DD_4_ESP8266
#include "wifidumbdisplay.h"

DumbDisplay dumbdisplay(new DDWiFiServerIO("ssid", "password", 10201));
LedGridDDLayer *ledLayer;

void setup() {
    // create a LED layer with 10 x 5 LEDs
    ledLayer = dumbdisplay.createLedGridLayer(10, 5);
    // setup that when LED is off, it's color is lightgray
    ledLayer->offColor("lightgray");
    // enable "feedback"
    ledLayer->enableFeedback();
}

void loop() {
    // check for "feedback"
    const DDFeedback *feedback = ledLayer->getFeedback();
    if (feedback != NULL) {
        // act upon the "feedback" by toggling the clicked LED
        ledLayer->toggle(feedback->x, feedback->y); 
    }
}



