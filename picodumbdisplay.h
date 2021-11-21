#ifndef esp32dumbdisplay_h
#define esp32dumbdisplay_h

#if !(defined DD_4_PICO_TX && defined DD_4_PICO_RX)

#error DD_4_PICO_TX and DD_4_PICO_RX need be defined in order to use DumbDisplay for ESP32
#error e.g. #define DD_4_PICO_TX 8
#error e.g. #define DD_4_PICO_RX 9

#else

#include "dumbdisplay.h"

//UART Serial2(8, 9, 0, 0);
UART Serial2(DD_4_PICO_TX, DD_4_PICO_RX, 0, 0);


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
    bool preConnect(bool firstCall) {
      Serial2.begin(baud);
      return true;
    }
    void flush() {
    }
  private:
    unsigned long baud;  
};



#endif
#endif