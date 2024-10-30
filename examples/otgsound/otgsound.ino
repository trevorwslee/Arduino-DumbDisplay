
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


#include "dumbdisplay.h"


// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));


// declare "YES" / "NO" lcd layers, acting as buttons ... they will be created in setup blick
LcdDDLayer* yesLayer;
LcdDDLayer* noLayer;


void setup() {
  // create "YES" lcd layer, acting as a button
  yesLayer = dumbdisplay.createLcdLayer(16, 3);
  yesLayer->writeCenteredLine("YES", 1);
  yesLayer->border(3, "green", "round");
  yesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "NO" lcd layer, acting as a button
  noLayer = dumbdisplay.createLcdLayer(16, 3);
  noLayer->writeCenteredLine("NO", 1);
  noLayer->border(3, "red", "round");
  noLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // auto "pin" the two layers vertically, one above the other
  dumbdisplay.configAutoPin(DD_AP_VERT);

}

void loop() {
  // check if "YES" clicked
  if (yesLayer->getFeedback()) {
    // if so, play the pre-installed "YES" WAV file
    dumbdisplay.playSound("voice_yes.wav");
  }

  // check if "NO" clicked
  if (noLayer->getFeedback()) {
    // if so, play the pre-installed "NO" WAV file
    dumbdisplay.playSound("voice_no.wav");
  }
}


