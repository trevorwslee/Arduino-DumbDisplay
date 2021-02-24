# DumbDisplay Arduino Library (v0.4.1)

DumbDisplay Ardunio Library enables you to utilize your Android phone as virtual output gadgets (as well as some very simple virtual input gadgets) for your Arduino / ESP32 experiments.


# Description

Instead of connecting real gadgets to your Arduino for outputing experiment results, you can make use of DumbDisplay for the purpose, to realize virtual gadagets on your Android phone.

Doing so you may defer buying / connecting real output gadgets until later stage of your experiment; also, you should be able to save a few Arduino pins for other experiment needs.

A few types of output layers can be created:
* LED-grid, which can also be used to simulate "bar-meter"
* LCD (text-based and graphical)
* Micro:bit-like canvas
* Turtle-like canvas
* Graphical LCD, which is derived from the Turtle layer (i.e. in addition to general feaures of graphical LCD, it also has Turtle-like features) 
* 7-Segment-row, which can be used to display a series of digits, plus a decimal dot

Notice that with the new "layer feedback" mechanism, user interaction (clicking of layers) can be routed to Arduino, and as a result, the layers can be used as input gadgets as well.


You can install the free DumbDisplay app (v0.4.1 or later) from Android Play Store -- https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay

The app can accept connection via
* SoftwareSerial (e.g. Bluetooth via HC-06)
* Serial (USB connected via OTG adapter)

Notes:
* Sorry that since I only have Arduino Uno, therefore the library is only tested with Arduino Uno (and partly with ESP32).
* In case DumbDisplay does not "handshake" with your Arduion correctly, you can try resetting your Adruino by pressing the "reset" button on your Adruion


# Sample Code

For Arduino, you have two options for connecting the DumbDisplay Android app.

* Via Serial 
  ```
    #include <dumbdisplay.h>
    DumbDisplay dumbdisplay(new DDInputOutput());
  ```
  - need to include dumbdisplay.h -- `#include <dumbdisplay.h>`
  - setup a `dumbdisplay` object-- `DumbDisplay dumbdisplay(new DDInputOutput())`
  - doing so will **automatically set Serial baud rate to 115200**, and **you should not be using Serial for other purposes**
* Via SoftwareSerial
  ```
    #include <ssdumbdisplay.h>
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3)));
  ```
  - need to include ssdumbdisplay.h -- `#include <ssdumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3)))`  
    - 2 and 3 are the pins used by SoftwareSerial
    - **the default baud rate is 115200**, which seems to work better from my own testing [with HC-06]
  - **You should not be using that SoftwareSerial for other purposes**
* Via **ESP32** BluetoothSerial (Experimental)
  ```
    #define DD_4_ESP32
    #include <esp32dumbdisplay.h>
    DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));
  ```
  - **MUST** define DD_4_ESP32 before `#include` -- `#define DD_4_ESP32`
  - include esp32dumbdisplay.h -- `#include <esp32dumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"))`  
    - "ESP32" is name used by BluetoothSerial
  - **You should not be using BluetoothSerial for other purposes**
  - In my own testing, the bluetooth communication will hang from time to time.


With a DumbDisplay object, you are ready to proceed with coding, like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/arduino/ddblink/ddblink.ino

```
  #include <ssdumbdisplay.h>

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD));
  LedGridDDLayer *led;

  void setup() {
      // create a LED layer
      led = dumbdisplay.createLedGridLayer();
  }

  void loop() {
      led->toggle();
      delay(1000);
  }
```

You can also try out "layer feedback" from DumbDisplay like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/arduino/ddonoffloopmb/ddonoffloopmb.ino


```
#include <ssdumbdisplay.h>

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

MbDDLayer* pMbLayer = NULL;

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // enable "feedback" -- auto flashing the clicked area
    pMbLayer->enableFeedback("fa");
}

void loop() {
    // check for "feedback"
    const DDFeedback *pFeedback = pMbLayer->getFeedback();
    if (pFeedback != NULL) {
        // act upon "feedback"
        pMbLayer->toggle(pFeedback->x, pFeedback->y); 
    }
}
```

Alternativelly, can setup "callback" function to handle "feedback" passively, like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/arduino/ddonoffmb/ddonoffmb.ino

```
#include <ssdumbdisplay.h>

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

MbDDLayer* pMbLayer = NULL;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, int x, int y) {
    // got a click on (x, y) ... toogle it
    pMbLayer->toggle(x, y);
}

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // setup "callback" function to handle "feedback" passively -- auto flashing the clicked area
    pMbLayer->setFeedbackHandler(FeedbackHandler, "fa");
}

void loop() {
    // give DD a chance to capture feedback
    DDYield();
}
```

