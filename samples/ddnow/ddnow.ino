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
BasicDDTunnel *datetimeTunnel;

void setup() {
  // create a graphical LCD layer for showing the current date-time got  
  graphical = dumbdisplay.createGraphicalLayer(200, 30);

  // create a LCD layer, as a button, to get click feedback
  button = dumbdisplay.createLcdLayer(12, 1);
  button->writeCenteredLine("check NOW");
  button->enableFeedback("fl");

  // auto pin the two layers created above vertically
  dumbdisplay.configAutoPin(DD_AP_VERT);

  // create a date-time service tunnel
  datetimeTunnel = dumbdisplay.createDateTimeServiceTunnel();
}


void loop() {
    if (button->getFeedback() != NULL) {
        // button clicked ==> get current date-time
        datetimeTunnel->reconnectTo("now");
    }
    String datetime;
    if (datetimeTunnel->readLine(datetime)) {
        // got current date-time feedback ==> display it
        graphical->clear();
        graphical->setCursor(0, 0);
        graphical->println("NOW:");
        graphical->println(datetime);
    }
    DDYield();  // give DumbDisplay a chance to do it's work
}


