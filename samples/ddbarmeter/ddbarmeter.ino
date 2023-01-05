#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput(57600));


void setup() {
    // configure to "auto pin (layout) layers" in the vertical direction
    dumbdisplay.configAutoPin(DD_AP_VERT);
    
    // create a LED layer
    LedGridDDLayer *led = dumbdisplay.createLedGridLayer(3, 1);
    // turn on LEDs
    led->onColor("red");
    led->turnOn(0, 0);
    led->onColor("green");
    led->turnOn(1, 0);
    led->onColor("blue");
    led->turnOn(2, 0);

    // create LED layers that will be used for "horizontal bar-meter"
    // with max "bar" size 32
    LedGridDDLayer *barmeter = dumbdisplay.createLedGridLayer(32, 1, 1, 5);
    barmeter->onColor("darkblue");
    barmeter->offColor("lightgreen");
    // set the "bar" to 10 (ie. 10 of total 32)
    barmeter->horizontalBar(10);


    // create a LCD layer
    LcdDDLayer * lcd = dumbdisplay.createLcdLayer();
    // write to LCD write messages (as lines)
    // notice that "C" means center-align
    lcd->writeLine("Hello There!", 0, "C");
    lcd->writeLine("How are you?", 1, "C");

}

void loop() {
}