Please note that Arduino will check for "feedback" in 4 occasions:
* before every get "feedback" with `getFeedback()`
* after every send of command
* once when `DDYield()` is called
* during the "wait loop" of `DDDelay()`


## More Samples


| 1. Micro:bit | 2. LEDs + "Bar Meter" + LCD | 3. Nested "auto pin" layers  | 4. Manual "pin" layers (LEDs + Turtle) | 5. Graphical [LCD] | 6. "Layer feedback" |
|--|--|--|--|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddmb.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddbarmeter.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddautopin.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddpinturtle.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddgraphical.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/dddoodle.png)|


### Screenshot 1 -- *Micro:bit*

A more interesting sample would be like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/ddmb/ddmb.ino

```
  #include "ssdumbdisplay.h"

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD));

  MbDDLayer *mb;
  int heading;

  void setup() {
    // create Micro:bit layer
    mb = dumbdisplay.createMicrobitLayer();
    // set background color
    mb->backgroundColor(DD_HEX_COLOR(0xF4A460));
  }

  void loop() {
    // set LED color
    String ledColor = DD_RGB_COLOR(128, 15 * heading, 255);
    mb->ledColor(ledColor);

    // show arrow
    MbArrow arrow = static_cast<MbArrow>(heading);  
    mb->showArrow(arrow);

    heading++;
    if (heading == 8)
      heading = 0;
    delay(1000);
  }
```

### Screenshot 2 -- *LEDs + "Bar Meter" + LCD*

An even more interesting sample would be like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/ddbarmeter/ddbarmeter.ino

```
  #include "ssdumbdisplay.h"

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD));

  void setup() {
      // configure to "auto pin (layout) layers" in the vertical direction -- V(*)
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

      // create another LED layers that will be used for "horizontal bar-meter"
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
```


### Screenshot 3 -- *Nested "auto pin" layers*

Auto pinning of layers is not restricted to a single direction. In fact, it can be nested, like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/ddautopin/ddautopin.ino

```
  #include "ssdumbdisplay.h"
  
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD));
  
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
```

### Screenshot 4 -- *Manual "pin" layers (LEDs + Turtle)*

To showcase Turtle, as well as the more controller way of "pinning" layers

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/ddpinturtle/ddpinturtle.ino

```
  #include "ssdumbdisplay.h"

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3)));

  TurtleDDLayer *turtle = NULL;
  int r = random(0, 255);
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
          r = random(0, 255);
      }
  }
```

### Screenshot 5 -- *Graphical [LCD]*

There is a graphical [LCD] layer which is derived from the Turtle layer (i.e. in addition to general feaures of graphical LCD, it also has Turtle-like features)

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/ddgraphical/ddgraphical.ino

```
#include <ssdumbdisplay.h>

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

void setup() {
  // create 4 graphical [LCD] layers
  GraphicalDDLayer *pLayer1 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer2 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer3 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer4 = dumbdisplay.createGraphicalLayer(151, 101);

  // set fill screen with color
  pLayer1->fillScreen("azure");
  pLayer2->fillScreen("azure");
  pLayer3->fillScreen("azure");
  pLayer4->fillScreen("azure");

  //  configure to "auto pin" the 4 layers
  // -- end result of DD_AP_XXX(...) is the layout spec "H(V(0+1)+V(2+3))"
  // -- . H/V: layout direction
  // -- . 0/1/2/3: layer id
  dumbdisplay.configAutoPin(DD_AP_HORI_2(
                              DD_AP_VERT_2(pLayer1->getLayerId(), pLayer2->getLayerId()),
                              DD_AP_VERT_2(pLayer3->getLayerId(), pLayer4->getLayerId())));

  // draw triangles
  int left = 0;
  int right = 150;
  int top = 0;
  int bottom = 100;
  int mid = 50;
  for (int i = 0; i < 15; i++) {
    left += 3;
    top += 3;
    right -= 3;
    bottom -= 3;
    int x1 = left;
    int y1 = mid;
    int x2 = right;
    int y2 = top;
    int x3 = right;
    int y3 = bottom;
    int r = 25 * i;
    int g = 255 - (10 * i);
    int b = 2 * i;
    pLayer1->drawTriangle(x1, y1, x2, y2, x3, y3, DD_RGB_COLOR(r, g, b));
  }

  // draw lines
  for (int i = 0;; i++) {
    int delta = 5 * i;
    int x1 = 150;
    int y1 = 0;
    int x2 = -150 + delta;
    int y2 = delta;
    pLayer2->drawLine(x1, y1, x2, y2, "blue");
    if (x2 > 150)
      break;
  }

  // draw rectangles
  for (int i = 0; i < 15; i++) {
    int delta = 3 * i;
    int x = delta;
    int y = delta;
    int w = 150 - 2 * x;
    int h = 100 - 2 * y;
    pLayer3->drawRect(x, y, w, h, "plum");
  }

  // draw circles
  int radius = 10;
  for (int i = 0; i < 8; i++) {
    int x = 2 * radius * i;
    for (int j = 0; j < 6; j++) {
      int y = 2 * radius * j;
      int r = radius;
      pLayer4->drawCircle(x, y, r, "teal");
      pLayer4->fillCircle(x + r, y + r, r, "gold");
    }
  }
}

void loop() {
}
```

