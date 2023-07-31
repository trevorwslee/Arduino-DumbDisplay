#include <Arduino.h>

//#define ESP_NOW_SERVER_FOR_MAC 0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15
//#define ESP_NOW_CLIENT

// ddjoystick.h is include in DumbDisplay Arduino library
#include "ddjoystick.h"

#if defined(ESP_NOW_CLIENT)
DecodedJoystick *Joystick1 = new DecodedJoystick(false);
DecodedJoystick *Joystick2 = new DecodedJoystick(true);
#elif defined(ESP32)
JoystickPressTracker *xTracker = SetupNewJoystickPressTracker(34, false);
JoystickPressTracker *yTracker = SetupNewJoystickPressTracker(35, true);
ButtonPressTracker *swTracker = SetupNewButtonPressTracker(33);
ButtonPressTracker *aTracker = SetupNewButtonPressTracker(23);
ButtonPressTracker *bTracker = SetupNewButtonPressTracker(22);
ButtonPressTracker *cTracker = SetupNewButtonPressTracker(21);
ButtonPressTracker *dTracker = SetupNewButtonPressTracker(19);
JoystickJoystick *joystick = new JoystickJoystick(xTracker, yTracker, swTracker);
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker);
JoystickInterface *Joystick1 = joystick;
JoystickInterface *Joystick2 = buttons;
#elif defined(ESP8266)
ButtonPressTracker *upTracker = SetupNewButtonPressTracker(D7);
ButtonPressTracker *downTracker = SetupNewButtonPressTracker(D6);
ButtonPressTracker *leftTracker = SetupNewButtonPressTracker(D5);
ButtonPressTracker *rightTracker = SetupNewButtonPressTracker(D4);
ButtonPressTracker *midTracker = SetupNewButtonPressTracker(D3);
ButtonPressTracker *setTracker = SetupNewButtonPressTracker(D2);
ButtonPressTracker *rstTracker = SetupNewButtonPressTracker(D1);
ButtonJoystick *joystick = new ButtonJoystick(upTracker, rightTracker, downTracker, leftTracker, midTracker);
ButtonsOnly *buttons = new ButtonsOnly(setTracker, rstTracker, NULL, NULL);
JoystickInterface *Joystick1 = joystick;
JoystickInterface *Joystick2 = buttons;
#elif defined(ARDUINO_AVR_UNO)
JoystickPressTracker *xTracker = SetupNewJoystickPressTracker(A0, false);
JoystickPressTracker *yTracker = SetupNewJoystickPressTracker(A1, true);
ButtonPressTracker *swTracker = SetupNewButtonPressTracker(8);
ButtonPressTracker *aTracker = SetupNewButtonPressTracker(2);
ButtonPressTracker *bTracker = SetupNewButtonPressTracker(3);
ButtonPressTracker *cTracker = SetupNewButtonPressTracker(4);
ButtonPressTracker *dTracker = SetupNewButtonPressTracker(5);
JoystickJoystick *joystick = new JoystickJoystick(xTracker, yTracker, swTracker);
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker);
JoystickInterface *Joystick1 = joystick;
JoystickInterface *Joystick2 = buttons;
#elif defined(ARDUINO_AVR_NANO)
JoystickPressTracker *xTracker = SetupNewJoystickPressTracker(A3, false);
JoystickPressTracker *yTracker = SetupNewJoystickPressTracker(A2, false);
ButtonPressTracker *swTracker = SetupNewButtonPressTracker(A1);
JoystickJoystick *joystick = new JoystickJoystick(xTracker, yTracker, swTracker);
ButtonPressTracker *cTracker = SetupNewButtonPressTracker(5);
ButtonPressTracker *bTracker = SetupNewButtonPressTracker(6);
ButtonPressTracker *dTracker = SetupNewButtonPressTracker(7);
ButtonPressTracker *aTracker = SetupNewButtonPressTracker(8);
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker);
JoystickInterface *Joystick1 = joystick;
JoystickInterface *Joystick2 = buttons;
#elif defined(PICO_SDK_VERSION_MAJOR)
JoystickPressTracker *xTracker = SetupNewJoystickPressTracker(26, true);
JoystickPressTracker *yTracker = SetupNewJoystickPressTracker(27, true);
ButtonPressTracker *swTracker = SetupNewButtonPressTracker(16);
ButtonPressTracker *aTracker = SetupNewButtonPressTracker(18);
ButtonPressTracker *dTracker = SetupNewButtonPressTracker(19);
ButtonPressTracker *cTracker = SetupNewButtonPressTracker(20);
ButtonPressTracker *bTracker = SetupNewButtonPressTracker(21);
JoystickJoystick *joystick = new JoystickJoystick(xTracker, yTracker, swTracker);
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker);
JoystickInterface *Joystick1 = joystick;
JoystickInterface *Joystick2 = buttons;
#endif

#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
#include <ESP8266WiFi.h>
#include <espnow.h>
struct ESPNowJoystickData
{
  JoystickPressCode joystickPressCode1;
  JoystickPressCode joystickPressCode2;
};
#endif

#if defined(ESP_NOW_SERVER_FOR_MAC)
uint8_t ClientMACAddress[] = {ESP_NOW_SERVER_FOR_MAC};
void OnSentData(uint8_t *mac_addr, uint8_t sendStatus)
{
}
#endif

#if defined(ESP_NOW_CLIENT)
volatile long lastReceivedJoystickDataMillis = 0;
void OnReceivedData(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  ESPNowJoystickData joystickData;
  memcpy(&joystickData, incomingData, len);
  Joystick1->decode(joystickData.joystickPressCode1);
  Joystick2->decode(joystickData.joystickPressCode2);
  lastReceivedJoystickDataMillis = millis();
}
#endif

