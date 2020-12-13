//************************************************//
//*** must define DD_4_ESP32 before includeing ***/
//************************************************//

#ifndef esp32dumbdisplay_h
#define esp32dumbdisplay_h

#ifndef DD_4_ESP32
#error DD_4_ESP32 need be defined in order to use DumbDisplay for ESP32
#endif

#ifdef DD_4_ESP32

#include "BluetoothSerial.h"
#include "dumbdisplay.h"


class DDBluetoothSerialIO: public DDInputOutput {
  public:
    /* ESP32 BluetoothSerial IO mechanism */
    /* - btLocalName -- BT name */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    DDBluetoothSerialIO(const String& btLocalName, bool enableSerial = false): DDInputOutput(enableSerial, enableSerial) {
      this->btLocalName = btLocalName;
    }
    bool available() {
      return serialBT.available();
    }
    char read() {
      return serialBT.read();
    } 
    void print(const char *p) {
      serialBT.print(p); 
    }
    void preConnect() {
      DDInputOutput::preConnect();
      serialBT.begin(btLocalName);
    }
    void flush() {
      serialBT.flush();
    }
  private:
    String btLocalName;
    BluetoothSerial serialBT;  
};



#endif
#endif