### Screenshot 6 -- *"Layer feedback"*

This very simple doodle sample shows how the "layer feedback" mechanism can be used to route user interaction (clicking) of layer to Arduino.

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/arduino/dddoodle/dddoodle.ino

```
#include <ssdumbdisplay.h>


DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

int dotSize = 5;
const char* penColor = "red";
TurtleDDLayer* pTurtleLayer = NULL;
LcdDDLayer* pLcdLayer = NULL;
LedGridDDLayer* pLedGridLayer = NULL;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, int x, int y) {
    if (pLayer == pLcdLayer) {
        // clicked the "clear" button
        pLayer->backgroundColor("white");
        Reset();
        delay(100);
        pLayer->backgroundColor("lightgray");
    } else if (pLayer == pLedGridLayer) {
        // clicked one of the 3 color options
        const char* color = NULL;
        if (x == 0)
            color = "red";
        else if (x == 1)    
            color = "green";
        else if (x == 2)    
            color = "blue";
        if (color != NULL) {
            pLedGridLayer->turnOff(x, 0);
            delay(100);
            pLedGridLayer->onColor(color);
            pLedGridLayer->turnOn(x, 0);
            penColor = color;
            pTurtleLayer->penColor(penColor);
            pTurtleLayer->dot(dotSize, penColor);
        }
    } else {
        // very simple doodle
        pTurtleLayer->goTo(x, y);
        pTurtleLayer->dot(dotSize, penColor);
    }
}

void Reset() {
    pTurtleLayer->clear();
    pTurtleLayer->penSize(2);
    pTurtleLayer->penColor(penColor);
    pTurtleLayer->home(false);
    pTurtleLayer->dot(dotSize, penColor);
}


void setup() {
    // use a Turtle layer for very simple doodle
    pTurtleLayer = dumbdisplay.createTurtleLayer(201, 201);
    pTurtleLayer->setFeedbackHandler(FeedbackHandler, "fs");
    pTurtleLayer->backgroundColor("azure");
    pTurtleLayer->fillColor("lemonchiffon");

    // use a LED layer for the "clear" button
    pLcdLayer = dumbdisplay.createLcdLayer(5, 1);   
    pLcdLayer->setFeedbackHandler(FeedbackHandler);
    pLcdLayer->backgroundColor("lightgray");
    pLcdLayer->print("CLEAR");

    // use a LED-grid layers for the 3 color options -- red, green and blue 
    pLedGridLayer = dumbdisplay.createLedGridLayer(3);
    pLedGridLayer->setFeedbackHandler(FeedbackHandler);
    pLedGridLayer->onColor("red");
    pLedGridLayer->turnOn(0);
    pLedGridLayer->onColor("green");
    pLedGridLayer->turnOn(1);
    pLedGridLayer->onColor("blue");
    pLedGridLayer->turnOn(2);
 
    // layout the different layers
    dumbdisplay.configAutoPin(DD_AP_VERT_2(
                                DD_AP_HORI_2(
                                    pLedGridLayer->getLayerId(),
                                    pLcdLayer->getLayerId()),
                                pTurtleLayer->getLayerId()));



    Reset();
}


void loop() {
    // give DD a chance to capture feedback
    DDYield();
}
```


# Reference

For reference, please look into the declarations of the different related classes in the header files; mostly dumbdisplay.h -- https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/dumbdisplay.h



# Thank You!

Greeting from the author Trevor Lee:

> Be good! Be happy!
> Peace be with you!
> Jesus loves you!


# License

MIT


# Change History

v0.4.1
- added auto "feedback" (e.g. auto flashing layer)

v0.4.0
- added "layer feedback" mechanism -- i.e. handler "hook" to handle when layer clicked 

v0.1.3
- added 7-Segment-row layer (`SevenSegmentRowDDLayer`)

v0.1.2
- added "graphical" LCD layer (`GraphicalDDLayer`)

v0.1.1
- added ESP32 "experimental" support

v0.1.0
- initial release
