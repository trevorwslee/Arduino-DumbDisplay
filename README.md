# DumbDisplay Arduino Library (v0.7.9)

DumbDisplay Ardunio Library enables you to utilize your Android phone as virtual output gadgets (as well as some simple inputting means) for your Arduino / ESP / STM32 / Respberry Pi Pico experiments.

You may want to watch the video **Introducing DumbDisplay -- the little helper for Arduino experiments** for a brief introduction -- https://www.youtube.com/watch?v=QZkhO6jTf0U

Plase notice that the above mentioned video is just one of the several that I have on using DumbDisplay to aid my own Arduino experiments -- https://www.youtube.com/watch?v=l-HrsJXIwBY&list=PL-VHNmqKQqiARqvxzN75V3sUF_wn1ysgV 


[ref](#desc)
[ref](Description)

# Description (manual desc)

Instead of connecting real gadgets to your Arduino IDE compatible micro-controller board for showing experiment results (or for getting simple input like clicking), you can make use of DumbDisplay for the purposes -- to realize virtual IO gadagets on your Android phone.

By doing so you can defer buying / connecting real gadgets until later stage of your experiment; also, you should be able to save a few micro-controller pins for other experiment needs.

A few types of layers can be created:
* LED-grid, which can also be used to simulate "bar-meter"
* LCD (text based)
* Micro:bit-like canvas
* Turtle-like canvas
* Graphical LCD, which is derived from the Turtle layer (i.e. in addition to general feaures of graphical LCD, it also has Turtle-like features) 
* 7-Segment-row, which can be used to display a series of digits, plus a decimal dot
* Plotter, which works similar to the plotter of DumbDisplay, but chart data provided by sending commands

Note that with the "layer feedback" mechanism, user interaction (like clicking of layers) can be routed back to the connected micro-controller, and as a result, the layers can be used as simple input gadgets as well.


# Installing DumbDisplay Arduino Library

## Arduino IDE

The basic steps are
1) download **CODE** ZIP file (the green button), from https://github.com/trevorwslee/Arduino-DumbDisplay
2) to install, use Arduino IDE menu option **Sktech** | **Include Library** | **Add .ZIP library...** and choose the ZIP you just downloaded

For demonstration on installing DumbDisplay Arduino Library, you may want to watch the video **Arduino Project -- HC-06 To DumbDisplay (BLINK with DumbDisplay)** -- https://www.youtube.com/watch?v=nN7nXRy7NMg

