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
TomTomMapDDLayer *tomtommap;
BasicDDTunnel *datetimeTunnel;
GpsServiceDDTunnel *gpsTunnel;


void prepareTunnels() {
  // create a date-time service tunnel
  datetimeTunnel = dumbdisplay.createDateTimeServiceTunnel();

  // create a GPS service tunnel
  gpsTunnel = dumbdisplay.createGpsServiceTunnel();
  gpsTunnel->reconnectForLocation(2);  // reconnect to service for GPS location, continuously
}


void setup() {
    // record the layout, basiclly for the ability of DD app reconnecting
    dumbdisplay.recordLayerSetupCommands();
  
    // create a graphical LCD layer for showing the current date-time got  
    graphical = dumbdisplay.createGraphicalLayer(230, 32);
    graphical->backgroundColor("azure");
    graphical->penColor("blue");
    graphical->border(3, "darkgray");

    // create a Tom Tom map layer
    tomtommap = dumbdisplay.createTomTomMapLayer("", 300, 200);  // *** a TomTom map api key is required; here, "" is used for demo purpose
    tomtommap->border(5, "darkgray", "round");

    // auto pin the two layers created above vertically
    dumbdisplay.configAutoPin(DD_AP_VERT);

    dumbdisplay.playbackLayerSetupCommands("rc-tomtommap");
}



DDConnectVersionTracker cvTracker;
String datetime;
DDLocation location;

void loop() {
    if (cvTracker.checkChanged(dumbdisplay)) {
        // there has been a reconnection ==> prepare tunnels
        prepareTunnels();
    }
    if (gpsTunnel->readLocation(location)) {
        // got GPS location feedback ==> sync TomTom map view
        tomtommap->goTo(location.latitude, location.longitude);
        dumbdisplay.writeComment("... wait a bit for the map ...");
        // reconnect to service for current date-time, one-shot
        datetimeTunnel->reconnectTo("now");
    }
    if (datetimeTunnel->readLine(datetime)) {
        // got current-date time, which is required after GPS location got ==> display it
        graphical->clear();
        graphical->setCursor(0, 0);
        graphical->println("🕰️: " + datetime);
        graphical->println("🌏: LAT=" + String(location.latitude, 4) + " / LONG=" + String(location.longitude, 4));
    }
    DDYield();  // give DumbDisplay a chance to do it's work
}
