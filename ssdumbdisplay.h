#ifndef ssdumbdisplay_h
#define ssdumbdisplay_h

#include "dumbdisplay.h"
#include <SoftwareSerial.h>

//#define DD_BLUETOOTH_BAUD DUMBDISPLAY_BAUD

/// Subclass of DDInputOutput
class DDSoftwareSerialIO: public DDInputOutput {
  public:
    /* Software Serial IO mechanism */
    /* - baud -- default to DUMBDISPLAY_BAUD */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    /* - serialBaud: Serial baud rate (if enableSerial) */
    DDSoftwareSerialIO(SoftwareSerial* pSS, unsigned long baud = DD_SERIAL_BAUD,
                       bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                       DDInputOutput(serialBaud, enableSerial, enableSerial) {
      this->baud = baud;
      this->pSS = pSS;
    }
    const char* getWhat() {
      return "SS";
    }
    bool available() {
      return pSS->available();
    }
    char read() {
      return pSS->read();
    } 
    void print(const String &s) {
      pSS->print(s); 
    }
    void print(const char *p) {
      pSS->print(p); 
    }
    void write(uint8_t b) {
      pSS->write(b); 
    }
    void write(const uint8_t *buf, size_t size) {
      pSS->write(buf, size); 
    }
    bool preConnect(bool firstCall) {
      DDInputOutput::preConnect(firstCall);
      // if (!setupForSerial) {
      //   Serial.begin(serialBaud);
      // }
      if (firstCall) {
        pSS->begin(baud);
      }
      //pSS->begin(baud/*DUMBDISPLAY_BAUD*/);
      return true;
    }
    void flush() {
      pSS->flush();
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return false;
    }
  private:
    unsigned long baud;
    SoftwareSerial* pSS;  
};



#endif