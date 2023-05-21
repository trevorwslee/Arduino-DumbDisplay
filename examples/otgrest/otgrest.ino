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

  // setup a "tunnel" to get "current time" JSON; suggest to specify the buffer size to be the same as fields wanted
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




