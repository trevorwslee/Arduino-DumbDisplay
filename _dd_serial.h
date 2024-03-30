#ifndef _dd_serial_h
#define _dd_serial_h


// #ifdef DD_4_ESP32
// #include <esp_spp_api.h>
// #include "HardwareSerial.h"
// #endif

/// Class for internal use only
class DDSerial {
  public:
    virtual void begin(unsigned long serialBaud) {
      Serial.begin(serialBaud);
    }
    virtual bool available() {
      return Serial.available();
    }
    virtual char read() {
      return Serial.read();
    }
    virtual void print(const String &s) {
      Serial.print(s);
    }
    virtual void print(const char *p) {
      Serial.print(p);
    }
    virtual void write(uint8_t b) {
      Serial.write(b);
    }
    virtual void flush() {
      Serial.flush();
    }
};

//extern DDSerial* _The_DD_Serial;

#endif