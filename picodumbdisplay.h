#ifndef picodumbdisplay_h
#define picodumbdisplay_h

// #if !(defined DD_4_PICO_TX && defined DD_4_PICO_RX)
//   #error DD_4_PICO_TX and DD_4_PICO_RX need be defined in order to use DumbDisplay for PICO
//   #error e.g. #define DD_4_PICO_TX 8
//   #error e.g. #define DD_4_PICO_RX 9
// #endif

#include "dumbdisplay.h"

//UART Serial2(8, 9, 0, 0);
//UART Serial2(DD_4_PICO_TX, DD_4_PICO_RX, 0, 0);



/// Deprecated. Use DDGenericIO (in genericdumbdisplay.h) instead.
class DDPicoSerialIO: public DDInputOutput {
  public:
    DDPicoSerialIO(int tx = 8, int rx = 9,
                  unsigned long baud = DD_SERIAL_BAUD,
                  bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                  DDInputOutput(serialBaud, enableSerial, enableSerial), serial(tx, rx, 0, 0) {
      this->baud = baud;
      // if (!enableSerial) {
      //   Serial.begin(serialBaud);  // not a good idea to do it here
      // }
    }
    const char* getWhat() {
      return "PICO";
    }
    bool available() {
      return serial.available();
    }
    char read() {
      return serial.read();
    } 
    void print(const String &s) {
      serial.print(s); 
    }
    void print(const char *p) {
      serial.print(p); 
    }
    void write(uint8_t b) {
      serial.write(b); 
    }
    void write(const uint8_t *buf, size_t size) {
      serial.write(buf, size); 
    }
    bool preConnect(bool firstCall) {
      if (firstCall) {
        serial.begin(baud);
      }
      return true;
    }
    void flush() {
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return true;
    }
  private:
    UART serial;
    unsigned long baud;  
};



//#endif
#endif