//************************************************//
//*** must define DD_4_ESP32 before includeing ***/
//************************************************//

#ifndef esp32bledumbdisplay_h
#define esp32bledumbdisplay_h

// #ifndef DD_4_ESP32
// #error DD_4_ESP32 need be defined in order to use DumbDisplay for ESP32 BLE
// #else

#ifndef ESP32
#error DDBLESerialIO is for ESP32
#else

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "dumbdisplay.h"

#define DD_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define DD_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DD_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define DD_RECEIVE_BUFFER_SIZE 256
#define DD_SEND_BUFFER_SIZE 20

// using INDICATE is more reliable, but slower
#define DD_LE_INDICATE

//#define DD_DEBUG_BLE 
//#define DD_DEBUG_BLE_NOT_SEND



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
    void write(uint8_t b) {
      pCallbacks->write(b); 
    }
    bool preConnect(bool firstCall) {
#ifdef DD_DEBUG_BLE
      if (!setupForSerial) {
        Serial.begin(115200);
      }
#endif
      DDInputOutput::preConnect(firstCall);
      initBLE();
      return true;
    }
    void flush() {
    }
    void validConnection() {
      if (pServerCallbacks->isDisconnected()) {
          delay(500); // give the bluetooth stack the chance to get things ready
          pServerCallbacks->restartAdvertising();
#ifdef DD_DEBUG_BLE
          Serial.println("... connection lost ... waiting for re-connection ...");
#endif      
      }
    }  
    bool canUseBuffer() {
      return false;
    }
  private:
    class ServerCallbacks: public BLEServerCallbacks {
      public:
        ServerCallbacks(BLEServer* pServer) {
          this->pServer = pServer;
          this->connectionState = 0;  // assume connecting
        }
      public:
        void restartAdvertising() {
          pServer->startAdvertising();
          connectionState = 0;
        }
        bool isDisconnected() { return connectionState == -1; }
        bool isConnected() { return connectionState == 1; }  
        bool isConnecting() { return connectionState == 0; }
      public:
        void onConnect(BLEServer* pServer) {
          connectionState = 1;
#ifdef DD_DEBUG_BLE
          Serial.println("BLE connected");
#endif      
        };
        void onDisconnect(BLEServer* pServer) {
          connectionState = -1;
#ifdef DD_DEBUG_BLE
          Serial.println("BLE disconnected");
#endif      
        }
      private:
        BLEServer* pServer;
        int connectionState;
    };
    class Callbacks: public BLECharacteristicCallbacks {
      public:
        Callbacks(BLECharacteristic *pTx, ServerCallbacks* pServerCallbacks) {
          this->pTx = pTx;
          this->pServerCallbacks = pServerCallbacks;
          this->buffering = false;
          this->bufferStart = 0;
          this->bufferEnd = 0;
#ifdef DD_SEND_BUFFER_SIZE
          this->sendBufferSize = 0;
#endif
        }
      public:
        void print(std::string s) {
#ifdef DD_SEND_BUFFER_SIZE
          int len = s.length();
          if ((len + sendBufferSize) > DD_SEND_BUFFER_SIZE) {
            _flushSendBuffer();
            if (len > DD_SEND_BUFFER_SIZE) {
              _print(s.c_str());
              return;
            }
          }
          for (int i = 0; i < len; i++) {
            sendBuffer[sendBufferSize++] = s[i];
          }
          if (sendBufferSize > 0 && sendBuffer[sendBufferSize - 1] == '\n') {
            _flushSendBuffer();
          }
#else
          _print(s.c_str());
#endif
        }  
        void write(uint8_t b) {
          int s = b;
          pTx->setValue(s);
#ifdef DD_LE_INDICATE          
          pTx->indicate();
#else          
          pTx->notify();
#endif
        }
        bool available() {
          while (true) {
            if (!buffering) {
              return bufferStart != bufferEnd;
            }
            yield();
          }
        }
        char read() {
          while (true) {
            if (!buffering) {
              char c = buffer[bufferStart];
              if (++bufferStart >= DD_RECEIVE_BUFFER_SIZE) {
                bufferStart = 0;
              }
// #ifdef DD_DEBUG_BLE
//               Serial.print('<');
//               Serial.print(c);
//               Serial.print('>');
// #endif
              return c;
            }
            yield();
          }
        }
      private:   
#ifdef DD_SEND_BUFFER_SIZE
        void _flushSendBuffer() {
          if (sendBufferSize > 0) {
            sendBuffer[sendBufferSize] = 0;
            _print(sendBuffer);
            sendBufferSize = 0;
          }
        }
#endif
        void _print(std::string s) {
          int len = s.length();
          while (len > 0) {
            if (len <= 20) {  // 20 is the BLE limit
              pTx->setValue(s);
#ifdef DD_LE_INDICATE          
              pTx->indicate();
#else          
              pTx->notify();
#endif
              break;
            } else {
              pTx->setValue(s.substr(0, 20));
#ifdef DD_LE_INDICATE          
              pTx->indicate();
#else          
              pTx->notify();
#endif
              s = s.substr(20);
              len -= 20;
            }
          }
#if defined(DD_DEBUG_BLE) && !defined(DD_DEBUG_BLE_NOT_SEND)
          if (pServerCallbacks->isConnected()) {
          } else if (pServerCallbacks->isConnecting()) {
            Serial.print("[connecting] ");
          } else {
            Serial.print("[not connected] ");
          }
          Serial.print("BLE sent ... ");
          Serial.print(" ... [");
          Serial.print(s.c_str());
          Serial.println("]");
#endif
        }
      public:
        void onWrite(BLECharacteristic *pCharacteristic) {
          buffering = true;
          std::string rxValue = pCharacteristic->getValue();
          int count = rxValue.size();
          for (int i = 0; i < count; i++) {
            int nextBufferEnd = bufferEnd + 1;
            if (nextBufferEnd >= DD_RECEIVE_BUFFER_SIZE) {
              nextBufferEnd = 0;
            }
            if (nextBufferEnd == bufferStart) {
              break;
            }
            buffer[bufferEnd] = rxValue[i];
            bufferEnd = nextBufferEnd;
          }
#ifdef DD_DEBUG_BLE
          Serial.print("BLE received ... ");
          Serial.print(count);
          Serial.print(" ... [");
          Serial.print(rxValue.c_str());
          Serial.print("] ==> ");
          Serial.print(bufferStart);
          Serial.print('-');
          Serial.print(bufferEnd);
          Serial.print('/');
          Serial.print(DD_RECEIVE_BUFFER_SIZE);
          Serial.println();
#endif
          buffering = false;
        }
      private:   
        BLECharacteristic *pTx;
        ServerCallbacks* pServerCallbacks;
        volatile bool buffering;
        uint8_t bufferStart;
        uint8_t bufferEnd;
        char buffer[DD_RECEIVE_BUFFER_SIZE];
#ifdef DD_SEND_BUFFER_SIZE
        uint8_t sendBufferSize;
        char sendBuffer[DD_SEND_BUFFER_SIZE + 1];
#endif    
    };
  private:
    void initBLE() {  
#ifdef DD_DEBUG_BLE
      Serial.println("initialized BLE ...");
#endif      
      BLEDevice::init(deviceName.c_str());
      BLEServer *pServer = BLEDevice::createServer();
      pServerCallbacks = new ServerCallbacks(pServer);
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
      Serial.println("... done initialized BLE ... waiting for connection ...");
#endif      
    }
  private:
    String deviceName;
    ServerCallbacks* pServerCallbacks;
    Callbacks* pCallbacks;
};


#endif
#endif