(To upgrade DumbDisplay Arduino Library, you will need to delete the old one first. You may refer to the video **Bridging Arduino UNO and Android DumbDisplay app -- DumbDisplayWifiBridge** -- https://www.youtube.com/watch?v=0UhRmXXBQi8&t=24s)


## PlatformIO

If you have an Arduino framework PlatformIO project that wants to make use of DumbDisplay Arduino Library, you can simply modify *platformio.ini* adding to *lib_deps* like:

```
lib_deps =
    https://github.com/trevorwslee/Arduino-DumbDisplay
```

(To upgrade DumbDisplay Arduino Library for that PlatformIO project, you can simply delete the 'depended libraries' directory `.pio/libdeps` to force all to be re-installed.)


# DumbDisplay Android app

Obviously, you will need to install an app on your Android phone. Indeed, for Arduino DumbDisplay to work, you will need to install the free DumbDisplay app from Android Play Store -- https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay

The app can accept connection via
* SoftwareSerial (e.g. Bluetooth by HC-06; even HC-08)
* BluetoothSerial (for ESP32)
* Bluetooth LE (for ESP32)
* WIFI (e.g. ESP01, ESP8266 and ESP32)
* Serial (USB connected via OTG adapter)
* Serial <-> WIFI via the simple included tool -- DumbDisplay WIFI Bridge (more on it later)
* Serial2 (hardware serial)

Notes:
* I have only tested DumbDisplay with the micro-controller boards that I have -- namely, Arduino Uno, ESP01, ESP8266, ESP32, STM32F103 and Raspberry Pi Pico.
* In case DumbDisplay does not "handshake" with your micro-controller board correctly, you can try resetting your Arduino by pressing the "reset" button on your Arduion.
* In certain use cases, and with a little bit of code change, DumbDisplay app can reconnect to your Arduino board after disconnect / app restart. Please refer to later section of this README.

 
# Sample Code

You have several options for connecting to DumbDisplay Android app.

* Via Serial 
  ```
    #include "dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDInputOutput());
  ```
  - need to include dumbdisplay.h -- `#include <dumbdisplay.h>`
  - setup a `dumbdisplay` object-- `DumbDisplay dumbdisplay(new DDInputOutput())`
  - doing so will **set Serial baud rate to the default 115200**, and you should not be using Serial for other purposes
  - note that **a lower baud rate, say 9600, may work better for some cases**
* Via `SoftwareSerial` (connected to Bluetooth module like HC-06) -- https://www.arduino.cc/en/Reference/softwareSerial
  ```
    #include "ssdumbdisplay.h"
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200));
  ```
  - need to include ssdumbdisplay.h -- `#include <ssdumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200))`  
    - 2 and 3 are the pins used by SoftwareSerial
    - **the default baud rate is 115200**, which seems to work better from my own testing with HC-06; however, you may want to test using lower baud rate in case connection is not stable; this is especially
    true for HC-08, which connects via BLE. 
  - you should not be using that SoftwareSerial for other purposes
  - again, **a lower baud rate, say 9600, may work better for some cases**  
* Via `Serial2` (for STM32, connected to Bluetooth module like HC-06) 
  ```
    #include "serial2dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDSerial2IO(115200));
  ```
  - need to include serial2dumbdisplay.h -- `#include <serial2dumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDSerial2IO(115200))`
  - e.g. STM32F103: PA3 (RX2) ==> TX; PA2 (TX2) ==> RX
* Via `Serial2` (for Raspberry Pi Pico, connected to Bluetooth module like HC-06) 
  ```
    #include <picodumbdisplay.h>
    DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));
  ```
  - need to include picodumbdisplay.h -- `#include <picodumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDPicoUart1IO(115200))`
  - **MUST** define DD_4_PICO_TX and DD_4_PICO_RX before including picodumbdisplay.h, like
  ```
    #define DD_4_PICO_TX 8
    #define DD_4_PICO_RX 9
    #include "picodumbdisplay.h"
    DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));
  ```
* Via **ESP32** `BluetoothSerial`
  ```
    #define DD_4_ESP32
    #include "esp32dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));
  ```
  - **MUST** define DD_4_ESP32 before `#include` -- `#define DD_4_ESP32`
  - include esp32dumbdisplay.h -- `#include <esp32dumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"))`  
    - "ESP32" is name used by `BluetoothSerial`
  - **you should not be using BluetoothSerial for other purposes**
* Via **ESP32** `BLE`
  ```
    #define DD_4_ESP32
    #include "esp32bledumbdisplay.h"
    DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32BLE"));
  ```
  - **MUST** define DD_4_ESP32 before `#include` -- `#define DD_4_ESP32`
  - include esp32bledumbdisplay.h -- `#include <esp32bledumbdisplay.h>`
  - setup a `dumbdisplay` object -- `DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32BLE"))`  
    - "ESP32BLE" is name used by `BLE`
  - **you should not be using ESP32's BLE for other purposes**
  - **be warned that `DDBLESerialIO` is slow**; if possible choose `DDBluetoothSerialIO` over `DDBLESerialIO` 
* Via WIFI as a `WiFiServer` (for ESP01/ESP8266/ESP32) -- https://www.arduino.cc/en/Reference/WiFi  
  ```
    #include "wifidumbdisplay.h"
    const char* ssid = "wifiname";
    const char* password = "wifipassword";
    const int serverPort = 10201;
    DumbDisplay dumbdisplay(new DDWiFiServerIO(ssid, password, serverPort));
  ```
  - WIFI credentials are passed to `WiFi`
  - By default, will setup and log using `Serial` with baud rate 115200; and you should see log lines like:
  ```
    binding WIFI wifiname
    binded WIFI wifiname
    listening on 192.168.1.134:10201 ...
  ```  
    where 192.168.1.134 is the "host" and 10201 is the "port"
  - It is only tested with ESP01, ESP8266 and ESP32 (which support WIFI without add-on) and it appears to be working fine when WIFI connection is stable (especially good for ESP8266).
  - With DumbDisply WIFI Bridge, you can simply code to use Arduino's Serial port to "connect" to DumbDisplay Android app, with DumbDisply WIFI Bridge running in the middle. Please refer to the section below that mention about DumbDisply WIFI Bridge.



With a DumbDisplay object, you are ready to proceed with coding, like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddblink/ddblink.ino

```
#include "ssdumbdisplay.h"

// assume HC-06 connected, to pin 2 and 3
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200));
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

Notes:
* DumbDisplay library will work cooperatively with your code, therefore, do give a change for DumbDisplay library chances to do its work. Please call `DDYeild()` and/or `DDDelay()` appropriately whenever possible. 
* Many commands sent will be "compressed", and will look a bit cryptic.
* In case a layer finishes all its usages in the middle of the sketch, it should be deleted in order for Arduino to claim back resources:
  ```
  dumbdisplay.deleteLayer(led);
  ```

## DumbDispaly "feedback" Mechanism

You can also try out "layer feedback" from DumbDisplay like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/samples/ddonoffloopmb/ddonoffloopmb.ino


```
#include "ssdumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

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


With the help of DumbDisplay WIFI Bridge (more on it in coming section), Arduino Uno can make use of DumbDisplay's "Tunnel" to get simple things from the Internet, like "quote of the day" from djxmmx.net.

In fact, DumbDisplay Android app also provides this "tunnel" feature; however, it appears that Android does not allow all connections, possibly due to the port restriction.

```
DumbDisplay dumbdisplay(new DDInputOutput(9600));
BasicDDTunnel *pTunnel;
void setup() {
 pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17"); 
}
void loop() {
  if (!pTunnel->eof()) {  // check not "reached" EOF
    if (pTunnel->count() > 0) {  // check something is there to read
      const String& data = pTunnel->readLine();    // read what got so far
      dumbdisplay.writeComment("{" + data + "}");  // write out what got as comment to DumbDisplay
    }
  } 
  DDDelay(200);  // delay a bit, and give DD a chance to so some work
}
```

In case a "tunnel" reached EOF, and need be reinvoked:

```
pTunnel->reconnect();
```

In case a "tunnel" finishes all its tasks in the middle of the sketch, it should be released in order for Arduino to claim back resources:

```
dumbdisplay.deleteTunnel(pTunnel);
```

In a more complicated case, you may want to get data from Internet open REST api that returns JSON. For simple "GET" case, `JsonDDTunnel` "tunnel" may be able to help:

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
  * HTTPS not yet supported

* use `count()` to check if the "tunnel" has anything to read, and use `read()` to read what got, like:
  ```
  while (!pTunnel->eof()) {
    while (pTunnel->count() > 0) {
      String fieldId;
      String fieldValue;
      pTunnel->read(fieldId, fieldValue);  // fieldId and fieldValue combined is a piece of JSON data 
      dumbdisplay.writeComment(fieldId + "=" + fieldValue);
    }
  }  
  ```
  note that `eof()` will check whether everything has returned and read before signaling EOF.


## More Samples


| 1. Micro:bit | 2. LEDs + "Bar Meter" + LCD | 3. Nested "auto pin" layers  | 4. Manual "pin" layers (LEDs + Turtle) | 5. Graphical [LCD] | 6. "Layer feedback" | 7. "Tunnel" for getting Quotes |
|--|--|--|--|--|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddmb.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddbarmeter.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddautopin.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddpinturtle.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddgraphical.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/dddoodle.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddquote.png)|


