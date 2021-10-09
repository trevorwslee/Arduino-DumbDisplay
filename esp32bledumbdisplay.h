//************************************************//
//*** must define DD_4_ESP32 before includeing ***/
//************************************************//

#ifndef esp32bledumbdisplay_h
#define esp32bledumbdisplay_h

#ifndef DD_4_ESP32
#error DD_4_ESP32 need be defined in order to use DumbDisplay for ESP32 BLE
#else

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "dumbdisplay.h"

#define DD_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define DD_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DD_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define DD_RECEIVE_BUFFER_SIZE 64
#define DD_LE_INDICATE

//#define DD_DEBUG_BLE 


class DDBLESerialIO: public DDInputOutput {
  public:
    /* ESP32 BLE IO mechanism */
    /* - deviceName -- BLE device name */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    /* - serialBaud: Serial baud rate (if enableSerial) */
    DDBLESerialIO(const String& deviceName, bool enableSerial = false,
                  unsigned long serialBaud = DD_SERIAL_BAUD):
                  DDInputOutput(serialBaud, enableSerial, enableSerial) {
      this->deviceName = deviceName;
    }
    bool available() {
      return pCallbacks->available();
    }
    char read() {
      return pCallbacks->read();
    } 
    void print(const String &s) {
      pCallbacks->print(s.c_str());
    }
    void print(const char *p) {
      pCallbacks->print(p);
    }
    void preConnect() {
#ifdef DD_DEBUG_BLE
      if (!setupForSerial) {
        Serial.begin(115200);
      }
#endif
      DDInputOutput::preConnect();
      initBLE();
    }
    void flush() {
    }
  private:
    class ServerCallbacks: public BLEServerCallbacks {
      public:
        void onConnect(BLEServer* pServer) {
          connected = true;
#ifdef DD_DEBUG_BLE
      Serial.println("BLE connected");
#endif      
        };
        void onDisconnect(BLEServer* pServer) {
          connected = false;
#ifdef DD_DEBUG_BLE
      Serial.println("BLE disconnected");
#endif      
        }
      public:
        bool connected;
    };
    class Callbacks: public BLECharacteristicCallbacks {
      public:
        Callbacks(BLECharacteristic *pTx, ServerCallbacks* pServerCallbacks) {
          this->pTx = pTx;
          this->pServerCallbacks = pServerCallbacks;
          this->buffering = false;
          this->bufferSize = 0;
        }
      public:
        void print(std::string s) {
          pTx->setValue(s);
#ifdef DD_LE_INDICATE          
          pTx->indicate();
#else          
          pTx->notify();
#endif
#ifdef DD_DEBUG_BLE
          if (!pServerCallbacks->connected) {
          Serial.print("[not connected] ");
          }
          Serial.print("BLE sent ... ");
          Serial.print(" ... [");
          Serial.print(s.c_str());
          Serial.println("]");
#endif
        }
        bool available() {
          while (true) {
            if (!buffering) {
              return bufferSize > 0;
            }
            yield();
          }
        }
        char read() {
          while (true) {
            if (!buffering) {
              char c = buffer[--bufferSize];
// #ifdef DD_DEBUG_BLE
//           Serial.print("<");
//           Serial.print(c);
//           Serial.print("<");
// #endif
              return c;
            }
            yield();
          }
        } 
      public:
        void onWrite(BLECharacteristic *pCharacteristic) {
          buffering = true;
          std::string rxValue = pCharacteristic->getValue();
          int count = rxValue.size();
          for (int i = count - 1; i >= 0; i--) {
            if (bufferSize > DD_RECEIVE_BUFFER_SIZE) {
              break;
            }
            buffer[bufferSize++] = rxValue[i];
          }
#ifdef DD_DEBUG_BLE
          Serial.print("BLE received ... ");
          Serial.print(count);
          Serial.print(" ... [");
          Serial.print(rxValue.c_str());
          Serial.print("] ==> ");
          Serial.print(bufferSize);
          Serial.print("/");
          Serial.print(DD_RECEIVE_BUFFER_SIZE);
          Serial.println();
#endif
          buffering = false;
        }
      private:   
        BLECharacteristic *pTx;
        ServerCallbacks* pServerCallbacks;
        volatile bool buffering;
        uint8_t bufferSize;
        char buffer[DD_RECEIVE_BUFFER_SIZE];
    };
  private:
    void initBLE() {  
#ifdef DD_DEBUG_BLE
      Serial.println("initialized BLE ...");
#endif      
      BLEDevice::init(deviceName.c_str());
      BLEServer *pServer = BLEDevice::createServer();
      pServerCallbacks = new ServerCallbacks();
      pServer->setCallbacks(pServerCallbacks);
      BLEService *pService = pServer->createService(DD_SERVICE_UUID);
      BLECharacteristic *pTx = pService->createCharacteristic(
										DD_CHARACTERISTIC_UUID_TX,
#ifdef DD_LE_INDICATE          
                    BLECharacteristic::PROPERTY_INDICATE
#else                    
										BLECharacteristic::PROPERTY_NOTIFY
#endif                    
									);
      pTx->addDescriptor(new BLE2902());
      BLECharacteristic *pRx = pService->createCharacteristic(
											 DD_CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);
      pCallbacks = new Callbacks(pTx, pServerCallbacks);              
      pRx->setCallbacks(pCallbacks);
      pService->start();
      pServer->getAdvertising()->start();
#ifdef DD_DEBUG_BLE
      Serial.println("... done initialized BLE");
#endif      
    }
  private:
    String deviceName;
    ServerCallbacks* pServerCallbacks;
    Callbacks* pCallbacks;
};


#endif
#endif