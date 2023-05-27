#ifndef serial2dumbdisplay_h
#define serial2dumbdisplay_h

// e.g. STM32F103: PA3 (RX2) ==> TX; PA2 (TX2) ==> RX

#include "dumbdisplay.h"

/// Subclass of DDInputOutput
class DDSerial2IO: public DDInputOutput {
  public:
    DDSerial2IO(unsigned long baud,
                bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                DDInputOutput(serialBaud, enableSerial, enableSerial) {
      this->baud = baud;
    }
    bool available() {
      return Serial2.available();
    }
    char read() {
      return Serial2.read();
    } 
    void print(const String &s) {
      Serial2.print(s); 
    }
    void print(const char *p) {
      Serial2.print(p); 
    }
    void write(uint8_t b) {
      Serial2.write(b); 
    }
    void write(const uint8_t *buf, size_t size) {
      Serial2.write(buf, size); 
    }
    bool preConnect(bool firstCall) {
      Serial2.begin(baud);
      return true;
    }
    void flush() {
    }
    bool canUseBuffer() {
      return true;
    }
  private:
    unsigned long baud;  
};

#endif