### Screenshot 1 -- *Micro:bit*

A more interesting sample would be like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddmb/ddmb.ino

```
#include "ssdumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
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

### Screenshot 2 -- *LEDs + "Bar Meter" + LCD*

An even more interesting sample would be like

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddbarmeter/ddbarmeter.ino

```
#include "ssdumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

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

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddautopin/ddautopin.ino

```
#include "ssdumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
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
```

### Screenshot 4 -- *Manual "pin" layers (LEDs + Turtle)*

To showcase Turtle, as well as the more controller way of "pinning" layers

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddpinturtle/ddpinturtle.ino

```
#include "ssdumbdisplay.h"

// assume HC-06 connected, to pin 2 and 3
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200));

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

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddgraphical/ddgraphical.ino

```
#include "ssdumbdisplay.h"

// assume HC-06 connected, to pin 2 and 3
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true));

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

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/dddoodle/dddoodle.ino

```
#include "ssdumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

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

### Screenshot 7 -- *"Tunnel" for getting Quotes*

This sample should demonstrate how to use "tunnel" to access the Internet for simple things, like "quote of the day" is this case:

https://github.com/trevorwslee/Arduino-DumbDisplay/blob/develop/samples/ddquote/ddquote.ino

```
#include "dumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
/* use a lower baud rate since Arduino Uno Serial buffer size is not very big                      */
DumbDisplay dumbdisplay(new DDInputOutput(9600));

