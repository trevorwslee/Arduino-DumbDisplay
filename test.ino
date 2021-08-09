// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"


#define TEST_TUNNEL

//#define USE_BLUETOOTH

boolean enableSerial = true;
unsigned long serialBaud = 57600;
#ifdef USE_BLUETOOTH
unsigned long baud = DUMBDISPLAY_BAUD;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), baud, enableSerial, serialBaud));
#else
DumbDisplay dumbdisplay(new DDInputOutput(serialBaud));
#endif


#ifdef TEST_TUNNEL
BasicDDTunnel *pTunnel;
LedGridDDLayer *pLayer;
#endif

void setup() {
  if (!enableSerial) {
    // if DD not using Serial, setup Serial here
    Serial.begin(serialBaud);
  }

  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    DDLogToSerial("=== connected ===");
    dumbdisplay.writeComment("Good Day!");
  }

#ifdef TEST_TUNNEL
  pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17");
  dumbdisplay.writeComment("created tunnel");
  //pTunnel->write("hello world"); 
  pLayer = dumbdisplay.createLedGridLayer(3, 2);
  pLayer->offColor("green");
#endif  
}

void loop() {
#ifdef TEST_TUNNEL
  DDYield();
  if (pTunnel->eof()) {
    dumbdisplay.writeComment("EOF");
    pLayer->toggle();
    DDDelay(2000);
  } else if (pTunnel->avail()) {
    String data = pTunnel->read();
    dumbdisplay.writeComment(data);
    pLayer->toggle();
    //Serial.print("=" + data);
  }
#else
  bool forDebugging = false;
  BasicDDTestLoop(dumbdisplay, forDebugging);
#endif
}


