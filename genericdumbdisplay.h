#ifndef genericdumbdisplay_h
#define genericdumbdisplay_h

#if !defined(DD_SERIAL)
  #error Before #include, must define the macro DD_SERIAL and optional DD_SERIAL_begin (a function call or a code block) \
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
/// <br>IMPORTANT:
/// - Before `#include`, must define `DD_SERIAL`
/// - and optionally define `DD_SERIAL_begin` if necessary
/// - `DD_SERIAL_begin` is a function call or a code block
/// - if `DD_SERIAL_begin` not defined, will call `DD_SERIAL.begin(115200)` instead
/// - e.g. SoftwareSerial -- 2 => TX; 3 => RX
///   <br>`#include <SoftwareSerial.h>`
///   <br>`SoftwareSerial ss(2, 3);`
///   <br>`#define DD_SERIAL ss`
/// - e.g. Pico -- 8: PICO_TX; 9: PICO_RX
///   <br>`UART uart(8, 9, 0, 0);`
///   <br>`#define DD_SERIAL uart`
/// - e.g. Arduino Mega -- 17 ==> TX; 16 ==> RX
///   <br>`#define DD_SERIAL Serial2`
/// - e.g. STM32F103 --  PA3 (RX2) ==> TX; PA2 (TX2) ==> RX
///   <br>`HardwareSerial hs(USART2);`
///   <br>`#define DD_SERIAL hs`
class DDGenericIO: public DDInputOutput {
  public:
    DDGenericIO(bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, enableSerial, enableSerial) {
    }
    const char* getWhat() {
      return "GENERIC";
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
      DDInputOutput::preConnect(firstCall);
      if (firstCall) {
#if defined(DD_SERIAL_begin)        
        DD_SERIAL_begin;
#else
        DD_SERIAL.begin(115200);
#endif  
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