GraphicalDDLayer *pLayer;
BasicDDTunnel *pTunnel;

bool gettingNewQuoto = true;

void setup() {
  // setup a "graphical" layer 
  pLayer = dumbdisplay.createGraphicalLayer(200, 150);  // size 200x150
  pLayer->border(10, "azure", "round");                 // a round border of size 10  
  pLayer->noBackgroundColor();                          // initial no background color
  pLayer->penColor("teal");                             // set pen color

  // setup a "tunnel" to access "quote of the day" by djxmmx.net
  pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17");  
}

void loop() {
    if (!pTunnel->eof()) {
      // not "reached" EOF (end-of-file)
      if (pTunnel->count() > 0) {
        // got something to read
        if (gettingNewQuoto) {
          // if just started to get the quote, reset something
          pLayer->clear();           // clear the "graphical" layer
          pLayer->setCursor(0, 10);  // set "cursor" to (0, 10)
        }
        String data = pTunnel->readLine();  // read what got so far
        pLayer->print(data);                // print out to the "graphical" layer what got so far
        gettingNewQuoto = false; 
      }
    } else {
      // "reached" EOF (i.e. got everything)
      // setup layer for getting "feedback" 
      pLayer->backgroundColor("azure");  // set background color
      pLayer->enableFeedback("f");       // enable "auto feedback" 
      while (true) {                     // loop and wait for layer clicked
        if (pLayer->getFeedback() != NULL) {
          break;
        }
      }
      // clicked ==> reset
      pLayer->noBackgroundColor();  // no background color 
      pLayer->disableFeedback();    // disable "feedback"
      pTunnel->reconnect();         // reconnect to djxmmx.net to get another quote
      gettingNewQuoto = true;       // indicating that a new quote is coming
    }
    DDDelay(500);  // delay a bit before another round; this also give DumbDisplay libary a chance to so its work
}
```

## Positioning of Layers

By default, layers are stacked one by one, with the one created first on the top of the stack. Each layer will be automatically stretched to fit the DumbDisplay, with the aspact ratio kept unchanged.

This behavior is not suitable for all cases. In fact, I would say that this default behavior is not suitable for many cases, except for the most simple case when you only have a single layer.

Actually, two mechanisms to "pin" the layers on DumbDisplay are provided -- automatic, and manual.

The automatic pinning of layers is the easier. You only need to call the DumbDisplay method `configAutoPin()` once to pin the layers, like
```
    dumbdisplay.configAutoPin(DD_AP_VERT_2(
                                DD_AP_HORI_2(
                                    pLedGridLayer->getLayerId(),
                                    pLcdLayer->getLayerId()),
                                pTurtleLayer->getLayerId()));
