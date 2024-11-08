#include "dumbdisplay.h"


// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput(57600));


TurtleDDLayer *turtle;
int r = random(0, 256);
int g = 128;
int b = 0;

void setup() {
    // create a Turtle layer with size 240 x 190
    turtle = dumbdisplay.createTurtleLayer(240, 190);
    // setup Turtle layer
    turtle->backgroundColor("azure");
    turtle->fillColor("lemonchiffon");
    turtle->penSize(1);
    turtle->penFilled(true);
    // initially draw something on the Turtle layer (will change some settings)
    turtle->centeredPolygon(70, 6, true);
    turtle->penFilled(false);
    turtle->circle(80, true);

    // create 4 LEDs -- left-top, right-top, right-bottom and left-bottom 
    LedGridDDLayer* ltLed = dumbdisplay.createLedGridLayer();
    LedGridDDLayer* rtLed = dumbdisplay.createLedGridLayer();
    LedGridDDLayer* rbLed = dumbdisplay.createLedGridLayer();
    LedGridDDLayer* lbLed = dumbdisplay.createLedGridLayer();

    // set LEDs background color
    ltLed->backgroundColor("green");
    rtLed->backgroundColor("green");
    rbLed->backgroundColor("green");
    lbLed->backgroundColor("green");

    // turn ON the LEDs
    ltLed->turnOn();
    rtLed->turnOn();
    rbLed->turnOn();
    lbLed->turnOn();

    // config "pin frame" to be 290 units x 250 units
    // 290: 25 + 240 + 25
    // 240: 25 + 190 + 25
    dumbdisplay.configPinFrame(290, 240);

    // pin top-left LED @ (0, 0) with size (25, 25)
    dumbdisplay.pinLayer(ltLed, 0, 0, 25, 25);
    // pin top-right LED @ (265, 0) with size (25, 25)
    dumbdisplay.pinLayer(rtLed, 265, 0, 25, 25);
    // pin right-bottom LED @ (265, 215) with size (25, 25)
    dumbdisplay.pinLayer(rbLed, 265, 215, 25, 25);
    // pin left-bottom LED @ (0, 215) with size (25, 25)
    dumbdisplay.pinLayer(lbLed, 0, 215, 25, 25);

    // pin Turtle @ (25, 25) with size (240, 190)
    dumbdisplay.pinLayer(turtle, 25, 25, 240, 190);
}


void loop() {
    delay(1000);
    turtle->penColor(DD_RGB_COLOR(r, g, b));
    turtle->circle(27);
    turtle->rectangle(90, 20);
    turtle->rightTurn(10);
    b = b + 20;
    if (b > 255) {
        b = 0;
        r = random(0, 256);
    }
}