const long JoystickPressAutoRepeatMillis = 200; // 0 means no auto repeat

#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
int espNowStatus = 0;
#endif

void setup()
{
  Serial.begin(115200);
#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
  WiFi.mode(WIFI_STA);
  espNowStatus = esp_now_init();
  if (espNowStatus == 0)
  {
#if defined(ESP_NOW_SERVER_FOR_MAC)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnSentData);
    esp_now_add_peer(ClientMACAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0); // 1 is the channel
#else
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnReceivedData);
#endif
  }
#endif
}

const char *ToRepresentation(const JoystickPress *joystickPress, bool swPressed)
{
  if (swPressed)
  {
    return "#";
  }
  int xPressed = joystickPress != NULL ? joystickPress->xPressed : 0;
  int yPressed = joystickPress != NULL ? joystickPress->yPressed : 0;
  if (xPressed == 0 && yPressed != 0)
  {
    return yPressed == -1 ? "↑" : "↓";
  }
  if (xPressed != 0 && yPressed == 0)
  {
    return xPressed == -1 ? "←" : "→";
  }
  if (xPressed != 0 && yPressed != 0)
  {
    if (xPressed == -1)
    {
      return yPressed == -1 ? "↖" : "↙";
    }
    else
    {
      return yPressed == -1 ? "↗" : "↘";
    }
  }
  return NULL;
}

char RepresentationBuffer[5];
const char *CheckRepresentation(JoystickInterface *joystick, int repeat)
{
  if (joystick->forButtonsOnly())
  {
    const ABCDPressed *pressed = joystick->checkABCDPressed(repeat);
    if (pressed != NULL)
    {
      int i = 0;
      if (pressed->aPressed)
      {
        RepresentationBuffer[i++] = 'A';
      }
      if (pressed->bPressed)
      {
        RepresentationBuffer[i++] = 'B';
      }
      if (pressed->cPressed)
      {
        RepresentationBuffer[i++] = 'C';
      }
      if (pressed->dPressed)
      {
        RepresentationBuffer[i++] = 'D';
      }
      if (i > 0)
      {
        RepresentationBuffer[i] = 0;
        return RepresentationBuffer;
      }
    }
  }
  else
  {
    const JoystickPress *joystickPress = joystick->checkJoystickPress(repeat);
    bool swPressed = joystick->checkSWPressed(repeat);
    if (joystickPress != NULL || swPressed)
    {
      return ToRepresentation(joystickPress, swPressed);
    }
  }
  return NULL;
}
bool CheckRepresentation(JoystickInterface *joystick, int repeat, String &buffer)
{
  bool pressed = true;
  const char *representation = CheckRepresentation(joystick, repeat);
  if (representation == NULL)
  {
    representation = ".";
    pressed = false;
  }
  buffer = String("[") + representation + "]";
  return pressed;
}
#if defined(ESP_NOW_SERVER_FOR_MAC)
bool CheckJoystickPressCode(JoystickInterface *joystick, JoystickPressCode &joystickPressCode, int repeat, String &representationBuffer)
{
  const char *representation;
  bool pressed;
  if (joystick->checkJoystickPressCode(joystickPressCode, repeat))
  {
    representationBuffer = String("x:") + String(joystickPressCode.xPressed) +
                           String("/y:") + String(joystickPressCode.yPressed) +
                           String("/sw:") + String(joystickPressCode.swPressed);
    representation = representationBuffer.c_str();
    pressed = true;
  }
  else
  {
    representation = ".";
    pressed = false;
  }
  representationBuffer = String("[") + representation + "]";
  return pressed;
}
#endif

long lastActivityMillis = 0;

void loop()
{
#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
  if (espNowStatus != 0)
  {
    Serial.println("ESP Now not properly initialized!");
    Serial.print("status is ");
    Serial.println(espNowStatus);
    delay(2000);
    return;
  }
#endif

#if defined(ESP_NOW_CLIENT)
  long nowMillis = millis();
  if (lastReceivedJoystickDataMillis == 0)
  {
    if ((nowMillis - lastActivityMillis) >= 2000)
    {
      Serial.print("IDLE: my MAC is ");
      Serial.println(WiFi.macAddress());
      lastActivityMillis = nowMillis;
    }
    return;
  }
  lastActivityMillis = nowMillis;
#endif

  String representation1;
  String representation2;

#if defined(ESP_NOW_SERVER_FOR_MAC)
  JoystickPressCode joystickPressCode1;
  JoystickPressCode joystickPressCode2;
  bool checkJoystick1 = CheckJoystickPressCode(Joystick1, joystickPressCode1, JoystickPressAutoRepeatMillis, representation1);
  bool checkJoystick2 = CheckJoystickPressCode(Joystick2, joystickPressCode2, JoystickPressAutoRepeatMillis, representation2);
  if (checkJoystick1 || checkJoystick2)
  {
    ESPNowJoystickData joystickData;
    memcpy(&joystickData.joystickPressCode1, &joystickPressCode1, sizeof(joystickPressCode1));
    memcpy(&joystickData.joystickPressCode2, &joystickPressCode2, sizeof(joystickPressCode2));
    esp_now_send(ClientMACAddress, (uint8_t *)&joystickData, sizeof(joystickData));
  }
#else
  bool checkJoystick1 = CheckRepresentation(Joystick1, JoystickPressAutoRepeatMillis, representation1);
  bool checkJoystick2 = CheckRepresentation(Joystick2, JoystickPressAutoRepeatMillis, representation2);
#endif
  if (checkJoystick1 || checkJoystick2)
  {
    Serial.println(representation1 + representation2);
  }
}
