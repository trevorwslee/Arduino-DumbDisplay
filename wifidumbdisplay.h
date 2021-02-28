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
      this->passphrase = passphrase;
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
      WiFi.begin(ssid, passphrase);
      long last = millis();
      while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        long diff = millis() - last;
        if (diff > 1000) {
          Serial.print("binding WIFI ");
          Serial.print(ssid);
          Serial.println(" ...");
          last = millis();
        }
      }
      Serial.print("binded WIFI ");
      Serial.println(ssid);
      server.begin();
      while (true) {
        delay(200);
        client = server.available();
        if (client) {
          break;
        } else {
          long diff = millis() - last;
          if (diff >= 1000) {
            Serial.print("listening on ");
            Serial.print(WiFi.localIP());
            Serial.print(":");
            Serial.print(port);
            Serial.println(" ...");
            last = millis();
          }
        }
      }
      Serial.println("client conntected");
    }
    void flush() {
      client.flush();
    }
  private:
    const char* ssid;
    const char *passphrase;
    int port;
    WiFiServer server;
    WiFiClient client;
};



#endif
