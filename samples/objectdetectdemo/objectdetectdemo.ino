
#include "Arduino.h"

// if want Bluetooth, uncomment the following line
// #define BLUETOOTH "ESP32BT"
#if defined(ESP32) && defined(BLUETOOTH) 
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true, 115200));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


GraphicalDDLayer *imageLayer;
SimpleToolDDTunnel *web_image_tunnel;
ObjectDetectDemoServiceDDTunnel *object_detect_tunnel;

void setup() {
  // create a graphical layer for drawing the web image to
  imageLayer = dumbdisplay.createGraphicalLayer(640, 480);
  imageLayer->border(10, "blue", "round");  
  imageLayer->padding(15);
  imageLayer->backgroundColor("white");
  imageLayer->penSize(2);

  // create a tunnel for downloading web image ... initially, no URL yet ... downloaded.png is the name of the image to save
  web_image_tunnel = dumbdisplay.createImageDownloadTunnel("", "downloaded.png");

  // create a tunnel for object detection demo via TensorFlow Lite running on phone side
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
  int maxNumObjs = 1;
#else
  int maxNumObjs = 3;
#endif
  object_detect_tunnel = dumbdisplay.createObjectDetectDemoServiceTunnel(0, 0, maxNumObjs);
}

inline const char* getDownloadImageURL() {
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#else
  // randomly pick an image source URL from a list
  int idx = random(5);
  switch(idx) {
    case 0: return "https://placekitten.com/640/480";
    case 1: return "https://source.unsplash.com/random/640x480";
    case 2: return "https://picsum.photos/640/480";
    case 3: return "https://loremflickr.com/640/480";
  }
#endif
  return "https://placedog.net/640/480?r";
}

void loop() {
    // set the URL to download web image ... using a bit different size so that web image will be different 
    String url = getDownloadImageURL();
    web_image_tunnel->reconnectTo(url);

    dumbdisplay.writeComment("...");
    while (true) {
      int result = web_image_tunnel->checkResult();
      if (result == 1) {
        // web image downloaded and saved successfully
        imageLayer->drawImageFile("downloaded.png");
        // detect objects in the image
        object_detect_tunnel->reconnectForObjectDetect("downloaded.png");
      } else if (result == -1) {
        // failed to download the image
        dumbdisplay.writeComment("XXX failed to download XXX");
      }
      if (result != 0) {
        break;
      }
    }

    imageLayer->backgroundColor("gray");   // set background color to gray, to indicate loaded and detecting
    imageLayer->enableFeedback("f") ;      // enable "auto feedback" 
    bool detected = false;
    while (true) {                        // loop and wait for object detection result, or graphical layer click for switching image
      if (object_detect_tunnel->eof()) {
        if (!detected) {
          dumbdisplay.writeComment("Click image to switch!");
          imageLayer->backgroundColor("blue");   // set background color to blue, to indicate all done
        }
        detected = true;
      } else {
        DDObjectDetectDemoResult objectDetectResult;
        if (object_detect_tunnel->readObjectDetectResult(objectDetectResult)) {
          dumbdisplay.writeComment(String(". ") + objectDetectResult.label);
          int x = objectDetectResult.left;
          int y = objectDetectResult.top;
          int w = objectDetectResult.right - objectDetectResult.left;
          int h = objectDetectResult.bottom - objectDetectResult.top;
          imageLayer->drawRect(x, y, w, h, "green");
          imageLayer->drawStr(x, y, objectDetectResult.label, "yellow", "a70%darkgreen", 32);
        }
      }
      if (imageLayer->getFeedback() != NULL) {
        break;
      }
    }
    imageLayer->backgroundColor("white");  // set background color to white, to indicate loading 
    imageLayer->disableFeedback();         // disable "feedback"
}
