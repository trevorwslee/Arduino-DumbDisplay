#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

#include "dumbdisplay.h"
#include <WiFi.h>

class DDWifiServerIO: public DDInputOutput {
  public:
    /* WiFI IO mechanism */
    /* - ssid: WIFI name (pass to WiFi) */
    /* - passphrase: WIFI password (pass to WiFi) */
    /* - serverPort: server port (pass to WiFiServer) */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    DDWifiServerIO(const char* ssid, const char *passphrase, int serverPort = DD_WIFI_PORT,
                   bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                     DDInputOutput(serialBaud, enableSerial, enableSerial),
                     server(serverPort) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
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
      DDInputOutput::preConnect();
      while (true) {
        bool connected = _connectToClient();
        if (connected)
          break;
        else {
          Serial.println("WiFi lost ... try again ...");
          WiFi.disconnect();
          delay(500);
        }  
      }
      Serial.println("client conntected");
    }
    void flush() {
      client.flush();
    }
  private:
    bool _connectToNetwork() {
      WiFi.begin(ssid, password);

      long last = millis();
      while (true) {
        uint8_t status = WiFi.status();
        if (status == WL_CONNECTED)
          break;
        if (status == WL_CONNECTION_LOST)
          return false;
        delay(200);
        long diff = millis() - last;
        if (diff > 1000) {
          Serial.print("binding WIFI ");
          Serial.print(ssid);
          Serial.print(" ... ");
          Serial.print(WiFi.status());
          Serial.println(" ...");
          last = millis();
        }
      }
      Serial.print("binded WIFI ");
      Serial.println(ssid);
      return true;
    }
    bool _connectToClient() {
      if (!_connectToNetwork())
        return false;
      server.begin();
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
            Serial.print("via WIFI ... ");
            Serial.print(WiFi.status());
            Serial.print(" ... ");
            Serial.print("listening on ");
            Serial.print(WiFi.localIP());
            Serial.print(":");
            Serial.print(port);
            Serial.println(" ...");
            last = millis();
          }
        }
      }
      return true;
    }
  private:
    const char* ssid;
    const char *password;
    int port;
    WiFiServer server;
    WiFiClient client;
};



#endif
