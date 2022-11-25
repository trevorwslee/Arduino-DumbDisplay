// UNO built-in LED -- 13
// ESP32 built-in LED -- 2


#include "ssdumbdisplay.h"
#include "ddtester.h"

#define TEST_TUNNEL
#define TUNNEL_ECHO

#define USE_BLUETOOTH

unsigned long serialBaud = 57600;
#ifdef USE_BLUETOOTH
// setup to connect with bluetooth, as well as be able to connect with serial
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(11, 10), 115200, true, serialBaud));
#else
DumbDisplay dumbdisplay(new DDInputOutput(serialBaud));
#endif



#ifdef TEST_TUNNEL
BasicDDTunnel *pTunnel;
LedGridDDLayer *pLayer;
#endif

void setup() {
  dumbdisplay.debugSetup(13);  // setup to use pin 13

  if (true) {
    dumbdisplay.connect();  // explicitly connect (so that the following comment will show)
    dumbdisplay.logToSerial("=== connected ===");
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
  if (pTunnel == NULL) {
    dumbdisplay.writeComment("CLOSED");
    pLayer->toggle();
    DDDelay(1000);
  } else if (pTunnel->eof()) {
    dumbdisplay.writeComment("EOF");
    pLayer->toggle();
  #ifdef TUNNEL_ECHO
    pTunnel->reconnect();
  #else
    dumbdisplay.deleteTunnel(pTunnel);
    pTunnel = NULL;
  #endif
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

