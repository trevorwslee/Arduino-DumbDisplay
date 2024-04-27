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

// actually, you have a choice here
// * if BLUETOOTH defined, it uses ESP32 bluetooth connection
// * if WIFI_SSID defined, it uses wifi connection
// * otherwise, it uses USB connection (OTG) with the default 115200 baud

#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true));
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#else
  // for direct USB connecction to phone
  // . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
  // . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
#endif



// declare a WebView layer
WebViewDDLayer *webView;


void setup() {
    // create the WebView layer object
    webView = dumbdisplay.createWebViewLayer(300, 300);

    // code the html
    String html = 
"<div style='width:100%;height:100%;display:flex;justify-content:center;align-items:center'>"    
  "<svg width='80vw' height='80vw' onclick='javascript:DD.feedbackWithText(\"click\",0,0,\"Hi, there!\")'>"
    "<defs>"
      "<radialGradient id='grad' cx='50%' cy='50%' r='50%' fx='50%' fy='50%'>"
        "<stop offset='0%' style='stop-color:rgb(0,255,0);stop-opacity:1' />"
        "<stop offset='100%' style='stop-color:rgb(255,0,0);stop-opacity:1' />"
      "</radialGradient>"
    "</defs>"
    "<circle cx='40vw' cy='40vw' r='40vw' fill='url(#grad)' />"
  "</svg>"
"</div>";

    // it is IMPORTANT to remove any newline characters from the html
    html.replace("\n", "");   

    // load the html into the WebView layer
    webView->loadHtml(html); 
}


void loop() {
  const DDFeedback* feedback = webView->getFeedback();
  if (feedback != NULL) {
    dumbdisplay.writeComment("* '" + feedback->text + "'");
  }
}
