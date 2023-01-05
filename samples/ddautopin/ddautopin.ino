#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput(57600));


LedGridDDLayer *rled;
LedGridDDLayer *gled;
LedGridDDLayer *bled;
LedGridDDLayer *hmeter;
LedGridDDLayer *vmeter;
LcdDDLayer *lcd;

int count = 20;

void setup() {
  // create R + G + B LED layers
  rled = dumbdisplay.createLedGridLayer();
  gled = dumbdisplay.createLedGridLayer();
  bled = dumbdisplay.createLedGridLayer();

  // create LED layers that will be used for "horizontal bar-meter"
  hmeter = dumbdisplay.createLedGridLayer(2 * count, 1, 1, 5);
  // create LED layers that will be used for "vertical bar-meter"
  vmeter = dumbdisplay.createLedGridLayer(1, 2 * count, 5, 1);
 
  // create a LCD layers with 2 rows of 16 characters
  lcd = dumbdisplay.createLcdLayer(16, 2);

  // configure to "auto pin" the different layers 
  // -- end result of DD_AP_XXX(...) is the layout spec "H(V(0+1+2)+V(3+5)+4)"
  // -- . H/V: layout direction
  // -- . 0/1/2/3/4/5: layer id
  dumbdisplay.configAutoPin(DD_AP_HORI_3(
                              DD_AP_VERT_3(rled->getLayerId(), gled->getLayerId(), bled->getLayerId()),
                              DD_AP_VERT_2(hmeter->getLayerId(), lcd->getLayerId()),
                              vmeter->getLayerId()));

  
  // setup RGB leds color and turn them on
  rled->onColor("red");
  gled->onColor("green");
  bled->onColor("blue");
  rled->turnOn();
  gled->turnOn();
  bled->turnOn();

  // set "bar meters" colors
  hmeter->onColor("blue");
  hmeter->offColor("yellow");
  hmeter->backgroundColor("black");
  vmeter->onColor("green");
  vmeter->offColor("lightgray");
  vmeter->backgroundColor("blue");

  // set LCD colors and print out something
  lcd->pixelColor("red");
  lcd->bgPixelColor("lightgreen");
  lcd->backgroundColor("black");
  lcd->print("hello world");  
  lcd->setCursor(0, 1);
  lcd->print("how are you?");
}

void loop() {
  delay(1000);
  if (random(2) == 0) {
    lcd->scrollDisplayLeft();
    count--;
  } else {  
    lcd->scrollDisplayRight();
    count++;
  }
  hmeter->horizontalBar(count);
  vmeter->verticalBar(count);
  if (random(2) == 0)
    rled->toggle();
  if (random(2) == 0)
    gled->toggle();
  if (random(2) == 0)
    bled->toggle();
}


