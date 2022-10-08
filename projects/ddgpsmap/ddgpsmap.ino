
//
// * assume OTG adaptor or DumbDisplayWifiBridge
// * will need software serial for reading data from NEO-7M U-BLOX
//

#include <SoftwareSerial.h>
#include "dumbdisplay.h"

#include "gpssignal.h"  // code for reading GPS signal from NEO-7M U-BLOX module ... not very intresting


#define NEO_RX 6   // RX pin of NEO-7M U-BLOX
#define NEO_TX 5   // TX pin of NEO-7M U-BLOX


SoftwareSerial gpsSerial(NEO_TX, NEO_RX);
GpsSignalReader gpsSignalReader(gpsSerial);


// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));

// declares several layers for the UI
GraphicalDDLayer* graphical;    // for showing the UTC time and position fixed, according to the readings of the GPS module
TerminalDDLayer* terminal;      // for showing trace of reading data from the GPS module
TomTomMapDDLayer *tomtommap;    // for showing the location fixed, on a TomTom map


long startMillis;  // the "time" the sketch starts running
long lastMillis;   // the "time" "waiting progress" (a dot) is printed

void setup() {
  startMillis = millis();
  lastMillis = startMillis;

  // it appears that it is better to explicityly make connection to DD app first
  dumbdisplay.connect();

  // the default UART baud rate for communicating with NEO-7M is 9600
  gpsSerial.begin(9600);

  // create the layers, as described above
  graphical = dumbdisplay.createGraphicalLayer(250, 30);
  graphical->setTextFont("DL::Ubuntu Mono");  // DL::Ubuntu Mono ==> specify that the downloadable Google font Ubuntu Monto should be used
  graphical->border(3, "black");
  graphical->padding(3);
  graphical->penColor("blue");
  graphical->backgroundColor("white");
  terminal = dumbdisplay.createTerminalLayer(600, 800);
  terminal->border(5, "blue");
  terminal->padding(5);
  tomtommap = dumbdisplay.createTomTomMapLayer("", 600, 800);  // *** a TomTom map api key is required; here, "" is used for demo purpose
  tomtommap->border(5, "blue");
  tomtommap->padding(5);
  tomtommap->visible(false);  // initially hidden

  // 'pin' the layers one by one vertically
  //  note that tomtommap is not visible initially
  dumbdisplay.configAutoPin(DD_AP_VERT);

}

bool waited = false;  // a flag indicating that the sketch waited for GPS readings
bool mapVisible = false;
GpsSignal gpsSignal;  // a structure for receiving GPS readings

void loop() {
  long nowMillis = millis();
  if ((nowMillis - lastMillis) > 2000) {
    // if it has been 2000 milli-seconds since last print "waiting progress" (a dot), print it again
    terminal->print(".");
    waited = true;
    lastMillis = nowMillis;
  }
  if (gpsSignalReader.readOnce(gpsSignal)) {  // try read GPS data once
    // read GPS data
    if (waited) {
      // waited before (i.e. dots printed before), terminate the dots
      terminal->println();
      waited = false;
    }
    // print out the GPS info read
    terminal->print("- utc: ");  // UTC time
    terminal->print(gpsSignal.utc_time);
    terminal->print(" ... ");
    if (gpsSignal.position_fixed) {
      // if position fixed, print the latitute / longitude / altitude of the position
      terminal->print("position fixed -- ");
      terminal->print("lat:");
      terminal->print(gpsSignal.latitude);
      terminal->print(" long:");
      terminal->print(gpsSignal.longitude);
      terminal->print(" alt:");
      terminal->print(gpsSignal.altitude);
    } else {
      // position not yet fixed
      terminal->print("position NOT fixed");
    }
    terminal->println();
    // print out the UTC time, and position fixed to the graphical layer
    graphical->clear();
    graphical->setCursor(0, 0);
    graphical->println("UTC -- " + String(gpsSignal.utc_time));
    graphical->print("LOC -- ");
    if (gpsSignal.position_fixed) {
      graphical->println("LAT:" + String(gpsSignal.latitude, 4) + " / LONG:" + String(gpsSignal.longitude, 4));
    } else {
      graphical->print("not fixed");
    }
    if (gpsSignal.position_fixed) {
      // if position fixed, show the location on the TomTom map
      tomtommap->zoomTo(gpsSignal.latitude, gpsSignal.longitude);
    }
    if (((nowMillis - startMillis) > 20000) && gpsSignal.position_fixed) {
      if (!mapVisible) {
        // hide terminal, and make tomtommap visible ... notice the "20000 millis from start" requirement
        terminal->visible(false);
        tomtommap->visible(true);
        mapVisible = true;
      }
    }  
    lastMillis = nowMillis;
  }
}
