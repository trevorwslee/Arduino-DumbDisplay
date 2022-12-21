
#include "Arduino.h"


#if defined(ESP32) 
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("32"));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


GraphicalDDLayer *graphical;
SimpleToolDDTunnel *web_image_tunnel;
ObjectDetetDemoServiceDDTunnel *object_detect_tunnel;

void setup() {
  // create a graphical layer for drawing the web image to
  graphical = dumbdisplay.createGraphicalLayer(640, 480);
  graphical->border(10, "blue", "round");  
  graphical->padding(15);
  graphical->backgroundColor("gray");
  graphical->penSize(2);

  // create a tunnel for downloading web image ... initially, no URL yet ... downloaded.png is the name of the image to save
  web_image_tunnel = dumbdisplay.createImageDownloadTunnel("", "downloaded.png");

  // create a tunnel for object detection demo via TensorFlow Lite running on phone side
  object_detect_tunnel = dumbdisplay.createObjectDetectDemoServiceTunnel();
}

const char* getDownloadImageURL() {
    int idx = random(5);
    switch(idx) {
      case 0: return "https://placekitten.com/640/480";
      case 1: return "https://source.unsplash.com/random/640x480";
      case 2: return "https://picsum.photos/640/480";
      case 3: return "https://loremflickr.com/640/480";
    }
    return "https://placedog.net/640/480?r";
}

void loop() {
    // set the URL to download web image ... using a bit different size so that web image will be different 
    String url = getDownloadImageURL();
    web_image_tunnel->reconnectTo(url);

    bool detecting = false;
    while (true) {
      int result = web_image_tunnel->checkResult();
      if (result == 1) {
        // web image downloaded and saved successfully
        graphical->clear();
        // draw the image
        graphical->drawImageFile("downloaded.png");
        // detect objects in the image
        object_detect_tunnel->reconnectForObjectDetect("downloaded.png");
        detecting = true;
      } else if (result == -1) {
        // failed to download the image
        dumbdisplay.writeComment("XXX failed to download XXX");
      }
      if (result != 0) {
        break;
      }
    }

    graphical->backgroundColor("blue");   // set background color to blue, to indicate loaded
    graphical->enableFeedback("f");       // enable "auto feedback" 
    while (true) {                        // loop and wait for object detection result, or grapical layer click for switching image
      if (detecting) {
        DDObjectDetectDemoResult objectDetectResult;
        if (object_detect_tunnel->readObjectDetectResult(objectDetectResult)) {
          dumbdisplay.writeComment(objectDetectResult.label);
          int x = objectDetectResult.left;
          int y = objectDetectResult.top;
          int w = objectDetectResult.right - objectDetectResult.left;
          int h = objectDetectResult.bottom - objectDetectResult.top;
          graphical->drawRect(x, y, w, h, "green");
          graphical->drawStr(x, y, objectDetectResult.label, "yellow", "", 32);
        }
      }
      if (graphical->getFeedback() != NULL) {
        break;
      }
    }
    graphical->backgroundColor("gray");  // set background color to gray, to indicate loading 
    graphical->disableFeedback();        // disable "feedback"
}
