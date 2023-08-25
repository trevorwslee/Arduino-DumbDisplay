#ifndef genericdumbdisplay_h
#define genericdumbdisplay_h

#if !defined(DD_SERIAL) || !defined(DD_SERIAL_begin)
  #error Must define the macro DD_SERIAL and DD_SERIAL_begin (a function call or a code block) \
    *** \
    e.g. STM32F103: PA3 (RX2) ==> TX; PA2 (TX2) ==> RX \
    #define DD_SERIAL Serial2 \
    *** \
    e.g. Pico \
    UART Serial2(8, 9, 0, 0);  // 8: PICO_TX; 9: PICO_RX \
    #define DD_SERIAL Serial2
#endif

#include "dumbdisplay.h"

/// Subclass of DDInputOutput
class DDGenericIO: public DDInputOutput {
  public:
    DDGenericIO(bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, enableSerial, enableSerial) {
    }
    bool available() {
      return DD_SERIAL.available();
    }
    char read() {
      return DD_SERIAL.read();
    } 
    void print(const String &s) {
      DD_SERIAL.print(s); 
    }
    void print(const char *p) {
      DD_SERIAL.print(p); 
    }
    void write(uint8_t b) {
      DD_SERIAL.write(b); 
    }
    void write(const uint8_t *buf, size_t size) {
      DD_SERIAL.write(buf, size); 
    }
    bool preConnect(bool firstCall) {
      //DD_SERIAL.begin(baud);
      if (firstCall) {
        DD_SERIAL_begin;
      } 
      return true;
    }
    void flush() {
    }
    bool canUseBuffer() {
      return true;
    }
};

#endif
