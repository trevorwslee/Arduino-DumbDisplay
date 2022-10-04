// **********
// *** DumbDisplay app supports selected fonts downloadable from Google: https://fonts.google.com/
// *** Regular and monospace:
// *** . B612, Cutive, Noto Sans, Oxygen, Roboto, Share Tech, Spline Sans, Ubuntu
// *** In order to ensure that these Google's fonts are ready when they are used, please check Settings | Pre-Download Fonts
// *********

#include "ssdumbdisplay.h"


#define USE_BLUETOOTH

#ifdef USE_BLUETOOTH
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));
#else
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif

void setup() {

  // create a graphical [LCD] layer
  GraphicalDDLayer *graphical = dumbdisplay.createGraphicalLayer(150, 300);
  graphical->backgroundColor("lightgray");
  graphical->setTextColor("blue");

  char* fonts[] = { "B612", "Cutive", "Noto Sans", "Oxygen", "Roboto", "Share Tech", "Spline Sans", "Ubuntu" };
  
  // regular
  graphical->setTextFont();
  graphical->println("REGULAR:");
  for (int i = 0; i < sizeof(fonts) / 2; i++) {
      graphical->setTextFont();
      graphical->print(". ");
      String fontName = String(fonts[i]);          // font-name like: Roboto
      graphical->setTextFont("DL::" + fontName);   // add prefix "DL:"
      graphical->println(fontName);
  }

  // monospace
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


