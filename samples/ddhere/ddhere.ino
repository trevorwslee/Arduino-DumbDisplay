#include "ssdumbdisplay.h"


#define BLUETOOTH


#ifdef BLUETOOTH


// assume HC-06 connected, to pin 2 and 3
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true));

#else

// otherwise, can use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif



GraphicalDDLayer *graphical;
LcdDDLayer *button;
GpsServiceDDTunnel *gpsTunnel;

void setup() {
  // create a graphical LCD layer for showing the current date-time got  
  graphical = dumbdisplay.createGraphicalLayer(200, 30);

  // create a LCD layer, as a button, to get click feedback
  button = dumbdisplay.createLcdLayer(12, 1);
  button->writeCenteredLine("check HERE");
  button->enableFeedback("fl");

  // auto pin the two layers created above vertically
  dumbdisplay.configAutoPin(DD_AP_VERT);

  // create a GPS service tunnel
  gpsTunnel = dumbdisplay.createGpsServiceTunnel();
}


void loop() {
    if (button->getFeedback() != NULL) {
        // button clicked ==> request GPS location
        gpsTunnel->reconnectForLocation();
    }
    DDLocation location;
    if (gpsTunnel->readLocation(location)) {
        // got GPS location feedback ==> display the location
        graphical->clear();
        graphical->setCursor(0, 0);
        graphical->println("LOC:");
        graphical->println("LAT:" + String(location.latitude, 4) + " / LONG:" + String(location.longitude, 4));
    }
    DDYield();  // give DumbDisplay a chance to do it's work
}


