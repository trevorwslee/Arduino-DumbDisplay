#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

// after inclusion, can check DD_USING_WIFI to be sure WIFI is used
#define DD_USING_WIFI

#include "dumbdisplay.h"

#if defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif


#ifndef DD_SHOW_IP_INTERVAL_SECONDS
  #define DD_SHOW_IP_INTERVAL_SECONDS 2
#endif  


//#define LOG_DDWIFI_STATUS


/// Subclass of DDInputOutput
class DDWiFiServerIO: public DDInputOutput {
  public:
    /* WiFI IO mechanism */
    /* - ssid: WIFI SSID / name; NULL if WiFi explicitly connected */
    /* - passphrase: WIFI password; can be NULL if Wifi explicitly connected */
    /* - serverPort: server port */
    DDWiFiServerIO(const char* ssid = NULL, const char *passphrase = NULL, int serverPort = DD_WIFI_PORT):
                   DDInputOutput(DD_SERIAL_BAUD, false, false), server(serverPort) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
      //this->logToSerial = logToSerial;
      //Serial.begin(DD_SERIAL_BAUD);
    }
    const char* getWhat() {
      return "WIFI";
    }
    // // experimental ... not quite working
    // bool availableAdditionalClient(WiFiClient& additionalClient) {
    //   if (client.connected()) {
    //       WiFiClient cli = server.available();
    //       if (cli) {
    //         additionalClient = cli;
    //         return true;
    //       }
    //   }
    //   return false;
    // }
    bool available() {
      return client.available() > 0;
    }
    char read() {
      return client.read();
    } 
    void print(const String &s) {
      client.print(s);
    }
    void print(const char *p) {
      client.print(p);
    }
    void write(uint8_t b) {
      client.write(b);
    }
    void write(const uint8_t *buf, size_t size) {
      if (false) {
        Serial.print("*** write ");
        Serial.print(size);
        Serial.print(" ... ");
        size_t written = client.write(buf, size);
        Serial.println(written);
      } else {
        client.write(buf, size);
      }
    }
    bool preConnect(bool firstCall) {
      if (firstCall) {  // since 2023-08-10
        if (!Serial) Serial.begin(DD_SERIAL_BAUD);
#if defined(ARDUINO_UNOR4_WIFI)
        if (WiFi.status() == WL_NO_MODULE) {
          Serial.println("XXX communication with WiFi module failed");
        }
        String fv = WiFi.firmwareVersion();
        if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
          Serial.println("XXX please upgrade the firmware");
        } else {
          Serial.println("* WIFI_FIRMWARE_LATEST_VERSION=" + String(WIFI_FIRMWARE_LATEST_VERSION));
        }
#endif
      }
      if (firstCall && !client.connected()) {
        connectionState = '0';
      }
      checkConnection();
      return connectionState == 'C';
    }
    void flush() {
      client.flush();
    }
    void validConnection() {
#ifdef LOG_DDWIFI_STATUS
      Serial.println(" ... validate ...");
#endif      
      checkConnection();
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return true;
    }
  private:
