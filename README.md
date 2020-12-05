# DumbDisplay Arduino Library

DumbDisplay Ardunio Library enables you to utilize your Android phone as output gadgets for your Arduino experiments.

| LEDs + LCD | LEDs + LCD | Turtle |
|------------|------------|--------|
|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddledlcd.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddautopin.png)|![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddturtle.png)|


# Description

Instead of connecting real output gadgets to you Arduino for output your experiment results, you can make use of DumbDisplay for the purposes. Don't so may save you a few pins for your use by your other experiment need.


You can install the free DumbDisplay app from Android Play Store -- https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay


The app can accept connection via
* SoftwareSerial (e.g. Bluetooth via HC-06 @ 115200 baud)
* Serial (USB connected with OTG adapter)


# Sample Code

For Arduino, you have two options for making connection with DumbDisplay Android app.

* Via Serial
  - need to include dumbdisplay.h -- `#include <dumbdisplay.h>`
  - setup a `dumbdisplay` -- `DumbDisplay dumbdisplay(new DDInputOutput())`
* Via SoftwareSerial
  - need to include ssdumbdisplay.h -- `#include <ssdumbdisplay.h>`
  - setup a `dumbdisplay` -- `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true))`
    ; where 2 and 3 are pins used by the SoftwareSerial


### *LEDs and LCD*

With a DumbDisplay object, you are ready to proceed, first by creating some a LED-grid, and a LCD layer, like

  #include <ssdumbdisplay.h>
  
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));
  
  void setup() {
      // configure to "auto pin (layout) layers" in the vertical direction
      dumbdisplay.configAutoPin(DD_AP_VERT);
      
      // create a LED layer
      LedGridDDLayer *pLed = dumbdisplay.createLedGridLayer(3, 1);
      // create a LCD layer
      LcdDDLayer * pLcd = dumbdisplay.createLcdLayer();
      
      // turn on LEDs
      pLed->onColor("red");
      pLed->turnOn(0, 0);
      pLed->onColor("green");
      pLed->turnOn(1, 0);
      pLed->onColor("blue");
      pLed->turnOn(2, 0);
      
      // print messages to LCD
      pLcd->setCursor(2, 0);
      pLcd->print("Hello There!");
      pLcd->setCursor(2, 1);
      pLcd->print("How are you!");
  }


### *LCD and LEDs (as "Bar-Meter")*

Now, you may use the LED-grid as a "bar-meter", like

  #include <ssdumbdisplay.h>
  
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));
  
  void setup() {
      LedGridDDLayer *hmeter = NULL;
      
      // create LED layers that will be used for "horizontal bar-meter"
      // with max "bar" size 128
      hmeter = dumbdisplay.createLedGridLayer(128, 1, 1, 5);
      
      // set the "bar" to 64 (ie. 64 of total 128)
      hmeter->horizontalBar(64);
  }


### *LEDs and LCD (nested "auto pinning of layers")*

  #include <ssdumbdisplay.h>
  
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));
  
  LedGridDDLayer *rled = NULL;
  LedGridDDLayer *gled = NULL;
  LedGridDDLayer *bled = NULL;
  LedGridDDLayer *hmeter = NULL;
  LedGridDDLayer *vmeter = NULL;
  LcdDDLayer *lcd = NULL;
  
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