```
`DD_AP_HORI_2()` / `DD_AP_VERT_2()` macro pins 2 layers shoulder-to-shoulder horizontally / vertically. It accepts 2 arguments, with each one either a layer id, or another *DD_AP_XXX* macro. 

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

To get a feel, you may want to refer to the video *Raspberry Pi Pico playing song melody tones, with DumbDisplay control and keyboard input* -- https://www.youtube.com/watch?v=l-HrsJXIwBY 

|  | |
|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/pico-speaker_connection.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddmelody.jpg)|




## Record and Playback Commands

It is apparent that turning on LEDs, drawing on graphical LCDs, etc, by sending text-based commands is not particularly efficient. Indeed, screen flickering is a commonplace, especial when there are lots of activities.

In order to relieve this flickering situation a bit, it is possilbe to freeze DumbDisplay's screen during sending bulk of commands:
* `dumbdisplay.recordLayerCommands()` -- start recording commands (freeze DumbDisplay screen)
* `dumbdisplay.playbackLayerCommands()` -- end recording commands and playback the recorded commands (unfreeze Dumbdisplay screen)

A sample sketch demonstrates that this DumbDisplay feature can help; the sample is adapted from one that shows off Arduino UNO with Joystick shield connecting to a OLED display: https://cyaninfinite.com/interfacing-arduino-joystick-shield-with-oled-display

Instead of posting the sample sketch here, please find it with the link: https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/projects/joystick/joystick.ino

| Arduino UNO with Joystick shield | DumbDisplay |
|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/joystick-arduino.jpg)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/joystick-dd.png)|


If you are interested, you may want to watch the video **Arduino JoyStick Shield and DumbDisplay** -- https://www.youtube.com/watch?v=9GYrZWXHfUo

## Survive DumbDisplay App Reconnection

In certain "stateless" cases, like DumbDisplay is simply used as means to show values, it is possible for DumbDisplay to be able to meaningfully reconnect after DumbDisplay app disconnect / restart, since DumbDisplay app does not persist "state" information.

The only missing piece is the layout of the different layers. And this missing piece can be "regained" by recording the layout commands, and automatically playback when DumbDisplay app reconnects.

To do this, you simply need to enclose the "setup" code with the record/playback mechanism mentioned previously. 

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

For a brief explanation of the sketch, you may want to watch the video **ESP8266 Servo Arduino experiment, subsequently, with simple DumbDisplay UI** -- https://www.youtube.com/watch?v=pgfL_qwj8cQ


| ESP8266 with Servo | DumbDisplay |
|--|--|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/esp8266-servo.jpg)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/servo-dd.png)|


## Idle Callback and ESP32 Deep Sleep

It is possible setup ESP32 to go to deep sleep when DumbDisplay library detects "idle", after, say, 15 seconds:

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

For reference, you may want to refer to the example as shown by the video **ESP32 Deep Sleep Experiment using Arduino with DumbDisplay** -- https://www.youtube.com/watch?v=a61hRLIaqy8 


# Reference

For reference, please look into the declarations of the different related classes in the header files; mostly dumbdisplay.h -- https://github.com/trevorwslee/Arduino-DumbDisplay/blob/master/dumbdisplay.h



# DumbDispaly WIFI Bridge

Very likely you will be using your desktop computer (Windows) for Arduino development, which will be connecting to your Arduino board via Serial connection. Wouldn't it be nice to be able to connect to DumbDisplay similarly, via the same Serial wiring as well?

Yes, you can do exactly that, with the help of the simple DumbDisplay WIFI Bridge Python program -- tools/DDWifiBrideg/DDWifiBridge.py. DumbDisplay WIFI Bridge acts as a "bridge" / "proxy" between your Ardiono board (Serial connection) and your mobile phone (WIFI connection). 

When running the DumbDisplay WIFI Bridge, on one side, it connects to your Arduino board via Serial connection, similar to how you Arduino IDE connect to your Arduino board. At the same time, it listens on port 10201 of your desktop, allowing DumbDisply to establish connection via WIFI. In other words, your desktop computer port 10201 is now a "bridge" / "proxy" to your Arduino DumpDisplay code. 

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
* My own experience shows that using a slower serial baud rate (like 57600 or even lower like 9600) will make the connection more stable.
* ***If DumbDisplay fails to make connection to DumbDisplay WIFI Bridge, check your descktop firewall settings; try switching desktop WIFI to use 2.4 GHz.***  


![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddwifibridge.png)


You may want to watch the video **Bridging Arduino UNO and Android DumbDisplay app -- DumbDisplayWifiBridge** -- https://www.youtube.com/watch?v=0UhRmXXBQi8



# Thank You!

Greeting from the author Trevor Lee:

> Be good! Be happy!
> Peace be with you!
> Jesus loves you!


# License

MIT


# Change History


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
