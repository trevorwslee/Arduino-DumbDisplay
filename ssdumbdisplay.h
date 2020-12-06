#ifndef ssdumbdisplay_h
#define ssdumbdisplay_h

#include "dumbdisplay.h"
#include <SoftwareSerial.h>


class DDSoftwareSerialIO: public DDInputOutput {
  public:
    /* Software Serial IO mechanism */
    /* - baud -- default to DUMBDISPLAY_BAUD */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    DDSoftwareSerialIO(SoftwareSerial* pSS, unsigned long baud = DUMBDISPLAY_BAUD, bool enableSerial = false): DDInputOutput(enableSerial, enableSerial) {
      this->baud = baud;
      this->pSS = pSS;
    }
    bool available() {
      return pSS->available();
    }
    char read() {
      return pSS->read();
    } 
    void print(const char *p) {
      pSS->print(p); 
    }
    void preConnect() {
      DDInputOutput::preConnect();
      pSS->begin(baud/*DUMBDISPLAY_BAUD*/);
    }
  private:
    unsigned long baud;
    SoftwareSerial* pSS;  
};



#endif