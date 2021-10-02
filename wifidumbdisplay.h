#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

#include "dumbdisplay.h"

#ifdef DD_4_ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif



//#define LOG_WIFI_STATUS
//#define RECONNECT_REACQUIRE_WIFI


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
    void preConnect() {
      if (logToSerial)
        Serial.begin(serialBaud);
      while (true) {
        bool connected = _connectToClient(true);
        if (connected)
          break;
        else {
          if (logToSerial) Serial.println("lost WiFi ... try again ...");
          WiFi.disconnect();
          delay(500);
        }  
      }
      if (logToSerial) Serial.println("client conntected");
    }
    void flush() {
      client.flush();
    }
    void validConnection() {
#ifdef LOG_WIFI_STATUS
      if (logToSerial) {
        Serial.print(" ... validate ...");
        Serial.print(lastKeepAliveMillis);
        Serial.println(" ... ");
      }
#endif      
#ifdef RECONNECT_REACQUIRE_WIFI
      if (!client.connected()) {
        if (logToSerial) Serial.println("release WiFi since lost connection");
        WiFi.disconnect();
      }
#endif        
      uint8_t status = WiFi.status();
      if (status != WL_CONNECTED) {
        if (logToSerial) Serial.println("lost WiFi ... try bind WiFi again ...");
        client.stop();
        WiFi.disconnect();
        if (_connectToClient(true)) {
          if (logToSerial) Serial.println("re-acquired WiFi and connection");
        }
      } else if (!client.connected()) {
        client.stop();
        if (logToSerial) Serial.println("lost connection ... try bind connect again ...");
          if (_connectToClient(false)) {
            if (logToSerial) Serial.println("reconnected");
          }
      }
    }
  private:
    bool _connectToNetwork() {
      WiFi.begin(ssid, password);
      long last = millis();
      while (true) {
        uint8_t status = WiFi.status();
        if (status == WL_CONNECTED)
          break;
        delay(200);
        long diff = millis() - last;
        if (diff > 1000) {
          if (logToSerial) {
            Serial.print("binding WIFI ");
            Serial.print(ssid);
#ifdef LOG_WIFI_STATUS
            Serial.print(" ... ");
            Serial.print(status);
#endif
            Serial.println(" ...");
          }
          last = millis();
        }
      }
      if (logToSerial) {
        Serial.print("binded WIFI ");
        Serial.println(ssid);
      }
      return true;
    }
    bool _connectToClient(bool initConnect) {
      if (initConnect) {
        if (!_connectToNetwork())
          return false;
        server.begin();
      }
      long last = millis();
      while (true) {
        client = server.available();
        if (client) {
          break;
        } else {
          if (WiFi.status() != WL_CONNECTED) 
            return false;
          long diff = millis() - last;
          if (diff >= 1000) {
            IPAddress localIP = WiFi.localIP();
            uint32_t localIPValue = localIP;
            if (logToSerial) {
#ifdef LOG_WIFI_STATUS
              Serial.print("via WIFI ... ");
              Serial.print(WiFi.status());
              Serial.print(" ... ");
#endif
              Serial.print("listening on ");
// #ifdef LOG_WIFI_STATUS
//               Serial.print("(");
//               Serial.print(localIPValue);
//               Serial.print(") ");
// #endif
              Serial.print(localIP);
              Serial.print(":");
              Serial.print(port);
              Serial.println(" ...");
            }
            last = millis();
            if (localIPValue == 0)
              return false;
          }
        }
      }
      return true;
    }
  private:
    const char* ssid;
    const char *password;
    int port;
    bool logToSerial;
    WiFiServer server;
    WiFiClient client;
};



#endif
