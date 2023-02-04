


#include <esp_now.h>
#include <WiFi.h>



#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)

  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

#endif


// define a structure as ESP Now packet (the same as server)
struct ESPNowCommandPacket {
  char commandTarget[32];
  char commandAction[32];
};


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  dumbdisplay.writeComment(String("received command (") + String(len) + ")");
  ESPNowCommandPacket packet;
  memcpy(&packet, incomingData, sizeof(packet));
  const char* commandTarget = packet.commandTarget;
  const char* commandAction = packet.commandAction;
  dumbdisplay.writeComment(String("received command [") + commandTarget + "] to [" + commandAction + "]");
}

void setup() {

// #if defined(DD_USING_WIFI)
//   Serial.begin(115200);
//   Serial.println(WiFi.macAddress());
//   Serial.println(WiFi.macAddress());
//   Serial.println(WiFi.macAddress());
// #endif

  dumbdisplay.connect();

  dumbdisplay.writeComment("initializing ...");
  dumbdisplay.writeComment(String("... ") + WiFi.macAddress() + " ...");

#if defined(DD_USING_WIFI)
  // Set device as a Wi-Fi Station and also a Access Point
  WiFi.mode(WIFI_AP_STA);
#else
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
#endif

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    dumbdisplay.writeComment("Error initializing ESP-NOW");
  }  

  // Register "receive callback"
  if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
    dumbdisplay.writeComment("Error registering receive callback");
  }

  dumbdisplay.writeComment("... initialized");
}


void loop() {
#if defined(DD_USING_WIFI)
  Serial.println(String("Client MAC is ") + WiFi.macAddress());
#endif
  dumbdisplay.writeComment(String("Client MAC is ") + WiFi.macAddress());
  DDDelay(2000);
}


