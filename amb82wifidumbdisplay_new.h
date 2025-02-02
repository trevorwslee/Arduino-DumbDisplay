// ***
// *** NOT WORKING
// ***


#ifndef wifidumbdisplay_h
#define wifidumbdisplay_h

// after inclusion, can check DD_USING_WIFI to be sure WIFI is used
#define DD_USING_WIFI

#include "dumbdisplay.h"

#include <WiFi.h>



#define LOG_DDWIFI_STATUS
//#define WRITE_BYTE_BY_BYTES



/// ***** NOT WORKING *****
/// Subclass of DDInputOutput
class DDAmb82WiFiServerIO: public DDInputOutput {
  public:
    /* WiFI IO mechanism */
    /* - ssid: WIFI name (pass to WiFi) */
    /* - passphrase: WIFI password (pass to WiFi) */
    /* - serverPort: server port (pass to WiFiServer) */
    DDAmb82WiFiServerIO(char* ssid, char *passphrase, int serverPort = DD_WIFI_PORT): DDInputOutput(DD_SERIAL_BAUD, false, false),
                        //server(serverPort) {
                        server(serverPort, tProtMode::TCP_MODE, tBlockingMode::NON_BLOCKING_MODE) {
      this->ssid = ssid;
      this->password = passphrase;
      this->port = serverPort;
      this->bufferFilled = false;
      //this->logToSerial = logToSerial;
      //Serial.begin(DD_SERIAL_BAUD);f
      //this->server.setNonBlockingMode();
    }
    const char* getWhat() {
      return "ambWIFI";
    }
    bool available() {
      return _fillBuffer();
      // int count =  client.available();
      // return count > 0;
    }
    char read() {
      if (!_fillBuffer()) {
        return -1;
      }
      char c = this->buffer[0];
      this->bufferFilled = false;
      return c;
      // char ch = client.read();
      // if (false) {  // TODO: disable
      //     Serial.print("- read:[");
      //     Serial.print(ch);
      //     Serial.println("]");
      // }
      // return ch;
    } 
    void print(const String &s) {
#ifdef WRITE_BYTE_BY_BYTES      
      for (size_t i = 0; i < s.length(); i++) {
        write(s.charAt(i));
      }
#else
      // if (true) {  // TODO: disable debug
      //   Serial.print("* print: [");
      //   Serial.print(s);
      //   Serial.println("]");
      // }
      client.print(s);
      // if (true) {  // TODO: forced delay since 2024-11-04
      //   delay(200);
      // }
#endif      
    }
    void print(const char *p) {
#ifdef WRITE_BYTE_BY_BYTES      
    while (true) {
      char c = *(p++);
      if (c == 0) {
        break;
      }
      write(c);
    }
#else
      client.print(p);
      // if (true) {
      //   client.flush();
      // }
      // if (true) {  // TODO: forced delay since 2024-11-04
      //   delay(200);
      // }
#endif
    }
    void write(uint8_t b) {
#ifdef WRITE_BYTE_BY_BYTES      
      while (true) {
        size_t count = client.write(b);
        if (count > 0) {
          break;
        }
        delay(200);
      }
#else
      client.write(b);
#endif
    }
    void write(const uint8_t *buf, size_t size) {
#ifdef WRITE_BYTE_BY_BYTES
      for (size_t i = 0; i < size; i++) {
        write(buf[i]);
      }       
#else
      client.write(buf, size);
      // if (false) {  // TODO: forced delay since 2024-11-04
      //   delay(200);
      // }
#endif      
    }
    bool preConnect(bool firstCall) {
      if (firstCall) {  // since 2023-08-10
        if (!Serial) Serial.begin(DD_SERIAL_BAUD);
// #if defined(ARDUINO_UNOR4_WIFI)
//         if (WiFi.status() == WL_NO_MODULE) {
//           Serial.println("XXX communication with WiFi module failed");
//         }
//         String fv = WiFi.firmwareVersion();
//         if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
//           Serial.println("XXX please upgrade the firmware");
//         } else {
//           Serial.println("* WIFI_FIRMWARE_LATEST_VERSION=" + String(WIFI_FIRMWARE_LATEST_VERSION));
//         }
// #endif
      }
      if (true) {  // since 2023-08-16
        if (firstCall && !client.connected()) {
          connectionState = '0';
        }
        checkConnection();
        return connectionState == 'C';
      } else if (true) {  // since 2023-08-15
        if (firstCall && !client.connected()) {
          Serial.println("setup WIFI");
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          connectionState = ' ';
          stateMillis = 0;
        }
        checkConnection();
        return connectionState == 'C';
      } else if (true) {  // since 2023-06-05
        if (firstCall) {
          if (true) {
            if (client.connected()) {
              Serial.println("stop client for new setup");
              client.stop();
              delay(2000);  // wait a bit
            }
            WiFi.disconnect();
          } else  if (true) {  // since 2023-06-03
            client.stop();
            WiFi.disconnect();
          }
          Serial.println("setup WIFI");
          WiFi.begin(ssid, password);
          connectionState = ' ';
          stateMillis = 0;
        }
        //  delay(200);
        checkConnection();
        return connectionState == 'C';
      } else {  
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
    }
    void flush() {
      if (false) {
        client.flush();  // flush is not the expected -- just flush send buffer
      }
    }
    void validConnection() {
#ifdef LOG_DDWIFI_STATUS
      Serial.println(" ... validate ...");
#endif      
      checkConnection();
    }
    bool canConnectPassive() {
      return false;  // TODO: make it passive
    }
    bool canUseBuffer() {
      return false;  // buffering might make it fails to send (and marked disconnected)
    }
  private:
    void checkConnection() {
      if (connectionState == '0' || connectionState == 'C') {  // since 2023-08-16 added check connectionState == '0'
        if (connectionState == '0' || WiFi.status() != WL_CONNECTED) {
          if (connectionState == 'C') {
            Serial.println("lost WiFi ... try bind WiFi again ...");
            client.stop();
          }
          WiFi.disconnect();
          Serial.println("setup WIFI");
          WiFi.begin(ssid, password);
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
          if (diff >= 1000) {
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
            stateMillis = millis();
          }
          WiFiClient cli = server.available();
          if (cli && cli.connected()) {  // TODO: for some reason, under lying _sock an be 0xFF, which seems to indicated "try again"
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
    bool _fillBuffer() {
      if (this->bufferFilled) {
        return true;
      }
      int n = client.read(this->buffer, 1);
      if (n > 0) {
        this->bufferFilled = true;
        return true;
      } else {
        return false;
      }
    }  
  private:
    char* ssid;
    char *password;
    int port;
    char connectionState;
    long stateMillis;
    WiFiServer server;
    WiFiClient client;
    char buffer[1];
    bool bufferFilled;
};




#endif
