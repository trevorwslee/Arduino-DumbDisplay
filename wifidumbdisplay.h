#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

#include "dumbdisplay.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif


#define DD_USING_WIFI


//#define LOG_DDWIFI_STATUS


/// Subclass of DDInputOutput
class DDWiFiServerIO: public DDInputOutput {
  public:
    /* WiFI IO mechanism */
    /* - ssid: WIFI name (pass to WiFi) */
    /* - passphrase: WIFI password (pass to WiFi) */
    /* - serverPort: server port (pass to WiFiServer) */
    /* - logToSerial: log to Serial (default will log toSerial) */
    /* - serialBaud: Serial baud rate, if logToSerial (default is 115200) */
    DDWiFiServerIO(const char* ssid, const char *passphrase, int serverPort = DD_WIFI_PORT,
                   bool logToSerial = true, unsigned long serialBaud = DD_SERIAL_BAUD):
                   DDInputOutput(serialBaud, false, false),
                   server(serverPort) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
      this->logToSerial = logToSerial;
      if (logToSerial) {
        Serial.begin(serialBaud);
      }
    }
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
      client.write(buf, size);
    }
    bool preConnect(bool firstCall) {
      if (firstCall) {
        // if (logToSerial)
        //   Serial.begin(serialBaud);
        if (true) {  // since 2023-06-03
          client.stop();
          WiFi.disconnect();
        }
        WiFi.begin(ssid, password);
        connectionState = ' ';
        stateMillis = 0;
      } else {
        checkConnection();
        delay(200);
      }
      return connectionState == 'C';
    }
    void flush() {
      client.flush();
    }
    void validConnection() {
#ifdef LOG_DDWIFI_STATUS
      if (logToSerial) {
        Serial.println(" ... validate ...");
      }
#endif      
      checkConnection();
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
      uint8_t status = WiFi.status();
      if (connectionState == 'C') {
        if (status != WL_CONNECTED) {
          if (logToSerial) Serial.println("lost WiFi ... try bind WiFi again ...");
          client.stop();
          WiFi.disconnect();
          connectionState = ' ';
          stateMillis = 0;
        } else if (!client.connected()) {
          client.stop();
          if (logToSerial) Serial.println("lost connection ... try bind connect again ...");
          connectionState = 'W';
          stateMillis = 0;
        } else {
          return;
        }
      }
      if (connectionState == ' ') {
        //uint8_t status = WiFi.status();
        if (status == WL_CONNECTED) {
          if (logToSerial) {
            Serial.print("binded WIFI ");
            Serial.println(ssid);
          }
          server.begin();
          connectionState = 'W';
          stateMillis = 0;
        } else {
          long diff = millis() - stateMillis;
          if (diff > 1000) {
            if (logToSerial) {
              Serial.print("binding WIFI ");
              Serial.print(ssid);
#ifdef LOG_DDWIFI_STATUS
              Serial.print(" ... ");
              Serial.print(status);
#endif
              Serial.println(" ...");
            }
            stateMillis = millis();
          }
        }
      } else {
        if (status != WL_CONNECTED) {
          connectionState = ' ';
          stateMillis = 0;
        } else {
          long diff = millis() - stateMillis;
          if (diff >= 1000) {
            IPAddress localIP = WiFi.localIP();
            uint32_t localIPValue = localIP;
            if (logToSerial) {
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
            }
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
            if (logToSerial) Serial.println("client conntected");
          }
        }  
      }
    }
  private:
    const char* ssid;
    const char *password;
    int port;
    bool logToSerial;
   char connectionState;
   long stateMillis;
    WiFiServer server;
    WiFiClient client;
};




#endif
