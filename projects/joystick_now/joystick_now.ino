

//#define ESP_NOW_SERVER_FOR_MAC 0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15
#define ESP_NOW_CLIENT


// ddjoystick.h is include in DumbDisplay Arduino library
#include "ddjoystick.h"



#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
#include <ESP8266WiFi.h>
#include <espnow.h>
struct ESPNowJoystickData {
  JoystickPressCode joystickPressCode1;
  JoystickPressCode joystickPressCode2;
};
#endif


#if defined(ESP_NOW_SERVER_FOR_MAC)
uint8_t ClientMACAddress[] = { ESP_NOW_SERVER_FOR_MAC };
#endif

#if defined(ESP_NOW_CLIENT)
ESPNowJoystickData receivedJoystickData;
volatile bool receivedJoystickDataValid = false;
volatile long lastReceivedJoystickDataMillis = 0;
void OnReceivedData(uint8_t * mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&receivedJoystickData, incomingData, len);
  receivedJoystickDataValid = true;
}
#endif



JoystickPressTracker *SetupNewJoystickPressTracker(uint8_t pin, bool reverseDir, int autoTuneThreshold = JoystickPressTracker::DefAutoTuneThreshold)
{
  pinMode(pin, INPUT);
  return new JoystickPressTracker(pin, reverseDir, autoTuneThreshold);
}
ButtonPressTracker *SetupNewButtonPressTracker(uint8_t pin)
{
  pinMode(pin, INPUT_PULLUP);
  return new ButtonPressTracker(pin);
}


#if defined(ESP_NOW_CLIENT)
DecodedJoystick *Joystick1 = new DecodedJoystick(true);
DecodedJoystick *Joystick2 = new DecodedJoystick(false);
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



int espNowStatus = 0;

void setup()
{
  Serial.begin(115200);
#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
  espNowStatus = esp_now_init();
  if (espNowStatus == 0) {
 #if defined(ESP_NOW_SERVER_FOR_MAC)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    //esp_now_register_send_cb(OnDataSent);
    espNowGood = esp_now_add_peer(ClientMACAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0) == 0;
 #else
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnReceivedData);
 #endif
  }
#endif

}



const long JoystickPressAutoRepeatMillis = 200; // 0 means no auto repeat

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
const char *CheckRepresentation(JoystickInterface* joystick, int repeat)
{
  if (joystick->forButtonsOnly()) {
    const ABCDPressed* pressed = joystick->checkABCDPressed(repeat);
    if (pressed != NULL) {
      int i = 0;
      if (pressed->aPressed) {
        RepresentationBuffer[i++] = 'A';
      } 
      if (pressed->bPressed) {
        RepresentationBuffer[i++] = 'B';
      } 
      if (pressed->cPressed) {
        RepresentationBuffer[i++] = 'C';
      } 
      if (pressed->dPressed) {
        RepresentationBuffer[i++] = 'D';
      } 
      if (i > 0) {
        RepresentationBuffer[i] = 0;
        return RepresentationBuffer;
      }
    }
  } else {
    const JoystickPress* joystickPress = joystick->checkJoystickPress(repeat);
    bool swPressed = joystick->checkSWPressed(repeat);
    if (joystickPress != NULL || swPressed) {
      return ToRepresentation(joystickPress, swPressed);
    }
  }
  return NULL;
}
bool CheckRepresentation(JoystickInterface* joystick, int repeat, String& buffer) {
#if defined(ESP_NOW_SERVER_FOR_MAC)  
  const char *representation;
  bool pressed = true;
  JoystickPressCode joystickPressCode;
  if (joystick->checkJoystickPressCode(joystickPressCode, repeat)) {
    buffer = String("x:") + String(joystickPressCode.xPressed) +
      String("/y:") + String(joystickPressCode.yPressed) +
      String("/sw:") + String(joystickPressCode.swPressed);
    representation = buffer.c_str();
  } else {
    representation = ".";
    pressed = false;
  }
#else
  bool pressed = true;
  const char *representation = CheckRepresentation(joystick, repeat);
  if (representation == NULL) {
    representation = ".";
    pressed = false;
  }
#endif  
  buffer = String("[") + representation + "]";
  return pressed;
}



long lastActivityMillis = 0;

void loop()
{
#if defined(ESP_NOW_SERVER_FOR_MAC) || defined(ESP_NOW_CLIENT)
  if (espNowStatus != 0) {
    Serial.println("ESP Now not properly initialized!");
    Serial.print("status is ");
    Serial.println(espNowStatus);
    delay(2000);
    return;
  }
#endif

  long nowMillis = millis();

#if defined(ESP_NOW_CLIENT)
  if (lastReceivedJoystickDataMillis == 0) {
    if ((nowMillis - lastActivityMillis) >= 2000) {
      Serial.print("IDLE: my MAC is ");
      Serial.println(WiFi.macAddress());
      lastActivityMillis = nowMillis;
    }
    return;
  }
  if (!receivedJoystickDataValid) {
    return;  
  }
  Joystick1->decode(receivedJoystickData.joystickPressCode1);
  Joystick2->decode(receivedJoystickData.joystickPressCode2);
  receivedJoystickDataValid = false;
  lastActivityMillis = nowMillis;
#endif

  String representation1;
  String representation2;
  if (CheckRepresentation(Joystick1, JoystickPressAutoRepeatMillis, representation1) ||
      CheckRepresentation(Joystick2, JoystickPressAutoRepeatMillis, representation2)) {
    Serial.println(representation1 + representation2);
  }
}
