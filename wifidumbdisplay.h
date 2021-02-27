#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

#include "dumbdisplay.h"
#include <WiFi.h>

class DDWifiServerIO: public DDInputOutput {
  public:
    /* WiFI IO mechanism */
    /* - enableSerial: enable Serial as well or not (if enabled, connecting via USB will also work) */
    DDWifiServerIO(const char* ssid, const char *passphrase, int serverPort,
                   bool enableSerial = false, unsigned long serialBaud = DD_SERIAL_BAUD):
                     DDInputOutput(serialBaud, enableSerial, enableSerial),
                     server(serverPort) {
      this->ssid = ssid;
      this->passphrase = passphrase;
      this->serverPort = serverPort;
    }
    bool available() {
      return client.available() > 0;
    }
    char read() {
      return client.read();
    } 
    void print(const String &s) {
      server.print(s); 
    }
    void print(const char *p) {
      server.print(p); 
    }
    void preConnect() {
      DDInputOutput::preConnect();
      WiFi.begin(ssid, passphrase);
      long last = millis();
      while (WiFi.status() != WL_CONNECTED) {
        long diff = millis() - last;
        if (diff >1000) {
          Serial.print("binding WIFI ");
          Serial.print(ssid);
          Serial.println(" ...");
          last = millis();
        }
      }
      Serial.print("binded WIFI ");
      Serial.println(ssid);
      while (true) {
        client = server.available();
        if (client) {
          break;
        } else {
          long diff = millis() - last;
          if (diff >= 1000) {
            Serial.print("listening on ");
            Serial.print(WiFi.localIP());
            Serial.print(":");
            Serial.print(serverPort);
            Serial.println(" ...");
            last = millis();
          }
        }
      }
      Serial.println("client conntected");
    }
    void flush() {
#ifndef DD_4_ESP32      
      server.flush();
#endif      
    }
  private:
    const char* ssid;
    const char *passphrase;
    int serverPort;
    WiFiServer server;
    WiFiClient client;
};



#endif
