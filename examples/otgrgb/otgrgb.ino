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
 * there is a related post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */


/******
 ****** 
 * Due to the large amount of data exchanged between the microcontroller board and the DumbDisplay app,
 * strongly suggest to turn off DumbDisplay app's "show commands"!
 ****** 
 ****** 
*/


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

