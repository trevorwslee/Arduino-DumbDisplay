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

DumbDisplay dumbdisplay(new DDInputOutput(115200));


GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;

void setup() {
  // create a graphical layer for drawing the web images to
  graphical = dumbdisplay.createGraphicalLayer(200, 300);

  // create tunnels for downloading web images ... and save to your phone ... optionally: in order to send less duplicated data (in URL), create a map entry for R
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://${R=raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots}/lock-unlocked.png", "lock-unlocked.png");
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://${R}/lock-locked.png", "lock-locked.png");
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
