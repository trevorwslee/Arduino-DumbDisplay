#include <Arduino.h>


// ***
// * For Serial
// * . SERIAL_BAUD_RATE is the baud rate; SERIAL_BAUD_RATE defaults to 115200
// * . assume "line ending" set to "line feed (LF) only"
// * . if using VSCode, can consider using SERIAL MONITOR
// ***

#define SERIAL_BAUD_RATE 115200


// ***
// * For WIFI ...
// * . please define WIFI_SSID and WIFI_PASSWORD (as below)
// * . WIFI_PORT is the port to listen to for client connection; WIFI_PORT defaults to 10201
// * . see Serial output for the IP
// ***

//#define WIFI_SSID        "<wifi-ssid>"
//#define WIFI_PASSWORD    "<wifi-password>"
#define WIFI_PORT        10201



#if defined(PICO_SDK_VERSION_MAJOR)
  // Raspberry Pi Pico ... TX: 8 ==> ESP RX (GPIO3) ; RX: 9  ==> ESP TX (GPIO1)
  UART Serial2(8, 9, 0, 0);
  #define ESP_SERIAL Serial2
#else
  // can use Serial2 for STM32
  // e.g. STM32F103: PA3 (RX2) ==> ESP TX (GPIO1) ; PA2 (TX2) ==> ESP RX (GPIO3)
  #error unexpected board
#endif  

// by default, use 115200 to communicate to ESP
#define ESP_BAUD_RATE    115200


//#define DEBUG_ESP_AT
#include "_loespat.h"


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  ESP_SERIAL.begin(ESP_BAUD_RATE);
}

long last_tick_ms = millis();
bool connected = false;
String ap_ip;
int link_id = -1;

void loop() {
  long now_ms = millis();
  bool tick = false;
  if ((now_ms - last_tick_ms) >= 1000) {
    // update [every second] tick
    tick = true;
    last_tick_ms = now_ms;
  }

  if (tick && !connected) {
    bool ok = true;
    if (ok) {
      // set station mode
      if (!LOEspAt::SetStationMode()) {
        Serial.println("XXX failed to set 'station mode'");
        ok = false;
      }
    }
    if (ok) {
      // connect to AP
      if (LOEspAt::ConnectAP(WIFI_SSID, WIFI_PASSWORD, ap_ip)) {
        int state = -1;
        for (int i = 0; i < 20; i++) {
          state = LOEspAt::CheckState();
          // 0: ESP32 station has not started any Wi-Fi connection.
          // 1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
          // 2: ESP32 station has connected to an AP, and got an IPv4 address.
          // 3: ESP32 station is in Wi-Fi connecting or reconnecting state.
          // 4: ESP32 station is in Wi-Fi disconnected state.
          if (state == -1 || state == 0 || state == 4) {
            break;
          }
          if (state == 2) {
            Serial.print("*** Connected to AP with IP ");
            Serial.println(ap_ip);
            break;
          }
          delay(100);
        }
        if (state != 2) {
          Serial.print("XXX connected to AP but not ready ... state=");
          Serial.println(state);
          ok = false;
        }
      } else {
        Serial.println("XXX failed to connect to AP");
        ok = false;
      }
      if (ok) {
        if (!LOEspAt::StartServer(10201)) {
          Serial.println("XXX failed to start server");
          ok = false;
        }
      }
      if (ok) {
        connected = true;
      }
    }
  }
  if (tick) {
    if (connected) {
      int check_link_id = LOEspAt::CheckForClientConnection();
      if (link_id == -1) {
        if (check_link_id != -1) {
          link_id = check_link_id;
          Serial.print("*** Client connected with 'link id' ");
          Serial.println(link_id);
        }
      } else {
        if (check_link_id == -1) {
          Serial.print("XXX client disconnected");
          link_id = -1;
        }
      }
      if (link_id == -1) {
        Serial.print("AP IP: ");
        Serial.println(ap_ip);
      }
    }
  }
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    if (link_id != -1) {
      msg += '\n';
      if (LOEspAt::SendDataToClient(link_id, msg)) {
        Serial.print("> LOCAL: ");
        Serial.print(msg);
      } else {
        Serial.println("XXX failed to forward message to client");
      }
    } else {
        Serial.println("XXX no client to forward message to");
    }
  }
  if (link_id != -1) {
    String msg;
    if (LOEspAt::ReceiveDataFromClient(link_id, msg)) {
      Serial.print("> REMOTE: ");
      Serial.print(msg);
    }
  }
}

