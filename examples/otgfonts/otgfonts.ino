/**
 * Sorry! Very likely, this sketch will not work for less-capable boards like Arduino Uno, Nano, etc. 
 */

/**
 * If BLUETOOTH is #defined, it uses ESP32 bluetooth connection
 * . BLUETOOTH is the name of the bluetooth device
 * If WIFI_SSID is #defined, it uses wifi connection
 * . once sketch running, connect to it with Serial Monitor to check for IP address
 * Otherwise, it uses USB connection (OTG) with the default 115200 baud
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
 * There is a related post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */


#include "dumbdisplay.h"

#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true));
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
#endif



void setup() {

  // create a graphical [LCD] layer
  GraphicalDDLayer *graphical = dumbdisplay.createGraphicalLayer(150, 300);
  graphical->backgroundColor("lightgray");
  graphical->setTextColor("blue");

  // the list of support downloadable fonts
  const int fontCount = 8;
  const String fonts[8] = { "B612", "Cutive", "Noto Sans", "Oxygen", "Roboto", "Share Tech", "Spline Sans", "Ubuntu" };
  
  // show font samples regular
  graphical->setTextFont();
  graphical->println("REGULAR:");
  for (int i = 0; i < fontCount; i++) {
      graphical->setTextFont();
      graphical->print(". ");
      const String& fontName = fonts[i];           // font-name like: Roboto
      graphical->setTextFont("DL::" + fontName);   // add prefix "DL::"
      graphical->println(fontName);
  }

  // show font samples monospace
  graphical->setTextFont();
  graphical->println();
  graphical->println("MONOSPACE:");
  for (int i = 0; i < fontCount; i++) {
      graphical->setTextFont();
      graphical->print(". ");
      String fontName = fonts[i] + " Mono";          // font-name like: Roboto Mono
      graphical->setTextFont("DL::" + fontName);     // add prefix "DL:"
      graphical->println(fontName);
  }
}

void loop() {

}
