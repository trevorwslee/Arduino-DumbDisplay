
//#define FOR_ESP32C3
#define BLUETOOTH

#ifdef FOR_ESP32C3
    #define DD_4_ESP32
    #include "esp32bledumbdisplay.h"
    // - use ESP32 BLE with name "ESP32C3"
    // - at the same time, enable Serial connection with 115200 baud 
    DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32C3", true));
#else
    #include "ssdumbdisplay.h"
    #ifdef BLUETOOTH
    // assume HC-06 connected, to pin 2 and 3
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true));
    #else
    // otherwise, can use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
    DumbDisplay dumbdisplay(new DDInputOutput(115200));
    #endif
#endif


GraphicalDDLayer *graphical;
TomTomMapDDLayer *tomtommap;
BasicDDTunnel *datetimeTunnel;
GpsServiceDDTunnel *gpsTunnel;

void setup() {
  // create a graphical LCD layer for showing the current date-time got  
  graphical = dumbdisplay.createGraphicalLayer(230, 32);
  graphical->backgroundColor("azure");
  graphical->penColor("blue");
  graphical->border(3, "darkgray");

  // create a Tom Tom map layer
  tomtommap = dumbdisplay.createTomTomMapLayer("", 300, 200);
  tomtommap->border(5, "darkgray", "round");

  // auto pin the two layers created above vertically
  dumbdisplay.configAutoPin(DD_AP_VERT);

  // create a date-time service tunnel
  datetimeTunnel = dumbdisplay.createDateTimeServiceTunnel();

  // create a GPS service tunnel
  gpsTunnel = dumbdisplay.createGpsServiceTunnel();
  gpsTunnel->reconnectForLocation(2);  // reconnect to service for GPS location, continuously
}


String datetime;
DDLocation location;

void loop() {
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
