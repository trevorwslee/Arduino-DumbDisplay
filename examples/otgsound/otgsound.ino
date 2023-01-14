
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


// declare "YES" / "NO" lcd layers, acting as buttons ... they will be created in setup blick
LcdDDLayer* voiceYesLayer;
LcdDDLayer* voiceNoLayer;


void setup() {
  // create "YES" lcd layer, acting as a button
  voiceYesLayer = dumbdisplay.createLcdLayer(16, 3);
  voiceYesLayer->writeCenteredLine("YES", 1);
  voiceYesLayer->border(3, "green", "round");
  voiceYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "NO" lcd layer, acting as a button
  voiceNoLayer = dumbdisplay.createLcdLayer(16, 3);
  voiceNoLayer->writeCenteredLine("NO", 1);
  voiceNoLayer->border(3, "red", "round");
  voiceNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // auto "pin" the two layers vertically, one above the other
  dumbdisplay.configAutoPin(DD_AP_VERT);
}

void loop() {
  // check if "YES" clicked
  if (voiceYesLayer->getFeedback()) {
    // if so, play the pre-installed "YES" WAV file
    dumbdisplay.playSound("voice_yes.wav");
  }

  if (voiceNoLayer->getFeedback()) {
    // if so, play the pre-installed "NO" WAV file
    dumbdisplay.playSound("voice_no.wav");
  }
}