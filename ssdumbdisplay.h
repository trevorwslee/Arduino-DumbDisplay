#ifndef ssdumbdisplay_h
#define ssdumbdisplay_h

#include "dumbdisplay.h"
#include <SoftwareSerial.h>

#define DD_BLUETOOTH_BAUD DUMBDISPLAY_BAUD

class DDSoftwareSerialIO: public DDInputOutput {
  public:
    /* Software Serial IO mechanism */
    /* - baud -- default to DUMBDISPLAY_BAUD */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    /* - serialBaud: Serial baud rate (if enableSerial) */
    DDSoftwareSerialIO(SoftwareSerial* pSS, unsigned long baud = DD_BLUETOOTH_BAUD,
                       bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                         DDInputOutput(serialBaud, enableSerial, enableSerial) {
      this->baud = baud;
      this->pSS = pSS;
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
    void preConnect() {
      DDInputOutput::preConnect();
      pSS->begin(baud/*DUMBDISPLAY_BAUD*/);
    }
    void flush() {
      pSS->flush();
    }
  private:
    unsigned long baud;
    SoftwareSerial* pSS;  
};



#endif