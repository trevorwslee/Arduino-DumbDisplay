

#include <ESP8266WiFi.h>
#include <espnow.h>

// a convenient macro for checking ESP Now result status
#define ESP_OK 0


#define FAN_PIN 0



// define a structure as ESP Now packet (the same as server)
struct ESPNowCommandPacket {
  char commandTarget[32];
  char commandAction[32];
};


void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  ESPNowCommandPacket receivedPacket;
  memcpy(&receivedPacket, incomingData, sizeof(receivedPacket));
  String commandTarget = receivedPacket.commandTarget;
  String commandAction = receivedPacket.commandAction;
  Serial.println(String("* Received command for [") + commandTarget + "] to [" + commandAction + "]");
  if (commandTarget == "fan") {
    if (commandAction == "on") {
      digitalWrite(FAN_PIN, 1);
      Serial.println("- turned on fan");
    } else if (commandAction == "off") {
      digitalWrite(FAN_PIN, 0);
      Serial.println("- turned off fan");
    }
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("*****");
  Serial.println(String("* agent MAC is ") + WiFi.macAddress());
  Serial.println("*****");


  pinMode(FAN_PIN, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("Done initializing ESP-NOW");
  } else {
    Serial.println("Error initializing ESP-NOW");
  }  

  // Register "receive callback"
  if (esp_now_register_recv_cb(OnDataRecv) == ESP_OK) {
    Serial.println("Done registering receive callback");
  } else {
    Serial.println("Error registering receive callback");
  }
}

long lastShowIdleMillis = 0;
void loop() {
  long nowMillis = millis();
  if ((nowMillis - lastShowIdleMillis) >= 5000) {
    Serial.println(String("ESP01 agent MAC is ") + WiFi.macAddress());
    lastShowIdleMillis = nowMillis;
 }
}