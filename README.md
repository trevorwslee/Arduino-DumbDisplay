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
  - setup a `dumbdisplay` -- `DumbDisplay dumbdisplay(new DDInputOutput());`
* Via SoftwareSerial
  - need to include ssdumbdisplay.h -- `#include <ssdumbdisplay.h>`
  - setup a `dumbdisplay` -- `DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true))`
    . where 2 and 3 are pins used by the SoftwareSerial

    



