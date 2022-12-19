
#include "Arduino.h"


#if defined(ESP32) 
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("32", true, 115200));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


GraphicalDDLayer *graphical;
SimpleToolDDTunnel *web_image_tunnel;

void setup() {
  // create a graphical layer for drawing the web image to
  graphical = dumbdisplay.createGraphicalLayer(640, 480);
  graphical->border(10, "azure", "round");  
  graphical->noBackgroundColor();
  graphical->penColor("navy");

  // create a tunnel for downloading web image ... initially, no URL yet ... downloaded.png is the name of the image to save
  web_image_tunnel = dumbdisplay.createImageDownloadTunnel("", "downloaded.png");
}

const char* getDownloadImageURL() {
    int idx = random(4);
    switch(idx) {
      case 0: return "https://placedog.net/640/480?r";
      case 1: return "https://source.unsplash.com/random/640x480";
      case 2: return "https://picsum.photos/640/480";
      case 3: return "https://loremflickr.com/640/480";
    }
    return "https://placekitten.com/640/480";
}

void loop() {
    // set the URL to download web image ... using a bit different size so that web image will be different 
    String url = getDownloadImageURL();
    web_image_tunnel->reconnectTo(url);

    while (true) {
      int result = web_image_tunnel->checkResult();
      if (result == 1) {
        // web image downloaded and saved successfully
        graphical->clear();
        // draw the image
        graphical->drawImageFile("downloaded.png");
      } else if (result == -1) {
        // failed to download the image
        graphical->clear();
        graphical->setCursor(10, 10);
        graphical->println("XXX failed to download XXX");
      }
      if (result != 0) {
        break;
      }
    }
    graphical->backgroundColor("azure");  // set background color
    graphical->enableFeedback("f");       // enable "auto feedback" 
    while (true) {                     // loop and wait for layer clicked
      if (graphical->getFeedback() != NULL) {
        break;
      }
    }
    graphical->noBackgroundColor();  // no background color 
    graphical->disableFeedback();    // disable "feedback"
    delay(500);
}
