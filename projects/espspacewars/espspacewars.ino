
// *** 
// * adapted from: TTGO T Display (ESP32) - Space Shooter game- free code
// * -- https://www.youtube.com/watch?v=KZMkGDyGjxQ&t=310s
// *** 


//#define ESP_NOW_SERVER_FOR_MAC 0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15
//#define ESP_NOW_CLIENT


#include "ddjoystick.h"


#if defined(ESP_NOW_CLIENT)

  #define DOWNLOAD_IMAGES
  #define SHOW_SPACE
  DecodedJoystick* joystick = new DecodedJoystick(false);
  DecodedJoystick* buttons = new DecodedJoystick(true);
  
#elif defined(ESP32)
  
  #define DOWNLOAD_IMAGES
  #define SHOW_SPACE
  #define DEBUG_LED_PIN 2
  JoystickInterface* buttons = new ButtonsOnly(SetupNewButtonPressTracker(22),
                                               SetupNewButtonPressTracker(23),
                                               NULL, NULL);
  JoystickInterface* joystick = new JoystickJoystick(SetupNewJoystickPressTracker(36, true),
                                                     SetupNewJoystickPressTracker(39, true),
                                                     NULL);
 
#elif defined(ESP8266)
  
  #define DOWNLOAD_IMAGES
  #define SHOW_SPACE
  ButtonPressTracker *upTracker = SetupNewButtonPressTracker(D7);
  ButtonPressTracker *downTracker = SetupNewButtonPressTracker(D6);
  ButtonPressTracker *leftTracker = SetupNewButtonPressTracker(D5);
  ButtonPressTracker *rightTracker = SetupNewButtonPressTracker(D4);
  ButtonPressTracker *setTracker = SetupNewButtonPressTracker(D2);
  ButtonPressTracker *rstTracker = SetupNewButtonPressTracker(D1);
  JoystickInterface *joystick = new ButtonJoystick(upTracker, rightTracker, downTracker, leftTracker, NULL);
  JoystickInterface *buttons = new ButtonsOnly(rstTracker, setTracker, NULL, NULL);

#else 

#error not configured for board yet

#endif




#if !defined(ESP_NOW_SERVER_FOR_MAC)

  #if defined(WIFI_SSID)
    #include "wifidumbdisplay.h"
    DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
  #else
    #include "dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDInputOutput(115200));
  #endif

#endif


#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
#include <ESP8266WiFi.h>
#include <espnow.h>
struct ESPNowJoystickData {
  JoystickPressCode joystickPressCode;
  JoystickPressCode buttonsPressCode;
};
#endif


#if defined(ESP_NOW_SERVER_FOR_MAC)
uint8_t ClientMACAddress[] = { ESP_NOW_SERVER_FOR_MAC };
void OnSentData(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("... sent ");
  if (sendStatus == 0)
  {
    Serial.print("SUCCESSFUL");
  }
  else
  {
    Serial.print("FAILED");
  }
  Serial.println(" ...");
}
#endif


int espNowStatus = 0;
long lastActivityMillis = 0;
int readyStage = 0;


#if defined(ESP_NOW_CLIENT)
volatile long lastReceivedJoystickDataMillis = 0;
void OnReceivedData(uint8_t * mac, uint8_t *incomingData, uint8_t len)
{
  ESPNowJoystickData receivedJoystickData;
  memcpy(&receivedJoystickData, incomingData, len);
  joystick->decode(receivedJoystickData.joystickPressCode);
  buttons->decode(receivedJoystickData.buttonsPressCode);
  lastActivityMillis = millis();
}
#endif





void OnIdle()
{
  yield();
#if defined(ESP_NOW_CLIENT)
  if (readyStage == 2) {
    long nowMillis = millis();
    if (lastReceivedJoystickDataMillis == 0) {
      if ((nowMillis - lastActivityMillis) >= 2000) {
        dumbdisplay.writeComment(String("My MAC is ") + WiFi.macAddress());
        lastActivityMillis = nowMillis;
      }
      return;
    }
    lastActivityMillis = nowMillis;
  }
#endif
}


#if !defined(ESP_NOW_SERVER_FOR_MAC)
#include "Core.h"
#endif


void setup(void)
{
#if defined(ESP_NOW_SERVER_FOR_MAC)
  Serial.begin(115200);
#endif

#if defined(DEBUG_LED_PIN)
  pinMode(DEBUG_LED_PIN, OUTPUT);
  digitalWrite(DEBUG_LED_PIN, 0);
#endif

#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
  WiFi.mode(WIFI_STA);
  espNowStatus = esp_now_init();
  if (espNowStatus == 0) {
 #if defined(ESP_NOW_SERVER_FOR_MAC)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnSentData);
    esp_now_add_peer(ClientMACAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
 #else
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnReceivedData);
 #endif
  }
#endif

}

void loop() {
#if defined(ESP_NOW_SERVER_FOR_MAC)
  JoystickPressCode joystickPressCode;
  JoystickPressCode buttonsPressCode;
  bool joystickPressed = joystick->checkJoystickPressCode(joystickPressCode, 50);
  bool buttonsPressed = buttons->checkJoystickPressCode(buttonsPressCode);
  if (joystickPressed || buttonsPressed) {
    ESPNowJoystickData joystickData;
    memcpy(&joystickData.joystickPressCode, &joystickPressCode, sizeof(joystickPressCode));
    memcpy(&joystickData.buttonsPressCode, &buttonsPressCode, sizeof(buttonsPressCode));
    esp_now_send(ClientMACAddress, (uint8_t *) &joystickData, sizeof(joystickData));
  }
  return;
#endif

#if defined(ESP_NOW_CLIENT)
  if (readyStage == 2) {
    OnIdle();
  }
#endif

#if !defined(ESP_NOW_SERVER_FOR_MAC)
  gameLoop();
#endif
}
