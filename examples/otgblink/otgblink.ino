/**
 * to run and see the result of this sketch, you will need two addition things:
 * . you will need to install Android DumbDisplay app from Play store
 *   https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay
 * . although there are several ways for microcontroller board to establish connection
 *   with DumbDisplay app, here, the simple OTG USB connection is assume;
 *   hence, you will need an OTG adaptor cable for connecting your microcontroller board
 *   to your Android phone
 * . after uploading the sketch to your microcontroller board, plug the USB cable
 *   to the OTG adaptor connected to your Android phone
 * . open the DumbDisplay app and make connection to your microcontroller board via the USB cable;
 *   hopefully, the UI is obvious enough :)
 * . for more details on DumbDisplay Arduino Library, please refer to
 *   https://github.com/trevorwslee/Arduino-DumbDisplay#readme
 * there is a related blog post that you may want to take a look:
 * . https://create.arduino.cc/projecthub/trevorwslee/blink-test-with-virtual-display-dumbdisplay-5c8350
 */


#include "dumbdisplay.h"


// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));

// declare a LED layer object, to be created in setup()
LedGridDDLayer *led;


void setup() {
    // create the LED layer object, with only a single LED
    led = dumbdisplay.createLedGridLayer();
}


void loop() {
    // toggle the LED
    led->toggle();

    // delay for a second
    delay(1000);
}
