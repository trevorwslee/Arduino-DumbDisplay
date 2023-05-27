# DumbDisplay Arduino Library (v0.9.8)

[DumbDisplay Ardunio Library](https://github.com/trevorwslee/Arduino-DumbDisplay) enables you to utilize your Android phone as virtual display gadgets (as well as some simple inputting means) for your microcontroller experiments.

You may want to watch the video [**Introducing DumbDisplay -- the little helper for Arduino experiments**](https://www.youtube.com/watch?v=QZkhO6jTf0U) for a brief introduction.


## Enjoy

* [Description](#description)
* [Installation](#installation)
  * [Arduino IDE](#arduino-ide)
  * [PlatformIO](#platformio)
* [DumbDisplay Android App](#dumbDisplay-android-app)
* [Coding Introduction](#coding-introduction)
  * [Samples](#samples)
  * [More Samples](#more-samples)
  * [More OTG Examples](#more-otg-examples)
* [Features](#features)  
  * [DumbDispaly "Feedback" Mechanism](#dumbdispaly-feedback-mechanism)
  * [DumbDispaly "Tunnel"](#dumbDispaly-tunnel)
  * [Service "Tunnels"](#service-tunnels)
  * ["Device Dependent View" Layers](#device-dependent-view-layers)
  * [Downloadable Font Support](#downloadable-font-support)
  * [Positioning of Layers](#positioning-of-layers)
  * [Record and Playback Commands](#record-and-playback-commands)
  * [Survive DumbDisplay App Reconnection](#survive-dumbdisplay-app-reconnection)
  * [More "Feedback" Options](#more-feedback-options)
  * [Idle Callback and ESP32 Deep Sleep](#idle-callback-and-esp32-deep-sleep)
  * [Using "Tunnel" to Download Images from the Web](#using-tunnel-to-download-images-from-the-web)
  * [Save Pictures to Phone Captured with ESP32 Cam](#save-pictures-to-phone-captured-with-esp32-cam)
  * [Caching Single-bit Bitmap to Phone](#caching-single-bit-bitmap-to-phone)
  * [Caching 16-bit Colored Bitmap to Phone](#caching-16-bit-colored-bitmap-to-phone)
  * [Saving Images for DumbDisplay](#saving-images-for-dumbdisplay)
  * [Audio Supports](#audio-supports)
* [Reference](#reference)
* [DumbDispaly WIFI Bridge](#dumbdispaly-wifi-bridge)
* [DumbDisplay App Hints](#dumbdisplay-app-hints)
* [Thank You!](#thank-you)
* [License](#license)
* [Change History](#change-history)



# Description

Instead of connecting real gadgets to your Arduino framework compatible microcontroller board for showing experiment results (or for getting simple input like pressing), you can make use of DumbDisplay for the purposes -- to realize virtual IO gadagets remotely on your Android phone, or locally with OTG adaptor connecting your microcontroller board and your Android phone.

By doing so you can defer buying / wiring real gadgets until later stage of your experiment. Even, you might be able to save a few microcontroller pins for other experiment needs, if you so decided that Android phone can be your display gadget (and more) with DumbDisplay app.

The core is [DumbDisplay](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_dumb_display.html).
On it, a few types of layers can be created mixed-and-matched:
* LED-grid, which can also be used to simulate "bar-meter" -- [LedGridDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_led_grid_d_d_layer.html)
* LCD (text based), which is also a good choice for simulating button -- [LcdDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_lcd_d_d_layer.html)
* Micro:bit-like canvas -- [MbDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_mb_d_d_layer.html)
* Turtle-like canvas -- [TurtleDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_turtle_d_d_layer.html)
* Graphical LCD, which is derived from the Turtle layer (i.e. in addition to general feaures of graphical LCD, it also has certain Turtle-like features) -- [GraphicalDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_graphical_d_d_layer.html) 
* 7-Segment-row, which can be used to display a series of digits, plus a decimal dot -- [SevenSegmentRowDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_seven_segment_row_d_d_layer.html)
* Joystick, which can be used for getting virtual joystick movement input, and can also be used for horizontal/vertial "slider" input -- [JoystickDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_joystick_d_d_layer.html)
* Plotter, which works similar to the plotter of DumbDisplay [when it is acting as serial monitor], but plotting data are sent by calling the layer's method -- [PlotterDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_plotter_d_d_layer.html)
* Terminal "device dependent view" layer, for logging sketch traces -- [TerminalDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_terminal_d_d_layer.html)
* TomTom map "device dependent view" layer, for showing location (latitude/longitude) -- [TomTomMapDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_tom_tom_map_d_d_layer.html)

Note that with the "layer feedback" mechanism, user interaction (like clicking of layers) can be routed back to the connected microcontroller, and as a result, the layers can be used as simple input gadgets as well. Please refer to [DumbDispaly "Feedback" Mechanism](#dumbdispaly-feedback-mechanism) for more on "layer feedback" mechanism.


# Installation

## Arduino IDE

The easiest way to install DumbDisplay Arduino Library is through Arduino IDE's Library Manager -- open ***Manage Libraries***, then search for "dumpdisplay" ... an item showing ```DumbDisplay by Trevor Lee``` should show up; install it. For a reference, you may want to see my post [Blink Test with Virtual Display, DumbDisplay](https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/)  

Alternative, you can choose to use the more "fluid" manual approach. The basic steps are
1) download **CODE** ZIP file (the green button), from https://github.com/trevorwslee/Arduino-DumbDisplay
2) to install, use Arduino IDE menu option **Sktech** | **Include Library** | **Add .ZIP library...** and choose the ZIP you just downloaded

For demonstration on installing DumbDisplay Arduino Library this manual way, you may want to watch the video [**Arduino Project -- HC-06 To DumbDisplay (BLINK with DumbDisplay)**](https://www.youtube.com/watch?v=nN7nXRy7NMg)

To upgrade DumbDisplay Arduino Library installed manually this way, you just need to replace the directory manually, by following the above steps again.


## PlatformIO

If you have an Arduino framework PlatformIO project that you want to make use of DumbDisplay Arduino Library, you can simply modify the project's *platformio.ini* adding to *lib_deps* like:

```
[env]
lib_deps =
    https://github.com/trevorwslee/Arduino-DumbDisplay
```


For demonstration on installing DumbDisplay Arduino Library for PlatformIO project, you may want to watch the video [**Arduino UNO Programming with PlatformIO and DumbDisplay**](https://www.youtube.com/watch?v=PkeFa2ih4EY) 

To upgrade DumbDisplay Arduino Library for that PlatformIO project, you can simply delete the 'depended libraries' directory `.pio/libdeps` to force all to be re-installed.


# DumbDisplay Android App

Obviously, you will need to install an app on your Android phone. Indeed, for Arduino DumbDisplay to work, you will need to install the free [DumbDisplay Arduino app](https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay) from Android Play Store

The app is itself a USB serial monitor, and certinaly can also accept DumbDisplay connection via
* SoftwareSerial (e.g. Bluetooth by HC-05 / HC-06; even HC-08)
* BluetoothSerial (for ESP32)
* Bluetooth LE (for ESP32, ESP32C3 and ESP32S3)
* WIFI (e.g. ESP01, ESP8266, ESP32 and PicoW)
* Serial (USB connected via OTG adapter)
* Serial <-> WIFI via the simple included tool -- [DumbDisplay WIFI Bridge](#dumbDispaly-wifi-bridge)
* Serial2 (hardware serial, like for Raspberry Pi Pico)

Notes:
* Out of so many microcontroller boards, I have only tested DumbDisplay with the microcontroller boards that I have access to. Nevertheless, I am hopeful that using Serial for other microcontroller boards should work just fine [in general].
* In case DumbDisplay does not handshake with your microcontroller board correctly, you can try resetting the board, say, by pressing the "reset" button on the board.

 
# Coding Introduction

The starting point is a [DumbDisplay](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_dumb_display.html) object, which requires an IO object for communicating with your Android DumbDisplay app:

* Via Serial -- via OTG; you may want to refer to [Blink Test with Virtual Display, DumbDisplay](https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/)
  ```
    #include "dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDInputOutput(115200));
  ```
  - need to include `dumbdisplay.h` -- `#include "dumbdisplay.h"`
  - setup a `dumbdisplay` object-- `DumbDisplay dumbdisplay(new DDInputOutput())`
  - you **should not** be using `Serial` for other purposes
  - the default baud rate is 115200;  a lower baud rate, say 9600, may work better in some cases
* Via [`SoftwareSerial`](https://www.arduino.cc/en/Reference/softwareSerial) -- connected to Bluetooth module like HC-06. For an example, you may want to refer to the post [Setup HC-05 and HC-06, for Wireless 'Number Invaders'](https://www.instructables.com/Setup-HC-05-and-HC-06-for-Wireless-Number-Invaders/)
  ```
    #include "ssdumbdisplay.h"
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200));
  ```
  - need to include `ssdumbdisplay.h` -- `#include "ssdumbdisplay.h"`
  - setup a `dumbdisplay` object -- e.g. `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200))`  
    - in this example, 2 and 3 are the pins used by `SoftwareSerial`
    - the default baud rate is 115200, which seems to work better from my own testing with HC-06; however, you may want to test using lower baud rate in case connection is not stable; this is especially true for HC-08, which connects via BLE. 
  - you **should not** be using that `SoftwareSerial` for other purposes
* Via `Serial2` -- for STM32, connected to Bluetooth module like HC-06 
  ```
    #include "serial2dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDSerial2IO(115200));
  ```
  - need to include `serial2dumbdisplay.h` -- `#include "serial2dumbdisplay.h"`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDSerial2IO(115200))`
  - e.g. for STM32F103: connect PA3 (RX2) to TX of HC-06 and connect PA2 (TX2) to RX of HC-06
* Via `Serial2` -- for Raspberry Pi Pico, connected to Bluetooth module like HC-06 
  ```
    #include <picodumbdisplay.h>
    DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));
  ```
  - need to include `picodumbdisplay.h` -- `#include "picodumbdisplay.h"`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDPicoUart1IO(115200))`
  - **MUST** define DD_4_PICO_TX and DD_4_PICO_RX before including `picodumbdisplay.h`, like
  ```
    #define DD_4_PICO_TX 8
    #define DD_4_PICO_RX 9
    #include "picodumbdisplay.h"
    DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));
  ```
* Via **ESP32** `BluetoothSerial`
  ```
    #include "esp32dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));
  ```
  - include `esp32dumbdisplay.h` -- `#include "esp32dumbdisplay.h"`
  - setup a `dumbdisplay` object -- e.g. `DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"))`  
    - in the sample, "ESP32" is the name used by `BluetoothSerial`
  - you **should not** be using `BluetoothSerial` for other purposes
* Via **ESP32** `BLE`
  ```
    #include "esp32bledumbdisplay.h"
    DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32BLE"));
  ```
  - include `esp32bledumbdisplay.h` -- `#include "esp32bledumbdisplay.h"`
  - setup a `dumbdisplay` object -- e.g. `DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32BLE"))`  
    - in the sample, "ESP32BLE" is the name used by BLE
  - you **should not** be using ESP32's BLE for other purposes
  - be **warned** that `DDBLESerialIO` is slow; if classic Bluetooth is supported by micrcontroller (like ESP32), choose `DDBluetoothSerialIO` instead 
* Via WIFI as a [`WiFiServer`](https://www.arduino.cc/en/Reference/WiFi) -- for ESP01/ESP8266/ESP32/PicoW  
  ```
    #include "wifidumbdisplay.h"
    const char* ssid = "wifiname";
    const char* password = "wifipassword";
    DumbDisplay dumbdisplay(new DDWiFiServerIO(ssid, password));
  ```
  - ESP01 is basically a ESP8266
  - WIFI credentials are passed to `WiFi`
  - by default, DumbDisplay will setup and log using `Serial` with baud rate 115200; and you should see log lines like:
  ```
    binding WIFI <wifiname>
    binded WIFI <wifiname>
    listening on 192.168.1.134:10201 ...
  ```  
    where 192.168.1.134 is the IP of your microcontroller and 10201 is the port which is the defaul port

  By making use of DumbDisply WIFI Bridge, WIFI connection is possible for any microcontroller board (e.g. Arduino UNO) --
  With DumbDisply WIFI Bridge running on your computer, you can keep the microcontroller connected with USB, and make WIFI connection with DumbDisplay Android app.
  Please refer to [DumbDisplay WIFI Bridge](#dumbDispaly-wifi-bridge) for more description on it.

|  | |
|--|--|
|With a DumbDisplay object, you are ready to proceed with coding, like https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/examples/otgblink/otgblink.ino|![](screenshots/otgblink.png)|


```
#include "dumbdisplay.h"

// create the DumbDisplay object; assuming OTG (USB) connection with 115200 baud (default)
DumbDisplay dumbdisplay(new DDInputOutput());
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

You may want to refer to the post [Blink Test With Virtual Display, DumbDisplay](https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/), which talks about a slightly modified version of the above sketch.



## Samples

Here, several examples are presented demonstrating the basis of DumbDisplay. More examples will be shown when DumbDisplay features are described in a bit more details in later sections.


| [Micro:bit](#sample----microbit) | [LEDs + "Bar Meter" + LCD](#sample----leds--bar-meter--lcd) | [Graphical [LCD]](#sample----graphical-lcd) |
|--|--|--|
|![](screenshots/ddmb.png)|![](screenshots/ddbarmeter.png)|![](screenshots/ddgraphical.png)|


### Sample -- *Micro:bit*

A more interesting sample would be like the one shown here, which shows how to use the [MbDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_mb_d_d_layer.html) to simulate a Micro:bit.

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddmb/ddmb.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput(57600));

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

### Sample -- *LEDs + "Bar Meter" + LCD*

An even more interesting sample would be like the example shown here, which demonstrates how the [LedGridDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_led_grid_d_d_layer.html) can be used.

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddbarmeter/ddbarmeter.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput());

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


### Sample -- *Graphical [LCD]*

There is a graphical [LCD] layer [GraphicalDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_graphical_d_d_layer.html) which is "derivded" from the Turtle layer (i.e. in addition to general feaures of graphical LCD, it also has Turtle-like features)

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddgraphical/ddgraphical.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput(57600));

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


## More Samples

| [Nested "auto pin" layers](#sample----nested-auto-pin-layers)  | [Manual "pin" layers (LEDs + Turtle)](#sample----manual-pin-layers-leds--turtle) | ["Layer feedback"](#sample----layer-feedback) |
|--|--|--|
|![](screenshots/ddautopin.png)|![](screenshots/ddpinturtle.png)|![](screenshots/dddoodle.png)|


### Sample -- *Nested "auto pin" layers*

Auto pinning of layers is not restricted to a single direction. In fact, it can be nested, like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddautopin/ddautopin.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput());

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


### Sample -- *Manual "pin" layers (LEDs + Turtle)*

To showcase Turtle layer [TurtleDDLayer](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_turtle_d_d_layer.html), as well as the more controller way of "pinning" layers

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddpinturtle/ddpinturtle.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput());

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

### Sample -- *"Layer feedback"*

This very simple doodle sample shows how the "layer feedback" mechanism can be used to route user interaction (clicking) of layer to Arduino.

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/dddoodle/dddoodle.ino

```
#include "dumbdisplay.h"

// for connection
// . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
// . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
DumbDisplay dumbdisplay(new DDInputOutput());

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
    // give DD a chance to capture "feedback"
    DDYield();
}
```

Notes:
* DumbDisplay library will work cooperatively with your code; therefore, do give DumbDisplay library chances to do its work. Please call `DDYeild()` and/or `DDDelay()` appropriately whenever possible. 



## More OTG Examples

| ["RGB "Sliders"](#example----rgb-sliders) |  ["Tunnel" for RESTful](#example----tunnel-for-restful) | ["Tunnel" for Web Image](#example----tunnel-for-web-image) | 
|--|--|--|
|![](screenshots/otgrgb.png)|![](screenshots/otgrest.png)|![](screenshots/otgwebimage.png)|


### Example -- *RGB "Sliders"*

This example make use of the virtual Joystick layers to realize three "sliders" for the three primiary colors RGB.
```
#include "dumbdisplay.h"

// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput());

// declare a graphical layer object to show the selected color; to be created in setup()
GraphicalDDLayer *colorLayer;
// declare the R "slider" layer
JoystickDDLayer *rSliderLayer;
// declare the G "slider" layer
JoystickDDLayer *gSliderLayer;
// declare the B "slider" layer
JoystickDDLayer *bSliderLayer;

int r = 0;
int g = 0;
int b = 0;

void setup() {
  // create the "selected color" layer
  colorLayer = dumbdisplay.createGraphicalLayer(350, 150);
    colorLayer->border(5, "black", "round", 2);
  
  // create the R "slider" layer
  rSliderLayer = dumbdisplay.createJoystickLayer(255, "hori", 0.5);
  rSliderLayer->border(3, "darkred", "round", 1);
  rSliderLayer->colors("red", DD_RGB_COLOR(0xff, 0x44, 0x44), "black", "darkgray");

  // create the G "slider" layer
  gSliderLayer = dumbdisplay.createJoystickLayer(255, "hori", 0.5);
  gSliderLayer->border(3, "darkgreen", "round", 1);
  gSliderLayer->colors("green", DD_RGB_COLOR(0x44, 0xff, 0x44), "black", "darkgray");

  // create the B "slider" layer
  bSliderLayer = dumbdisplay.createJoystickLayer(255, "hori", 0.5);
  bSliderLayer->border(3, "darkblue", "round", 1);
  bSliderLayer->colors("blue", DD_RGB_COLOR(0x44, 0x44, 0xff), "black", "darkgray");

  // "auto pin" the layers vertically
  dumbdisplay.configAutoPin(DD_AP_VERT);

  colorLayer->backgroundColor(DD_RGB_COLOR(r, g, b));
}

void loop() {
  int oldR = r;
  int oldG = g;
  int oldB = b;
  
  const DDFeedback*  fb;

  fb = rSliderLayer->getFeedback();
  if (fb != NULL) {
    r = fb->x;
  }
  fb = gSliderLayer->getFeedback();
  if (fb != NULL) {
    g = fb->x;
  }
  fb = bSliderLayer->getFeedback();
  if (fb != NULL) {
    b = fb->x;
  }

  if (r != oldR || g != oldG || b != oldB) {
    colorLayer->backgroundColor(DD_RGB_COLOR(r, g, b));
  }

}
```

### Example -- *"Tunnel" for RESTful*

This example should demonstrate how to use "tunnel" to access the Internet for simple things, like calling RESTful api:

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/examples/ogtrest/ogtrest.ino

```
#include "dumbdisplay.h"

// create the DumbDisplay object; assuming OTG (USB) connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput());

// declare a graphical layer object, to be created in setup()
GraphicalDDLayer *graphicalLayer;
// declare a tunnel object, to be created in setup()
JsonDDTunnel *restTunnel;

void setup() {
  // setup a "graphial" layer with size 350x150
  graphicalLayer = dumbdisplay.createGraphicalLayer(350, 150);
  graphicalLayer->backgroundColor("yellow");        // set background color to yellow
  graphicalLayer->border(10, "blue", "round");      // a round blue border of size 10  
  graphicalLayer->penColor("red");                  // set pen color

  // setup a "tunnel" to get "current time" JSON data; suggest to specify the buffer size to be the same as fields wanted
  restTunnel = dumbdisplay.createFilteredJsonTunnel("http://worldtimeapi.org/api/timezone/Asia/Hong_Kong", "client_ip,timezone,datetime,utc_datetime", true, 4);  

  graphicalLayer->println();
  graphicalLayer->println("-----");
  while (!restTunnel->eof()) {           // check that not EOF (i.e. something still coming)
    while (restTunnel->count() > 0) {    // check that received something
      String fieldId;
      String fieldValue;
      restTunnel->read(fieldId, fieldValue);                // read whatever received
      dumbdisplay.writeComment(fieldId + "=" + fieldValue); // write out that whatever to DD app as comment
      if (fieldId == "client_ip" || fieldId == "timezone" || fieldId == "datetime" || fieldId == "utc_datetime") {
        // if the expected field, print it out
        graphicalLayer->print(fieldId);
        graphicalLayer->print("=");
        graphicalLayer->println(fieldValue);
      }
    }
  }
  graphicalLayer->println("-----");
}

void loop() {
}
```

### Example -- *"Tunnel" for Web Image*

This example should demonstrate how to use "tunnel" to get images from the Web and disply them -- blink with web images:

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/examples/ogtwebimage/ogtwebimage.ino
```
#include "dumbdisplay.h"

DumbDisplay dumbdisplay(new DDInputOutput(115200));


GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;

void setup() {
  // create a graphical layer for drawing the web images to
  graphical = dumbdisplay.createGraphicalLayer(200, 300);

  // create tunnels for downloading web images ... and save to your phone
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", "lock-unlocked.png");
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", "lock-locked.png");
}

bool locked = false;
void loop() {
  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int result_unlocked = tunnel_unlocked->checkResult();
  int result_locked = tunnel_locked->checkResult();

  int result;
  const char* image_file_name;
  if (locked) {
    image_file_name = "lock-locked.png";
    result = result_locked;
  } else {
    image_file_name = "lock-unlocked.png";
    result = result_unlocked;
  }
  if (result == 1) {
    graphical->drawImageFile(image_file_name);
  } else if (result == 0) {
    // downloading
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("... ...");
    graphical->println(image_file_name);
    graphical->println("... ...");
  } else if (result == -1) {
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("XXX failed to download XXX");
  }
  locked = !locked;
  delay(1000);
}
```



# Features


## DumbDispaly "Feedback" Mechanism

You can also try out "layer feedback" from DumbDisplay like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddonoffloopmb/ddonoffloopmb.ino


```
#include "dumbdisplay.h"

// for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
DumbDisplay dumbdisplay(new DDInputOutput());

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

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddonoffmb/ddonoffmb.ino

```
#include "ssdumbdisplay.h"

// assume HC-06 connected, to pin 2 and 3
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true));

MbDDLayer* pMbLayer = NULL;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    // got a click on (x, y) ... toogle it
    pMbLayer->toggle(feedback.x, feedback.y);
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

Please note that DumbDisplay library will check for "feedback" in several occasions:
* before every get "feedback" with `getFeedback()`
* after every send of command
* once when `DDYield()` is called
* during the "wait loop" of `DDDelay()`
* calling "tunnel" to check for EOF


## DumbDispaly "Tunnel"


By using DumbDisplay "tunnels", even Arduino UNO can get simple data from the Internet via DumbDisplay app. The above "Tunnel" for RESTful example should already show-case this feature.

```
#include "dumbdisplay.h"

DumbDisplay dumbdisplay(new DDInputOutput());

JsonDDTunnel *restTunnel;

void setup() {
  // setup a "tunnel" to get "current time" JSON data; suggest to specify the buffer size to be the same as fields wanted
  restTunnel = dumbdisplay.createFilteredJsonTunnel("http://worldtimeapi.org/api/timezone/Asia/Hong_Kong", "client_ip,timezone,datetime,utc_datetime", true, 4);  
  while (!restTunnel->eof()) {           // check that not EOF (i.e. something still coming)
    while (restTunnel->count() > 0) {    // check that received something
      String fieldId;
      String fieldValue;
      restTunnel->read(fieldId, fieldValue);                // read whatever received
      dumbdisplay.writeComment(fieldId + "=" + fieldValue); // write out that whatever to DD app as comment
      if (fieldId == "client_ip" || fieldId == "timezone" || fieldId == "datetime" || fieldId == "utc_datetime") {
        // if the expected field ...
        ...
      }
    }
  }
}

void loop() {
}
```

In case a "tunnel" reaches EOF, and needs be reinvoked:

```
restTunnel->reconnect();
```

In case a "tunnel" finishes all its tasks in the middle of the sketch, it should be released in order for Arduino to claim back resources:

```
dumbdisplay.deleteTunnel(restTunnel);
```

Here is some description on the how JSON response to JSON data is converted, and how to loop getting the JSON data:

* you construct `JsonDDTunnel` "tunnel" and make REST request like:
  ```
  pTunnel = dumbdisplay.createJsonTunnel("http://worldtimeapi.org/api/timezone/Asia/Hong_Kong"); 
  ```
* you read JSON data from the "tunnel" a piece at a time;
  e.g. if the JSON is
  ```
  { 
    "full_name": "Bruce Lee",
    "name":
    {
      "first": "Bruce",
      "last": "Lee"
    },
    "gender": "Male",
    "age": 32
  }
  ```  
 
  then, the following JSON pieces will be returned:
  * `full_name` = `Bruce Lee`
  * `name.first` = `Bruce`
  * `name.last` = `Lee`
  * `gender` = `Male`
  * `age` = `32`
  
  notes:
  * all returned values will be text
  * control characters like `\r` not supported
  * since lots of data could be acquired,  `Serial` connection might not be suitable due to it's small buffer size
  
* use `count()` to check if the "tunnel" has anything to read, and use `read()` to read what got, like:
  ```
  while (!restTunnel->eof()) {
    while (restTunnel->count() > 0) {
      String fieldId;
      String fieldValue;
      restTunnel->read(fieldId, fieldValue);  // fieldId and fieldValue combined is a piece of JSON data 
      dumbdisplay.writeComment(fieldId + "=" + fieldValue);
    }
  }  
  ```
  note that `eof()` will check whether everything has returned and read before signaling EOF.


## Service "Tunnels"

Service "tunnels" is a kind of "tunnels" that aids getting specific external data, by making use of your Android's phone features.

The two service "tunnels" are:
* "Date-time service tunnel" for getting current date-time from your Android phone
  ```
    BasicDDTunnel *datetimeTunnel = dumbdisplay.createDateTimeServiceTunnel();
    datetimeTunnel->reconnectTo("now");
    ...
    String datetime;
    if (datetimeTunnel->readLine(datetime)) {
      ...
    } 
  ```
  Note that other than getting "now" date-time as text, you can use "now-millis" to get date-time in milli-seconds.  
  The complete "now" sample sketch: https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddnow/ddnow.ino
* "GPS service tunnel" for getting your Android phone's location
  In order for DumbDisplay app to access your phone's GPS service, permission is needed;
  please select DumbDisplay app menu item ***Settings*** and click the ***Location Service button***.
  ```
    GpsServiceDDTunnel *gpsTunnel = dumbdisplay.createGpsServiceTunnel();
    gpsTunnel->reconnectForLocation();
    ...
    DDLocation location;
    if (gpsTunnel->readLocation(location)) {
      ...
    }  
  ```   
  The complete "here" sample sketch: https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddhere/ddhere.ino.
* TensorFlow Lite object detection demo service "tunnel"
  ```
    ObjectDetetDemoServiceDDTunnel *object_detect_tunnel = dumbdisplay.createObjectDetectDemoServiceTunnel();
    ...
    object_detect_tunnel->reconnectForObjectDetect("downloaded.png");
    ...
    DDObjectDetectDemoResult objectDetectResult;
    if (object_detect_tunnel->readObjectDetectResult(objectDetectResult)) {
      dumbdisplay.writeComment(String(". ") + objectDetectResult.label);
      int x = objectDetectResult.left;
      int y = objectDetectResult.top;
      int w = objectDetectResult.right - objectDetectResult.left;
      int h = objectDetectResult.bottom - objectDetectResult.top;
      graphical->drawRect(x, y, w, h, "green");
      graphical->drawStr(x, y, objectDetectResult.label, "yellow", "a70%darkgreen", 32);
    }
  ```
|  | |
|--|--|
|For the complete demo, please refer to [Arduino AI Fun With TensorFlow Lite, Via DumbDisplay](https://www.instructables.com/Arduino-AI-Fun-With-TensorFlow-Lite-Via-DumbDispla/).|![](screenshots/esp32camobjectdetect.gif)|
  


## "Device Dependent View" Layers

A "device dependent view" layer is a layer that embeeds a specific kind of Android View as a DD Layer. And hence, it's rendering is totally controlled by the Android View itself. DumbDisplay app simply provides a place where it will reside.

Nevertheless, do note that:
* DDLayer's margin, border, padding, as well as visibility, will work as expected.
* The "device dependent view" DD Layer size -- of the "opening" for the Android view -- is just like graphical LCD layer,
  but be warned that it will ***not*** be scaled, like other DD Layers.

There are two "device dependent view" layer available.

### Terminal Layer

```TerminalDDLayer``` is a simple "device dependent view" layer that simulates the function of a simple serial terminal (monitor) like DumbDisplay app itself. You create such layer like

A sample use is: https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/projects/ddgpsmap/ddgpsmap.ino

The sample demonstrates how to read simple GPS location data from module like NEO-7M U-BLOX, formats and output the data
to a ```TerminalDDLayer```:

```
  ...
  #define NEO_RX 6   // RX pin of NEO-7M U-BLOX
  #define NEO_TX 5   // TX pin of NEO-7M U-BLOX
  SoftwareSerial gpsSerial(NEO_TX, NEO_RX);
  GpsSignalReader gpsSignalReader(gpsSerial);
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
  TerminalDDLayer* terminal;
  void setup() {
    gpsSerial.begin(9600);
    terminal= dumbdisplay.createTerminalLayer(600, 800);
  }
GpsSignal gpsSignal;
void loop() {
  if (gpsSignalReader.readOnce(gpsSignal)) {
    terminal->print("- utc: ");
    terminal->print(gpsSignal.utc_time);
    terminal->print(" ... ");
    if (gpsSignal.position_fixed) {
      terminal->print("position fixed -- ");
      terminal->print("lat:");
      terminal->print(gpsSignal.latutude);
      terminal->print(" long:");
      terminal->print(gpsSignal.longitude);
     ...
    }
  }
```

The above sketch assumes using OTG USB adaptor for connection to Android DumbDisplay app. And as a result, bringing the above GPS experiment outdoor should be easier. Not only the microcontroller board can be powered by your Android phone, you can observe running traces of the sketch with your phone as well.


### TomTom Map Layer


The only "device dependent view" layer is [```TomTomMapDDLayer```](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_tom_tom_map_d_d_layer.html).

|  | |
|--|--|
|![](screenshots/ddnowhere.jpg)|For demonstration, the above "now/here" samples are combined into a more "useful" sketch that also makes use of this Android View to show the GPS location retrieved, continuously. The complete "nowhere" sample is https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddnowhere/ddnowhere.ino|


## Downloadable Font Support

Layers like [```GraphicalDDLayer```](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_graphical_d_d_layer.html) can use specified font for rendering text; however, there are not many fonts in normal Android installments.
DumbDisplay app supports the use of  selective downloadable font open sourced by Google, namely, B612, Cutive, Noto Sans, Oxygen, Roboto, Share Tech, Spline Sans and Ubuntu.

```
  ...
  GraphicalDDLayer *graphical = dumbdisplay.createGraphicalLayer(150, 300);
  ...
  graphical->setTextFont("DL::Roboto");
  ...
```

In order to ensure that these Google's fonts are ready for DumbDisplay app when they are used, please check ***Settings | Pre-Download Fonts***

For a complete sample sketch of using downloadable font, please refer to https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddfonts/ddfonts.ino


## Positioning of Layers

By default, layers are stacked one by one, with the one created first on the top of the stack. Each layer will be automatically stretched to fit the DumbDisplay screen, with the aspact ratio kept unchanged.

This stacking behavior is not suitable for all cases. In fact, I would say that this default behavior is not suitable for many cases, except for the most simple case when you only have a single layer.

Actually, two mechanisms to "pin" the layers on DumbDisplay are provided -- automatic and manual.

The automatic pinning of layers is the easier. You only need to call the DumbDisplay method `configAutoPin()` once to pin the layers, like
```
    dumbdisplay.configAutoPin(DD_AP_VERT_2(
                                DD_AP_HORI_2(
                                    pLedGridLayer->getLayerId(),
                                    pLcdLayer->getLayerId()),
                                pTurtleLayer->getLayerId()));
```
`DD_AP_HORI_2()` / `DD_AP_VERT_2()` macro pins 2 layers side by side horizontally / vertically. It accepts 2 arguments, with each one either a layer id, or another **DD_AP_XXX** macro. 

The different **DD_AP_XXX* macros are
* **DD_AP_HORI_`N`()** : Horizontally layout `N` layers (ids) or nested **DD_AP_XXX**; with **DD_AP_HORI** layouts all layers horizontally
* **DD_AP_VERT_`N`()** : Vertiallly layout `N` layers (ids) or nested **DD_AP_XXX**; with **DD_AP_VERT** layouts all layers vertcally
* **DD_AP_STACK_`N`()** : Stack `N` layers (ids) or nested **DD_AP_XXX**; with the **DD_AP_STACK** stacks all layers
* **DD_AP_PADDING()** : It accepts padding sizes -- left, top, right and bottom -- and a layer (id) (or nested **DD_AP_XXX**)   
* **DD_AP_SPACER()** : It is a invisible "spacer" layer with dimension -- width and height

The manual way of pinning layers is a bit more complicated. First, a "pin frame" needs be defined with a fixed size; by default, the size is 100 by 100. To change the "pin frame" fixed size, use the DumbDisplay method `configPinFrame()`. 

On the "pin frame", the different layers are explicitly pinned.

For example
```
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
```
The DumbDisplay method `pinLayer` accepts 5 arguments. The first argument is the layer to pin. The rest four arguments define the rectangular area on the "pin frame" to pin the layer to -- the four argments are "left-top" corner and the "width-height" of the rectangular area.

As a matter of fact, the "auto pin" mechanism can be used in conjunction with the manual pinning mechanism. The method to used is `pinAutoPinLayers`.

To get a feel, you may want to refer to the video [**Raspberry Pi Pico playing song melody tones, with DumbDisplay control and keyboard input**](https://www.youtube.com/watch?v=l-HrsJXIwBY) 

|  | |
|--|--|
|![](screenshots/pico-speaker_connection.png)|![](screenshots/ddmelody.jpg)|

Going back to "auto pin". In fact, there is a builder for such "auto pin" config -- [DDAutoPinConfig](https://trevorwslee.github.io/ArduinoDumbDisplay/html/class_d_d_auto_pin_config.html)

Using it shoud be appearent. Hopefully, some example should be sifficient.

Example 1:
```
  dumbdisplay->configAutoPin(DDAutoPinConfig('V').
                              beginGroup('H').
                                  addSpacer(1, 1).
                                  addLayer(pLedGridLayer).
                                  addSpacer(2, 1).
                                  addLayer(pLcdLayer).
                                  addSpacer(1, 1).
                              endGroup().
                              addLayer(pTurtleLayer).
                              build());
```
is equivalent to
```
  dumbdisplay->configAutoPin(DD_AP_VERT_2(
                              DD_AP_HORI_5(
                                  DD_AP_SPACER(1, 1),
                                  pLedGridLayer->getLayerId(),
                                  DD_AP_SPACER(2, 1),
                                  pLcdLayer->getLayerId(),
                                  DD_AP_SPACER(1, 1)),
                              pTurtleLayer->getLayerId()));
```

Example 2:
```
    dumbdisplay.configAutoPin(
      DDAutoPinConfig('V').
        beginGroup('H').
          addLayer(plotterLayer).
          beginGroup('V').
            addLayer(rLayer).addLayer(gLayer).addLayer(bLayer). 
          endGroup().
        endGroup().
        beginGroup('S').
          addLayer(colorLayer).
          beginPaddedGroup('H', 50, 200, 50, 200).
            addLayer(r7Layer).addLayer(g7Layer).addLayer(b7Layer).
          endPaddedGroup().
        endGroup().
        beginGroup('H').
          addLayer(whiteLayer).addLayer(blackLayer).
        endGroup().
        build()
    );
```
is equalivent to
```
    dumbdisplay.configAutoPin(
      DD_AP_VERT_3(
        DD_AP_HORI_2(
          plotterLayer->getLayerId(),
          DD_AP_VERT_3(rLayer->getLayerId(), gLayer->getLayerId(), bLayer->getLayerId())
        ),
        DD_AP_STACK_2(
          colorLayer->getLayerId(),
          DD_AP_PADDING(50, 200, 50, 200,
            DD_AP_HORI_3(r7Layer->getLayerId(), g7Layer->getLayerId(), b7Layer->getLayerId()))
        ),
        DD_AP_HORI_2(whiteLayer->getLayerId(), blackLayer->getLayerId())
      )  
    );
```

You can choose which one is more convenient for you!


## Record and Playback Commands

It is apparent that turning on LEDs, drawing on graphical LCDs, etc, by sending text-based commands is not particularly efficient. Indeed, screen flickering is a commonplace, especial when there are lots of activities.

In order to relieve this flickering situation a bit, it is possilbe to freeze DumbDisplay's screen during sending bulk of commands:
* `dumbdisplay.recordLayerCommands()` -- start recording commands (freeze DumbDisplay screen)
* `dumbdisplay.playbackLayerCommands()` -- end recording commands and playback the recorded commands (unfreeze Dumbdisplay screen)

A sample sketch demonstrates that this DumbDisplay feature can help; the sample is adapted from one that shows off Arduino UNO with Joystick shield connecting to a OLED display: https://cyaninfinite.com/interfacing-arduino-joystick-shield-with-oled-display

Instead of posting the sample sketch here, please find it with the link: https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/projects/joystick/joystick.ino

| Arduino UNO with Joystick shield | DumbDisplay |
|--|--|
|![](screenshots/joystick-arduino.jpg)|![](screenshots/joystick-dd.png)|


If you are interested, you may want to watch the video **Arduino JoyStick Shield and DumbDisplay** -- https://www.youtube.com/watch?v=9GYrZWXHfUo


## Survive DumbDisplay App Reconnection

In certain "stateless" cases, like DumbDisplay is simply used as means to show values, it is possible for DumbDisplay to be able to meaningfully reconnect after DumbDisplay app disconnect / restart. (Do note that DumbDisplay app does not persist "state" information.)

The only missing piece is the layout of the different layers. And this missing piece can be "regained" by recording the layout commands, and automatically playback when DumbDisplay app reconnects.

To do this, you simply need to enclose the "layer setup" code with the record/playback mechanism mentioned previously. 

More precisely, you will need to use the following methods of DumbDisplay object:
* `dumbdisplay.recordLayerSetupCommands()` -- start recording "setup" commands (freeze DumbDisplay screen)
* `dumbdisplay.playbackLayerSetupCommands("<setup-id>")` -- end recording "setup" commands and playback the recorded "setup" commands. **The argument `"<setup-id>"`, is the name for DumbDisplay app to persist the "setup" commands. When reconnect, those "setup" commands will be played back automatically.**

E.g.
```
  // start recording the commands to setup DD (app side)
  dumbdisplay.recordLayerSetupCommands();

  // create a 7-seg layer for 4 digits
  sevenSeg = dumbdisplay.create7SegmentRowLayer(4);
  sevenSeg->border(15, "darkblue", "round");
  sevenSeg->padding(10);
  sevenSeg->resetSegmentOffColor(DD_HEX_COLOR(0xeeddcc));

  // stop recording and play back the recorded commands
  // more importantly, a "id" is given so that
  // the records commands can be reused during restart of DD app 
  dumbdisplay.playbackLayerSetupCommands("up4howlong");
```

For the complete sketch of the above example, please refer to https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddup4howlong/ddup4howlong.ino


## More "Feedback" Options

Besides the usual `CLICK`, `DOUBLECLICK` and `LONGPRESS` "feedback" types are also possible.

For example, if want to modify previously mentioned *"Layer feedback"* sample to only clear dots on double-check, can change the code in `FeedbackHandler()`

from 

```
    if (pLayer == pLcdLayer) {
        // clicked the "clear" button
        pLayer->backgroundColor("white");
        Reset();
        delay(100);
        pLayer->backgroundColor("lightgray");
    }
```

to

```
    if (pLayer == pLcdLayer) {
        // clicked the "clear" button
        if (type == DOUBLECLICK) {
          pLayer->backgroundColor("white");
          Reset();
          delay(100);
          pLayer->backgroundColor("lightgray");
        }
    }
```

Additionally, if want to make use of the "auto repeat" option, can change code like:

from 

```
pTurtleLayer->setFeedbackHandler(FeedbackHandler, "fs");
```

to

```
pTurtleLayer->setFeedbackHandler(FeedbackHandler, "fs:lprpt50");
```

This simple change will enable "auto repeat" option -- as long as still pressed, long press feedback will auto repeat every 50ms.

Better yet, if want the dragging to be trigger as so as possible, can use the option like

```
pTurtleLayer->setFeedbackHandler(FeedbackHandler, "fs:rpt50");
```

Like "lprpt50", "rpt50" enables "auto repeat" option. The difference is that "rpt50" will simulate dragging -- clicking continuously.

As a reference, you may want to refer to the servo project -- https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/projects/servo/servo.ino 

For a brief explanation of the sketch, you may want to watch the video [**ESP8266 Servo Arduino experiment, subsequently, with simple DumbDisplay UI**](https://www.youtube.com/watch?v=pgfL_qwj8cQ)


| ESP8266 with Servo | DumbDisplay |
|--|--|
|![](screenshots/esp8266-servo.jpg)|![](screenshots/servo-dd.png)|

As a matter of fact, there is more a realistic dragging option. To enable such "drag" option, specify it like
```
pGrahpicalLayer->setFeedbackHandler(FeedbackHandler, "fs:drag");
```
***Note that such "drag" will always end with a "feedback" with x and y both -1.***

In case you want to specify the "end" X/Y value, say as -9999, you can do so by using option like `"fs:drag-9999"`, like. 
```
pTurtleLayer->setFeedbackHandler(FeedbackHandler, "fs:drag-9999");
```

|||
|--|--|
|For a complete example, please refer to the sketch as shown in the YouTube -- [Building a DL model for the Mnist Dataset, to building an Arduino Sketch for ESP32S3 (also ESP32)](https://www.youtube.com/watch?v=cL1-5BKJu30) The drawing of the hand-written digit is basially triggered by "drag" "feedbacks" |![](screenshots/esp32_mnist.gif)|



## Idle Callback and ESP32 Deep Sleep

It is possible to setup ESP32 to go to deep sleep when DumbDisplay library detects "idle", after, say, 15 seconds:

```
void IdleCallback(long idleForMillis) {
  if (idleForMillis > 15000) {  // go to sleep if idle for more than 15 seconds
    esp_sleep_enable_timer_wakeup(5 * 1000 * 1000);  // wake up in 5 seconds
    esp_deep_sleep_start();
  }
}
...
void setup() {
  ...
  dumbdisplay.setIdleCalback(IdleCallback);
  ...
}
```

For reference, you may want to refer to the example as shown by the video [**ESP32 Deep Sleep Experiment using Arduino with DumbDisplay**](https://www.youtube.com/watch?v=a61hRLIaqy8) 


## Using "Tunnel" to Download Images from the Web

It is possible to download image from the Web, save it to your phone, and draw it out to a graphical DD Layer.

This is done via an "image download tunnel" that you can create like

```
pTunnel = dumbdisplay.createImageDownloadTunnel("https://placekitten.com/680/480", "downloaded.png");
```

As preparation, you will need to grant DumbDisplay app permission to access your phone's storage.

Select the menu item ***settings*** and click the button ***access images***. This will trigger Android to ask for permission on behalf of DumbDisplay app, to access your phone's picture storage.

Once permission granted, DumbDisplay app will create a private folder, and write a small sample image `dumbdisplay.png` there. From now on, DumbDisplay will access the folder for any image files that it will need to read / write.

Since it takes a bit of time to download image file from the Web, you will need to check it's download status asyncrhonously like

```
while (true) {
  ...
  int result = pTunnel->checkResult();
  if (result == 1) {
    // web image downloaded and saved successfully
    ...
    break;
  } else if (result == -1) {
    // failed to download the image
    ...
    break;
  }
  ...
}
```

When the image downloaded and saved successfully, you can draw it to a graphical DD layer like

```
pLayer->drawImageFileFit("downloaded.png");
```

For a complete sample, please refer to the sample sketch https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/webimage/webimage.ino 



## Save Pictures to Phone Captured with ESP32 Cam

DumbDisplay Arduino Library provides a mechanism to save pictures captured, like with ESP32 Cam, to you Android phone's internal storage, like

```
  camera_fb_t *fb = esp_camera_fb_get();
  ...
  dumbdisplay.saveImage("esp32-cam-captured-image.jpg", fb->buf, fb->len);
```

For a complete sample, please refer to the sample sketch https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/esp32camddtest/esp32camddtest.ino 

You may also want to watch the YouTube Video ESP32-CAM Experiment -- [**Capture and Stream Pictures to Mobile Phone**](https://www.youtube.com/watch?v=D0tinZi5l5s) -- for a brief description of the experiment.


## Caching Single-bit Bitmap to Phone

A single-bit bitmap image is a common image format for displaying image in Arduino environment. And here in DumbDisplay, such 
image is referred to as pixel image (as opposed to full colored image).

To certain extend, DumbDisplay supports displaying these pixel images to graphical image as well. 
Two steps are involves:

* The pixel image is transferred to DumbDisplay at initialization time, like

```
// 'phone', 24x24px
const unsigned char phoneBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x81, 0xfc, 0x3f, 0xc3, 0xfc, 0x3f, 
  0xe7, 0xfc, 0x39, 0xe7, 0x9c, 0x38, 0x66, 0x1e, 0x3f, 0xe7, 0xfc, 0x79, 0xe7, 0x9e, 0x78, 0x66, 
  0x1e, 0x7f, 0xe7, 0xfe, 0x39, 0xe7, 0x9c, 0x78, 0x66, 0x1e, 0x7f, 0xe7, 0xfe, 0x3f, 0xe7, 0xfc, 
  0x7f, 0xe7, 0xfc, 0x3f, 0xe7, 0xfc, 0x01, 0xe7, 0x80, 0x00, 0x66, 0x00, 0x00, 0x24, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
...
  void setup() {
    ...
    unsigned char buffer[240];
    display->cachePixelImage("phone.png", PgmCopyBytes(phoneBitmap, sizeof(phoneBitmap), buffer), 24, 24, COLOR_1);
    ...
  }
```
Under the hook, the pixel image is actually converted to, say in this case, PNG format.
* The cached pixel image is displayed to graphical layer as needed, like
```
  ...
  display->drawImageFile("phone.png", 0, 0); 
  ...
```
Notice how the previously mentioned display image file command is used here.


| |  |
|--|--|
|For a complete example, and much more than just displaying bitmap image, please refer to my adaption of the "Pocket Computer" Arduino Nano project I found in YouTube -- [Arduino Pocket Computer featuring calculator, stopwatch, calendar, game and phone book](https://www.youtube.com/watch?v=NTaq6f7NV5U) by Volos Projects|![](screenshots/ddpocketcomputer.png)|



## Caching 16-bit Colored Bitmap to Phone

Not only can you cache single-bit bitmap to your phone, you can cache 16-bit (5-6-5) colored bitmap to your phone, like
```
  const uint16_t rocket[] PROGMEM = { ... };
  ...
  void setup() {
    ...
    display->cachePixelImage16("rocket.png", rocket, 24, 12);
    ...
  }
```
The cached 16-bit pixel image is displayed to graphical layer as needed, like
```
  ...
  display->drawImageFile("rocket.png", 0, 0); 
  ...
```

| | |
|--|--|
|In fact, I guess a better strategy will be to download the needed images, and use it in your sketch, as demonstrated by my post [Adaptation of "Space Wars" Game with DumbDisplay](https://www.instructables.com/Adaptation-of-Space-Wars-Game-With-DumbDisplay/).|![](screenshots/ddspacewars.gif)|


## Saving Images for DumbDisplay

Better than sending image data from microcontroller to DumbDisplay app every time, you may want to save the images to DumbDisplay app image storage, for the use by your sketch. As hinted by the post, the steps can be like
| | |
|--|--|
|1) use your phone's Chrome browser to open the image page; <br>2) long press the image to bring up the available options; <br>3) select to share the image with DumbDisplay app|![](screenshots/ddsaveimages.gif)|

Notes:
* not only from Chrome, you can share and save images from any app that can share images that it sees
* images saved to DumbDisplay app's image storage will always be PNG; hence when asked for image name, you don't need the ".png" extension
* you can use a file manager to navigate to the image storage; hints: the path is something like `/<main-storage>/Android/data/nobody.trevorlee.dumbdisplay/files/Pictures/DumbDisplay/`


## Audio Supports 

| | |
|--|--|
|![](screenshots/esp32-mic.png)|DumbDisplay has certain supports of Audio as well. You may want to refer to [ESP32 Mic Testing With INMP441 and DumbDisplay](https://www.instructables.com/ESP32-Mic-Testing-With-INMP441-and-DumbDisplay/) for samples on DumbDisplay audio supports. Additionally, you may also be interested in a more extensive application -- [Demo of ESP-Now Voice Commander Fun With Wit.ai and DumbDisplay](https://www.youtube.com/watch?v=dhlLU7gmmbE)|


# Reference

For reference, you may want to resort to the headers of the different related classes. To better display the headers, [Doxygen](https://www.doxygen.nl/index.html) is used to generate doc HTML pages autmoatically -- https://trevorwslee.github.io/ArduinoDumbDisplay/html



# DumbDispaly WIFI Bridge

Very likely you will be using your desktop computer (say Windows) for Arduino development, which will be connecting to your microcontroller via Serial connection. Wouldn't it be nice to be able to connect to DumbDisplay similarly, via the same Serial wiring as well?

Yes, you can achieve similar effect, with the help of the simple DumbDisplay WIFI Bridge Python program -- `tools/DDWifiBrideg/DDWifiBridge.py`. DumbDisplay WIFI Bridge acts as a "bridge" / "proxy" between your microcontroller (Serial connection) and your mobile phone (WIFI connection). 

![](screenshots/ddwifibridge.png)

When running the DumbDisplay WIFI Bridge, on one side, it connects to your microcontroller board via Serial connection, similar to how you Arduino IDE connect to your microcontroller board. At the same time, it listens on port 10201 of your desktop, allowing DumbDisply to establish connection via WIFI. In other words, your desktop computer port 10201 is now a "bridge" / "proxy" to your sketch (with DumbDisplay). 

Notes:
* There is also a seperate repository for DumbDisply WIFI Bridge -- https://github.com/trevorwslee/DDWifiBridge 
* DumbDisply WIFI Bridge makes use of the PySerial library, which can be install like
  ```
  pip install pyserial
  ```
* In Linux, acessing serial port will need special access right; you can grant such right to yourself (the user) like
  ```
  sudo usermod -a -G dialout <user>
  ```  
* My own experience is that using a slower serial baud rate (like 57600 or even lower like 9600) will make the connection more stable.
* ***If DumbDisplay fails to make connection to DumbDisplay WIFI Bridge, check your descktop firewall settings; try switching desktop WIFI to use 2.4 GHz.***  


For example, when something like the above Graphical [LCD] example is run with DumbDisplay WIFI Bridge as well as an Android emulator (e.g. [Genymotion](https://www.genymotion.com/)), you can see something like:

![](screenshots/ddwifibridge2.png)

You may want to watch the video [**Bridging Arduino UNO and Android DumbDisplay app -- DumbDisplayWifiBridge**](https://www.youtube.com/watch?v=0UhRmXXBQi8)


# DumbDisplay App Hints

* Many command parameters sent will be encoded for compression, and will look a bit cryptic (when shown on DumbDisplay app terminal view). If you want to disable parameter encoding, define `DD_DISABLE_PARAM_ENCODEING` before including `dumbdisplay.h`, like

  ```
  #define DD_DISABLE_PARAM_ENCODEING
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
  ```

* In fact, showing commands on DumbDisplay app may slow things down, even makes your DumbDisplay app non-responsive/freeze, especially when commands are sent in fast succession. Hence, suggest to disable DumbDisplay app's `Show Commands` option.

* Setting DumbDisplay app's `Pixel Density` to **Medium** will make the layer's text and other aspects look better. Setting it to **High** or even **Fine** would be very taxing to your phone. If want better looking text, but don't want to pay the price, try setting it to **Over**. Hopefully, **Over** is less taxing, since the text rendering is implemented differently, resulting in rendered text sligtly "over" the boundary where it should be, but looks better 

|**Normal**|**Medium**|**High**|**Fine**|**Over**|
|--|--|--|--|--|
|![](screenshots/dd_pixel_density_normal.png)|![](screenshots/dd_pixel_density_medium.png)|![](screenshots/dd_pixel_density_high.png)|![](screenshots/dd_pixel_density_fine.png)|![](screenshots/dd_pixel_density_over.png)|


* You can drag the bottom left/right side of the DumbDisplay canvas to have it resized.
* You can pinch-zoom the DumbDisplay canvas to resize it as well, if `Allow Zoom` is off. BTW, with the experimental `Allow Zoom` on, pinch-zooming the DumbDisplay canvas will eventually zoom it. When it is zoomed, it will not produce any "feedback". You double-click the canvas to return it to normal size.
* You can long press the terminal view to disable it's autoscrolling. BTW, terminal view has the `Keep Lines` limit, which you set with the `Setting` page. And this `Keep Lines` can certainly affect how much memory DumbDisplay will be used, should you have so much lines to be display by the terminal view.



# Thank You!

Greeting from the author Trevor Lee:

> Be good! Be happy!
> Peace be with you!
> May God bless you!
> Jesus loves you!


# License

MIT


# Change History

v0.9.8
  - enhanced documentation
  - added JoystickDDLayer
  - started to use Doxygen to generate doc HTML
  - bug fix

v0.9.7
  - enhanced graphical layer
  - added dragging "auto repeat" option
  - bug fix


v0.9.6-r3
  - bug fix

v0.9.6-r2
  - enhanced sound support
  - enhanced "auto pin"
  - bug fix

v0.9.6-r1
  - enhanced sound support
  - bug fix

v0.9.6
  - added sound support
  - enhanced JSON "tunnel"
  - bug fix

v0.9.5
  - can save/cache grayscale image
  - added TensorFlow Lite Object Detection Demo tunnel
  - bug fix

v0.9.4
  - enhanced "feedback"
  - can be a "share target" for saving images to storage
  - enhanced graphical layers
  - bug fixes

v0.9.3
  - enhanced graphical layers
  - bug fixes

v0.9.2
  - enhanced 7-segment layers
  - bug fixes

v0.9.1
  - added 'terminal' layer
  - bug fixes

v0.9.0
  - added 'service tunnel'
  - added TomTom map layer
  - big fixes

v0.8.4
  - added layer margin support
  - big fixes


v0.8.3
  - add more options for "JSON Tunnel"
  - bug fixes


v0.8.2
  - add "save image" support, basically for ESP32-Cam board
  - bug fixes


v0.8.1
  - aded "image download tunnel"
  - "JSON tunnel" now supports HTTPs (via Android phone)
  - added load/save/draw image to "graphical layer"
  - bug fixes


v0.8.0
  - added more basic layer functions
  - added more LCD layer functions
  - bug fixes


v0.7.9
  - added more "auto pin" functions
  - added "tone" to DumbDisplay
  - bug fixes

v0.7.7
  - minor changes

v0.7.6
  - added "idle callback"

v0.7.5
  - added auto repeat "feedback" option, which can be used to simulate dragging
  - added plotter layer `PlotterDDLayer`
  - added double-click / long-press "feedback"
  - bug fixes

v0.7.0
  - added ability to reconnect after disconnect
  - added ESP32 BLE connection support

v0.6.3
  - added simple JSON "tunnel" for calling simple Internet REST api
  - DDWifiBridge can now run as command-line tool without UI
  - bug fixes

v0.6.2
  - added capability to store recorded commands to phone
  - bug fixes

v0.6.1
  - added 'tunnel', to access Internet without special board support of WIFI connectivity
  - bug fixes

v0.6.0
  - added 'command buffering', sort of freeze the screen, until played back all at once
  - bug fixes

v0.5.1
- added DumbDispaly WIFI Bridge
- bug fixes

v0.5.0
- added WIFI support

v0.4.2
- added auto "feedback" (e.g. auto flashing of layer)
- added "auto pin" spacer -- DD_AP_SPACER
- added layer border
- adding WIFI support

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
