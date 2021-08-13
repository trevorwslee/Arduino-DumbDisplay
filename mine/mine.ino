// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"

#define TEST_TUNNEL
#define TUNNEL_ECHO

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
  #ifdef TUNNEL_ECHO
    pTunnel = dumbdisplay.createBasicTunnel("192.168.0.203:12345");
  #else
    pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17");
  #endif
    dumbdisplay.writeComment("created tunnel");
  pLayer = dumbdisplay.createLedGridLayer(3, 2);
  pLayer->offColor("green");
#endif  
}

void loop() {
#ifdef TEST_TUNNEL
  //dumbdisplay.writeComment("$$$");
  if (pTunnel->eof()) {
    dumbdisplay.writeComment("{EOF}");
    pLayer->toggle();
    DDDelay(1000);
  } else {
  #ifdef TUNNEL_ECHO
    //dumbdisplay.writeComment("...");
    pTunnel->writeLine("hello world"); 
    DDDelay(1000);
  #endif    
    if (pTunnel->count() > 0) {
      const String& data = pTunnel->readLine();
      dumbdisplay.writeComment("{" + data + "}");
      pLayer->toggle();
      //Serial.print("=" + data);
    }
  }
#else
  bool forDebugging = false;
  BasicDDTestLoop(dumbdisplay, forDebugging);
#endif
}


