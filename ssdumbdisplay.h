#ifndef ssdumbdisplay_h
#define ssdumbdisplay_h

#include "dumbdisplay.h"
#include <SoftwareSerial.h>


class DDSoftwareSerialIO: public DDInputOutput {
  public:
    DDSoftwareSerialIO(SoftwareSerial* pSS, bool enableSerial): DDInputOutput(enableSerial) {
      this->pSS = pSS;
      pSS->begin(DUMBDISPLAY_BAUD);
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
  private:
    SoftwareSerial* pSS;  
};



#endif