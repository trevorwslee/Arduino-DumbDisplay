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



#if defined(ARDUINO_AVR_UNO)
  // Arduino UNO
  // . 4 => TX; 5 => RX
  #include "SoftwareSerial.h"
  SoftwareSerial SS(4, 5);
  #define ESP_SERIAL SS
#elif defined(ARDUINO_ARCH_RP2040)
  // Raspberry Pi Pico
  // . TX: 8 ==> ESP01 RX (GPIO3) ; RX: 9 ==> ESP01 TX (GPIO1)
  UART Serial2(8, 9, 0, 0);
  #define ESP_SERIAL Serial2
#elif defined(ARDUINO_ARCH_STM32)
  // STM32
  // . STM32 PA3 (RX2) => ESP01 TX
  // . STM32 PA2 (TX2) => ESP01 RX
  HardwareSerial Serial2(PA3, PA2);
  #define ESP_SERIAL serial2 
#else
  #error "Unsupported board"
#endif  




//#define DEBUG_ESP_AT
#include "_loespat.h"


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  ESP_SERIAL.begin(115200);


  Serial.println("*** initializing ***");

#if defined(ARDUINO_AVR_UNO)  
  // adjust the baud rate, temporarily, between Arduino UNO and ESP01
  LOEspAt::SetBaudRate(14400);
  ESP_SERIAL.begin(14400);
#endif

  LOEspAt::InitAt();
  Serial.println("*** ready ***");
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
          LOEspAt::SendDataToClient(link_id, "<hi there>\n");
        }
      } else {
        if (check_link_id == -1) {
          Serial.println("XXX client disconnected");
          link_id = -1;
        }
      }
      if (link_id == -1) {
        Serial.print("END POINT: ");
        Serial.print(ap_ip);
        Serial.print(':');
        Serial.println(WIFI_PORT);
      }
    }
  }
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    if (link_id != -1) {
      if (LOEspAt::SendDataToClient(link_id, msg + "\n")) {
        Serial.print(">>> ");
        Serial.println(msg);
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
      Serial.print("<<< ");
      Serial.print(msg);
    }
  }
}

