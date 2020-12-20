# DumbDisplay Arduino Library (v0.1.2)

DumbDisplay Ardunio Library enables you to utilize your Android phone as virtual output gadgets for your Arduino / ESP32 experiments.


# Description

Instead of connecting real gadgets to your Arduino for outputing experiment results, you can make use of DumbDisplay for the purpose, to realize virtual gadagets on your Android phone.

Doing so you may defer buying / connecting real output gadgets until later stage of your experiment; also, you should be able to save a few Arduino pins for other experiment needs.

A few types of output layers can be created:
* LED-grid, which can also be used to simulate "bar-meter"
* LCD
* Micro:bit-like canvas
* Turtle-like canvas

You can install the free DumbDisplay app (v0.3.3 or later) from Android Play Store -- https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay

The app can accept connection via
* SoftwareSerial (e.g. Bluetooth via HC-06)
* Serial (USB connected via OTG adapter)

Notes:
* Sorry that since I only have Arduino Uno, therefore the library is only tested with Arduino Uno (and partly with ESP32).
* In case DumbDisply does not "handshake" with your Arduion correctly, you can try resetting your Adruino.


# Sample Code

For Arduino, you have two options for connecting the DumbDisplay Android app.

* Via Serial 
  ```
    #include "#include <dumbdisplay.h>"
    DumbDisplay dumbdisplay(new DDInputOutput());
  ```
  - need to include dumbdisplay.h -- `#include <dumbdisplay.h>`
  - setup a `dumbdisplay` object-- `DumbDisplay dumbdisplay(new DDInputOutput())`
  - doing so will **automatically set Serial baud rate to 115200**, and **you should not be using Serial for other purposes**
* Via SoftwareSerial
  ```
    #include "#include <ssdumbdisplay.h>"
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), 9600));
  ```
  - need to include ssdumbdisplay.h -- `#include <ssdumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), 9600))`  
    - 2 and 3 are the pins used by SoftwareSerial
    - **the default baud rate is 115200**, which seems to work better from my own testing [with HC-06]
  - **You should not be using that SoftwareSerial for other purposes**
* Via **ESP32** BluetoothSerial (Experimental)
  ```
    #define DD_4_ESP32
    #include <esp32dumbdisplay.h>
    DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true));
  ```
  - **MUST** define DD_4_ESP32 before `#include` -- `#define DD_4_ESP32`
  - include esp32dumbdisplay.h -- `#include <esp32dumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true))`  
    - "ESP32" is name used by BluetoothSerial
  - **You should not be using BluetoothSerial for other purposes**
  - In my own testing, the bluetooth communication will hang from time to time.


With a DumbDisplay object, you are ready to proceed coding, like

```
  #include <ssdumbdisplay.h>

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD));
  LedGridDDLayer *led;

  void setup() {
      // create a LED layer
      led = dumbdisplay.createLedGridLayer();
  }

  void loop() {
      led->toggle(0, 0);
      delay(1000);
  }
```


## More Samples


| 1. Micro:bit | 2. LEDs + "Bar Meter" + LCD | 3. Nested "auto pin" layers  | 4. Manual "pin" layers (LEDs + Turtle) | Graphical [LCD] |
|--|--|--|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddmb.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddbarmeter.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddautopin.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddpinturtle.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddgraphical.png)|


### Screenshot 1 -- *Micro:bit*

A more interesting sample would be like

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

Even more interesting sample would be like

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

# Reference

For reference, please look into the declarations of the different related classes in the header files; mostly dumbdisplay.h -- https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/dumbdisplay.h


# Thank You!

Greeting from the author Trevor Lee:

> Be good! Be happy!
> Peace be with you!
> Jesus loves you!


# Change History

v0.1.2
- added "graphical" LCD layer (`GraphicalDDLayer`)

v0.1.1
- added ESP32 "experimental" support

v0.1.0
- initial release
