

//#define BLUETOOTH

#ifdef BLUETOOTH

#include "ssdumbdisplay.h"

// assume HC-06 connected, to pin 2 and 3; and assume it is using baud 9600
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 9600));

#else

#include "dumbdisplay.h"

//otherwise, can use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif



GraphicalDDLayer *pLayer;
SimpleToolDDTunnel *pTunnel;

void setup() {
  // create a graphical layer for drawing the web image to
  pLayer = dumbdisplay.createGraphicalLayer(500, 300);
  pLayer->border(10, "azure", "round");  
  pLayer->noBackgroundColor();
  pLayer->penColor("navy");

  // create a tunnel for downloading web image ... initially, no URL yet ... downloaded.png is the name of the image to save
  pTunnel = dumbdisplay.createImageDownloadTunnel("", "downloaded.png");
}

void loop() {
    // set the URL to download web image ... using a bit different size so that web image will be different 
    String url = String("https://placekitten.com/680/") + String(460 + rand() % 20);
    pTunnel->reconnectTo(url);

    while (true) {
      int result = pTunnel->checkResult();
      if (result == 1) {
        // web image downloaded and saved successfully
        pLayer->clear();
        // unload to make sure the cache is cleared
        pLayer->unloadImageFile("downloaded.png");
        // draw the image
        pLayer->drawImageFileFit("downloaded.png", 10, 10);
      } else if (result == -1) {
        // failed to download the image
        pLayer->clear();
        pLayer->setCursor(10, 10);
        pLayer->println("XXX failed to download XXX");
      }
      if (result != 0) {
        break;
      }
    }
    pLayer->backgroundColor("azure");  // set background color
    pLayer->enableFeedback("f");       // enable "auto feedback" 
    while (true) {                     // loop and wait for layer clicked
      if (pLayer->getFeedback() != NULL) {
        break;
      }
    }
    pLayer->noBackgroundColor();  // no background color 
    pLayer->disableFeedback();    // disable "feedback"
    delay(500);
}