//     bool _connectToNetwork() {
//       WiFi.begin(ssid, password);
//       long last = millis();
//       while (true) {
//         uint8_t status = WiFi.status();
//         if (status == WL_CONNECTED)
//           break;
//         delay(200);
//         long diff = millis() - last;
//         if (diff > 1000) {
//           if (logToSerial) {
//             Serial.print("binding WIFI ");
//             Serial.print(ssid);
// #ifdef LOG_DDWIFI_STATUS
//             Serial.print(" ... ");
//             Serial.print(status);
// #endif
//             Serial.println(" ...");
//           }
//           last = millis();
//         }
//       }
//       if (logToSerial) {
//         Serial.print("binded WIFI ");
//         Serial.println(ssid);
//       }
//       return true;
//     }
//     bool _connectToClient(bool initConnect) {
//       if (initConnect) {
//         if (!_connectToNetwork())
//           return false;
//         server.begin();
//       }
//       long last = millis();
//       while (true) {
//         client = server.available();
//         if (client) {
//           break;
//         } else {
//           if (WiFi.status() != WL_CONNECTED) 
//             return false;
//           long diff = millis() - last;
//           if (diff >= 1000) {
//             IPAddress localIP = WiFi.localIP();
//             uint32_t localIPValue = localIP;
//             if (logToSerial) {
// #ifdef LOG_DDWIFI_STATUS
//               Serial.print("via WIFI ... ");
//               Serial.print(WiFi.status());
//               Serial.print(" ... ");
// #endif
//               Serial.print("listening on ");
// // #ifdef LOG_DDWIFI_STATUS
// //               Serial.print("(");
// //               Serial.print(localIPValue);
// //               Serial.print(") ");
// // #endif
//               Serial.print(localIP);
//               Serial.print(":");
//               Serial.print(port);
//               Serial.println(" ...");
//             }
//             last = millis();
//             if (localIPValue == 0)
//               return false;
//           }
//         }
//       }
//       return true;
//     }
    void checkConnection() {
      if (connectionState == '0' || connectionState == 'C') {  // since 2023-08-16 added check connectionState == '0'
        if (connectionState == '0' || WiFi.status() != WL_CONNECTED) {
          if (connectionState == 'C') {
            Serial.println("lost WiFi ... try bind WiFi again ...");
            client.stop();
          }
          if (ssid != NULL && WiFi.status() != WL_CONNECTED) {  // since 2025-03-10
            WiFi.disconnect();
            Serial.println("setup WIFI");
            WiFi.begin(ssid, password);
          } else {
            Serial.println("assume explicitly connected WIFI");
          }
          connectionState = ' ';
          stateMillis = 0;
        } else if (!client.connected()) {
          client.stop();
          Serial.println("lost connection ... try bind again ...");
          connectionState = 'W';
          stateMillis = 0;
        } else {
          return;
        }
      }
      if (connectionState == ' ') {
        uint8_t status = WiFi.status();
        if (status == WL_CONNECTED) {
          Serial.print("binded WIFI ");
          Serial.println(ssid);
          server.begin();
          connectionState = 'W';
          stateMillis = 0;
        } else {
          long diff = millis() - stateMillis;
          if (stateMillis == 0 || diff > 1000) {
            Serial.print("binding WIFI ");
            Serial.print(ssid);
#ifdef LOG_DDWIFI_STATUS
            Serial.print(" ... ");
            Serial.print(status);
#endif
            Serial.println(" ...");
            stateMillis = millis();
          }
        }
      } else {
        uint8_t status = WiFi.status();
        if (status != WL_CONNECTED) {
          connectionState = ' ';
          stateMillis = 0;
        } else {
          long diff = millis() - stateMillis;
          if (stateMillis == 0 || (DD_SHOW_IP_INTERVAL_SECONDS > 0 && diff >= (1000 * DD_SHOW_IP_INTERVAL_SECONDS))) {
            IPAddress localIP = WiFi.localIP();
            //uint32_t localIPValue = localIP;
#ifdef LOG_DDWIFI_STATUS
            Serial.print("via WIFI ... ");
            Serial.print(WiFi.status());
            Serial.print(" ... ");
#endif
            Serial.print("listening on ");
            Serial.print(localIP);
            Serial.print(":");
            Serial.print(port);
            Serial.println(" ...");
            // if (true) {
            //   // testing for ESP
            //   Serial.println("*** sleep ...");
            //   esp_sleep_enable_timer_wakeup(10 * 1000);  // 10ms
            //   esp_light_sleep_start();
            //   Serial.println("*** ... up");
            // }
            stateMillis = millis();
          }
          WiFiClient cli = server.available();
          if (cli) {
            client = cli;
            connectionState = 'C';
            stateMillis = 0;
            Serial.println("client connected");
          } else {
            //Serial.print("~");
          }
        }  
      }
    }
  private:
    const char* ssid;
    const char *password;
    int port;
    //bool logToSerial;
    char connectionState;
    long stateMillis;
    WiFiServer server;
    WiFiClient client;
};




#endif
