//************************************************//
//*** must define DD_4_ESP32 before including ***/
//************************************************//

#ifndef esp32dumbdisplay_h
#define esp32dumbdisplay_h

// after inclusion, can check DD_USING_WIFI to be sure WIFI is used
#define DD_USING_BT

// #ifndef DD_4_ESP32
// #error DD_4_ESP32 need be defined in order to use DumbDisplay for ESP32
// #else

#ifndef ESP32
  #error DDBluetoothSerialIO is for ESP32
#endif

//#else


#include "BluetoothSerial.h"
#include "dumbdisplay.h"


/// Subclass of DDInputOutput
class DDBluetoothSerialIO: public DDInputOutput {
  public:
    /* ESP32 BluetoothSerial IO mechanism */
    /* - btLocalName -- BT name */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    /* - serialBaud: Serial baud rate (if enableSerial) */
    DDBluetoothSerialIO(const String& btLocalName, bool enableSerial = false,
                        unsigned long serialBaud = DD_SERIAL_BAUD):
                        DDInputOutput(serialBaud, enableSerial, enableSerial) {
      this->btLocalName = btLocalName;
      // if (!enableSerial) {  // disabled since 2023-08-13
      //   Serial.begin(serialBaud);
      // }
    }
    const char* getWhat() {
      return "ESP32BT";
    }
    bool available() {
      return serialBT.available();
    }
    char read() {
      return serialBT.read();
    } 
    void print(const String &s) {
      serialBT.print(s); 
    }
    void print(const char *p) {
      serialBT.print(p); 
    }
    void write(uint8_t b) {
      serialBT.write(b); 
    }
    void write(const uint8_t *buf, size_t size) {
      serialBT.write(buf, size); 
    }
    bool preConnect(bool firstCall) {
      DDInputOutput::preConnect(firstCall);
      serialBT.begin(btLocalName);
      if (!willUseSerial()) {  // since 2024-08-13
        if (firstCall) {
          if (!Serial) Serial.begin(DD_SERIAL_BAUD);
        }
        String address = serialBT.getBtAddressString();
        address.toUpperCase();
        Serial.println("bluetooth address: " + address);
      }
      return true;
    }
    void flush() {
      if (false) {
        serialBT.flush();  // not the expected "flush"
      }
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return true;
    }
  private:
    String btLocalName;
    BluetoothSerial serialBT;  
};


//#endif
#endif