
#ifndef atwifidumbdisplay_h
#define atwifidumbdisplay_h

#if !defined(ESP_SERIAL)
  #error Must define the macro ESP_SERIAL and optionally ESP_SERIAL_begin (a function call or a code block) \
    *** \
    e.g. STM32F103: PA3 (RX2) ==> TX; PA2 (TX2) ==> RX \
    #define DD_SERIAL Serial2 \
    *** \
    e.g. Pico \
    UART Serial2(8, 9, 0, 0);  // 8: PICO_TX; 9: PICO_RX \
    #define DD_SERIAL Serial2
#endif

#include "Arduino.h"
#include "dumbdisplay.h"

//#define DEGUG_SHOW_STATE

//#define DEBUG_ESP_AT
//#define FORCE_DEBUG_NO_SILENT
#include "_loespat.h"


// after inclusion, can check DD_USING_WIFI to be sure WIFI is used
#define DD_USING_WIFI


/// Subclass of DDInputOutput
class DDATWiFiIO: public DDInputOutput {
  public:
    /* ESP chipset as WIFI with AT commands IO mechanism */
    /* - ssid: WIFI name */
    /* - passphrase: WIFI password */
    /* - serverPort: server port */
    DDATWiFiIO(const char* ssid, const char *passphrase, int serverPort = DD_WIFI_PORT): DDInputOutput(DD_SERIAL_BAUD, false, false) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
      this->ip = "";
      this->linkId = -1;
      this->data = "";
      this->dataIdx = 0;
      this->lastValidateMs = 0;
      //this->connectionState = '0';
    }
    const char* getWhat() {
      return "ATWIFI";
    }
    bool available() {
      return atCheckAvailable();
    }
    char read() {
      return atRead();
    } 
    void print(const String &s) {
      atPrint(s);
    }
    void print(const char *p) {
      atPrint(p);
    }
    void write(uint8_t b) {
      atWrite(b);
    }
    void write(const uint8_t *buf, size_t size) {
      atWrite(buf, size);
    }
    bool preConnect(bool firstCall) {
      if (firstCall) {
        if (!Serial) Serial.begin(DD_SERIAL_BAUD);
#if defined(ESP_SERIAL_begin)        
        ESP_SERIAL_begin;
#else
        ESP_SERIAL.begin(115200);
#endif  
        LOEspAt::InitAt();      
      }
      return atPreConnect(firstCall);
    }
    void flush() {
      atFlush();
    }
    void validConnection() {
// #ifdef LOG_DDWIFI_STATUS
//       Serial.println(" ... validate ...");
// #endif      
      atCheckConnection(false);
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return true;
    }
  private:
    inline bool atCheckAvailable() {
      if (dataIdx < data.length()) {
        return true;
      }
      if (linkId != -1) {
        if (LOEspAt::ReceiveDataFromClient(linkId, data)) {
          dataIdx = 0;
          if (dataIdx < data.length()) {
            return true;
          }
        }
      }
      return false;
    }
    inline char atRead() {
      if (dataIdx < data.length()) {
        return data.charAt(dataIdx++);
      }
      return 0;
    } 
    inline void atPrint(const String &s) {
      if (linkId != -1) {
        LOEspAt::SendDataToClient(linkId, s);
      }
    }
    inline void atPrint(const char *p) {
      if (linkId != -1) {
        LOEspAt::SendDataToClient(linkId, p);
      }
    }
    inline void atPrint(uint8_t b) {
      if (linkId != -1) {
        LOEspAt::SendDataToClient(linkId, b);
      }
    }
    inline void atWrite(uint8_t b) {
      if (linkId != -1) {
        LOEspAt::SendDataToClient(linkId, b);
      }
    }
    inline void atWrite(const uint8_t *buf, size_t size) {
      if (linkId != -1) {
        LOEspAt::SendDataToClient(linkId, buf, size);
      }
    }
    inline void atFlush() {
    }
    bool atPreConnect(bool firstCall) {
      if (true) {
        if ((firstCall || connectionState == '0') && linkId == -1) {
          connectionState = '0';
          if (!atSetupWIFI()) {
            Serial.println("failed to initially setup WIFI");
            return false;
          }
          connectionState = ' ';
        }
        atCheckConnection(true);
        return connectionState == 'C';
      } else {
        if ((firstCall || connectionState == '0') && linkId == -1) {
          connectionState = '0';
          Serial.println("setup AT WIFI");
          LOEspAt::DisconnectAP();
          bool failed = false;
          if (!LOEspAt::CheckAt()) {
            Serial.println("XXX AT not ready");
            failed = true;
          }
          if (!failed) {
            if (!LOEspAt::SetStationMode()) {
              Serial.println("XXX failed to set 'station mode'");
              failed = true;
            }
          }
          if (!failed) {
            delay(1000); // delay a bit
            if (!LOEspAt::ConnectAP(ssid, password, ip)) {
              Serial.println("XXX failed to start AP???");
              failed = true;
            }
          if (!failed) {}
            if (!LOEspAt::StartServer(port)) {
              Serial.println("XXX failed to start server");
              LOEspAt::DisconnectAP();
              failed = true;
            }
          }
          if (failed) {
            Serial.println("failed to setup AT WIFI");
            atReset();
            return false;
          }
          Serial.println("DONE setup AT WIFI");
          //WiFi.begin(ssid, password);
          connectionState = ' ';
          //stateMillis = 0;
        }
        //  delay(200);
        atCheckConnection(true);
        return connectionState == 'C';
      }
    }
    void atReset() {
      Serial.println("reseting ...");
      LOEspAt::Reset();
      delay(2000);
      Serial.println("... done reset");

    }
    void atCheckConnection(bool forPreConnect) {
      if (true) {  // don't validate so frequently
        // don't validate so frequently
        long now = millis();
        if ((now - lastValidateMs) < (forPreConnect ? 500 : 2000)) {
          return;
        }
        lastValidateMs = now;
      }
      int state = LOEspAt::CheckState();
#if defined(DEGUG_SHOW_STATE)
      Serial.print("[");
      Serial.print(connectionState);
      Serial.print("]: ");
      Serial.println(state);
#endif
      if (state == -1) {
        Serial.println("failed to check AT WIFI state");
        atReset();
        //LOEspAt::DisconnectAP();
        connectionState = ' ';
        return;
      }
      // if (state == 0) {
      //   // 0: ESP32 station has not started any Wi-Fi connection.
      //   if (!LOEspAt::Reset() || !LOEspAt::ConnectAP(ssid, password, ip)) {
      //     Serial.println("XXX failed to start AP during connection check");
      //   }
      //   return;
      // }
      if (state == 1 || state == 3) {
        // 1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
        // 3: ESP32 station is in Wi-Fi connecting or reconnecting state.
        return;
      }
      if (connectionState == 'C') {
        if (state != 2) {
          // 2: ESP32 station has connected to an AP, and got an IPv4 address.
          Serial.println("lost AT WIFI ... try bind AT WIFI again ...");
          Serial.println(state);
          if (true) {
            if (!atSetupWIFI()) {
              Serial.println("failed to setup WIFI again");
              atReset();
              return;
            }
          } else {
            // client.stop();
            // WiFi.disconnect();
            atDisconnectClient();
            LOEspAt::DisconnectAP();
          }
          connectionState = ' ';
          //stateMillis = 0;
        } else if (linkId != -1) {
          //client.stop();
          //atDisconnectClient();
          int checkLinkId = LOEspAt::CheckForClientConnection();
          if (checkLinkId == -1) {
            linkId = -1;
            Serial.println("lost connection ... try connect again ...");
            connectionState = 'W';
            //stateMillis = 0;
          } else {
            return;
          }
        } else {
          return;
        }
      }
      if (connectionState == ' ') {
        //uint8_t status = WiFi.status();
        if (state == 2) {
          Serial.print("binded AT WIFI ");
          Serial.println(ssid);
          //server.begin();
          connectionState = 'W';
          //stateMillis = 0;
        } else {
          //long diff = millis() - stateMillis;
          //if (stateMillis == 0 || diff > 1000) {
            //LOEspAt::StartServer(port);
            Serial.print("binding AT WIFI ");
            Serial.print(ssid);
// #ifdef LOG_DDWIFI_STATUS
//             Serial.print(" ... ");
//             Serial.print(state);
// #endif
            Serial.println(" ...");
            //stateMillis = millis();
          //}
        }
      } else {
        if (state != 2) {
          connectionState = ' ';
          //stateMillis = 0;
        } else {
          //long diff = millis() - stateMillis;
          //if (diff >= 1000) {
            //IPAddress localIP = WiFi.localIP();
            //uint32_t localIPValue = localIP;
// #ifdef LOG_DDWIFI_STATUS
//             Serial.print("via WIFI ... ");
//             Serial.print(LOEspAt::CheckState()/*WiFi.status()*/);
//             Serial.print(" ... ");
// #endif
            if (true) {
              int clientState = LOEspAt::CheckServerState();
              //Serial.println(clientState);
              if (clientState != 1) {
                Serial.println("server not running");
                atReset();
                connectionState == ' ';
                 return;
              }
            }
            Serial.print("listening on ");
            Serial.print(ip);
            Serial.print(":");
            Serial.print(port);
            Serial.println(" ...");
            //stateMillis = millis();
          //}
          if (true) {
            int checkLinkId = LOEspAt::CheckForClientConnection();
            if (checkLinkId != -1) {
              linkId = checkLinkId;
              connectionState = 'C';
              //stateMillis = 0;
              Serial.println("client connected");
            } 
          } else {
            if (linkId == -1) {
              linkId = LOEspAt::CheckForClientConnection();
              if (linkId != -1) {
                connectionState = 'C';
                //stateMillis = 0;
                Serial.println("client connected");
              }
            }
          }
        }  
      }
    }
    bool atSetupWIFI() {
      Serial.println("setup AT WIFI");
      LOEspAt::DisconnectAP();
      bool failed = false;
      if (!LOEspAt::CheckAt()) {
        Serial.println("XXX AT not ready");
        failed = true;
      }
      if (!failed) {
        if (!LOEspAt::SetStationMode()) {
          Serial.println("XXX failed to set 'station mode'");
          failed = true;
        }
      }
      if (!failed) {
        delay(1000); // delay a bit
        if (!LOEspAt::ConnectAP(ssid, password, ip)) {
          Serial.println("XXX failed to start AP when");
          failed = true;
        }
      if (!failed) {}
        if (!LOEspAt::StartServer(port)) {
          Serial.println("XXX failed to start server");
          LOEspAt::DisconnectAP();
          failed = true;
        }
      }
      if (failed) {
        Serial.println("failed to setup AT WIFI");
        atReset();
        return false;
      }
      Serial.println("DONE setup AT WIFI");
      return true;
    }
    void atDisconnectClient() {
      if (linkId != -1) {
          LOEspAt::DisconnectClient(linkId);
          linkId = -1;
      }
    }
  private:
    const char* ssid;
    const char* password;
    int port;
    char connectionState;
    //long stateMillis;
    String ip;
    int linkId;
    String data;
    int dataIdx;
    long lastValidateMs;
};



#endif