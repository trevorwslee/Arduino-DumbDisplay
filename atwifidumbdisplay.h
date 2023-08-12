
#ifndef atwifidumbdisplay_h
#define atwifidumbdisplay_h

#ifndef ESP_SERIAL
  #error Must define the macro ESP_SERIAL ... and must begin() it first thing in setup() block \
    *** \
    e.g. STM32F103: PA3 (RX2) ==> TX; PA2 (TX2) ==> RX \
    #define DD_SERIAL Serial2 \
    *** \
    e.g. Pico \
    UART Serial2(8, 9, 0, 0);  // 8: PICO_TX; 9: PICO_RX \
    #define DD_SERIAL Serial2
#endif

#include "dumbdisplay.h"

#define DD_DEBUG_AT

#ifdef DD_DEBUG_AT
  #define DEBUG_ESP_AT
#endif
#include "_loespat.h"


/// Subclass of DDInputOutput
class DDATWiFiIO: public DDInputOutput {
  public:
    /* ESP chipset as WIFI with AT commands IO mechanism */
    /* - ssid: WIFI name (pass to WiFi) */
    /* - passphrase: WIFI password (pass to WiFi) */
    /* - serverPort: server port (pass to WiFiServer) */
    DDATWiFiIO(const char* ssid, const char *passphrase, int serverPort = DD_WIFI_PORT):
                   DDInputOutput(serialBaud, false, false) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
      Serial.begin(DD_SERIAL_BAUD);
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
      atWrite.write(buf, size);
    }
    bool preConnect(bool firstCall) {
      return atConnect();
    }
    void flush() {
      atFlush();
    }
    void validConnection() {
#ifdef LOG_DDWIFI_STATUS
      if (logToSerial) {
        Serial.println(" ... validate ...");
      }
#endif      
      atCheckConnection();
    }
    bool canConnectPassive() {
      return true;
    }
    bool canUseBuffer() {
      return true;
    }
  private:
    void atCheckConnection() {
        int state = LOEspAt::CheckState();
        if (state == 1 || state == 3) {
          // 1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
          // 3: ESP32 station is in Wi-Fi connecting or reconnecting state.
          return;
        }
        if (state != 2) {
          // 2: ESP32 station has connected to an AP, and got an IPv4 address.
          if (logToSerial) Serial.println("lost WiFi ... try bind WiFi again ...");
          //client.stop();
          //WiFi.disconnect();
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
          if (stateMillis == 0 || diff > 1000) {
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
    //bool logToSerial;
    char connectionState;
    long stateMillis;
    //WiFiServer server;
    //WiFiClient client;
};



#endif