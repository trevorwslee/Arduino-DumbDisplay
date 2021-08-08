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
  BasicDDTunnel *pTunnel = dumbdisplay.createBasicTunnel("**test**");
  dumbdisplay.writeComment("created tunnel");
  pTunnel->write("hello world"); 
#endif  
}

void loop() {
  // bool forDebugging = false;
  // BasicDDTestLoop(dumbdisplay, forDebugging);
}


