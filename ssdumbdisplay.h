#ifndef ssdumbdisplay_h
#define ssdumbdisplay_h

#include "dumbdisplay.h"
#include <SoftwareSerial.h>


class DDSoftwareSerialIO: public DDInputOutput {
  public:
    /* Software Serial IO mechanism */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    DDSoftwareSerialIO(SoftwareSerial* pSS, bool enableSerial = false): DDInputOutput(enableSerial, enableSerial) {
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
      pSS->begin(DUMBDISPLAY_BAUD);
    }
  private:
    SoftwareSerial* pSS;  
};



#endif