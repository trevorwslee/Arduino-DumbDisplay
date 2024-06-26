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

DumbDisplay dumbdisplay(new DDInputOutput(115200));


void setup() {

  // create a graphical [LCD] layer
  GraphicalDDLayer *graphical = dumbdisplay.createGraphicalLayer(150, 300);
  graphical->backgroundColor("lightgray");
  graphical->setTextColor("blue");

  // the list of support downloadable fonts
  char* fonts[] = { "B612", "Cutive", "Noto Sans", "Oxygen", "Roboto", "Share Tech", "Spline Sans", "Ubuntu" };
  
  // show font samples regular
  graphical->setTextFont();
  graphical->println("REGULAR:");
  for (int i = 0; i < sizeof(fonts) / 2; i++) {
      graphical->setTextFont();
      graphical->print(". ");
      String fontName = String(fonts[i]);          // font-name like: Roboto
      graphical->setTextFont("DL::" + fontName);   // add prefix "DL:"
      graphical->println(fontName);
  }

  // show font samples monospace
  graphical->setTextFont();
  graphical->println();
  graphical->println("MONOSPACE:");
  for (int i = 0; i < sizeof(fonts) / 2; i++) {
      graphical->setTextFont();
      graphical->print(". ");
      String fontName = String(fonts[i]) + " Mono";  // font-name like: Roboto Mono
      graphical->setTextFont("DL::" + fontName);     // add prefix "DL:"
      graphical->println(fontName);
  }
}

void loop() {

}
