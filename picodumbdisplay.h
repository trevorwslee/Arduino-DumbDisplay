//************************************************//
//*** must define DD_4_ESP32 before includeing ***/
//************************************************//

#ifndef esp32dumbdisplay_h
#define esp32dumbdisplay_h

#ifndef DD_4_PICO
#error DD_4_PICO need be defined in order to use DumbDisplay for ESP32
#endif

#ifdef DD_4_PICO

#include "dumbdisplay.h"

UART Serial2(8, 9, 0, 0);


class DDPicoUart1IO: public DDInputOutput {
  public:
    /* using PICO Uart1 */
    DDPicoUart1IO(unsigned long baud): DDInputOutput(DD_SERIAL_BAUD, false, false) {
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
    void preConnect() {
      Serial2.begin(baud);
    }
    void flush() {
    }
  private:
    unsigned long baud;  
};



#endif
#endif