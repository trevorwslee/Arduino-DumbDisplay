#include "ddjoystick.h"

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
  return ".";
}
// const char *ToRepresentationFromCode(JoystickPressCode* joystickPressCode) {
//   if (joystickPressCode != NULL) {
//     JoystickPress joystickPress;
//     int xPressedCode = joystickPressCode->xPressed;
//     int yPrexxedCode = joystickPressCode->yPressed;
//     if (xPressedCode == 2 || xPressedCode == 1) {
//       joystickPress.xPressed = 1;
//     } else if (xPressedCode == -1) {
//       joystickPress.xPressed = -1;
//     }
//     return ToRepresentation(&joystickPress, joystickPressCode->swPressed);
//   } else {
//     return ".";
//   }
// }


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

#if defined(ARDUINO_AVR_UNO)
JoystickPressTracker *xTracker = SetupNewJoystickPressTracker(A0, false);
JoystickPressTracker *yTracker = SetupNewJoystickPressTracker(A1, true);
ButtonPressTracker *swTracker = SetupNewButtonPressTracker(8);
ButtonPressTracker *aTracker = SetupNewButtonPressTracker(2);
ButtonPressTracker *bTracker = SetupNewButtonPressTracker(3);
ButtonPressTracker *cTracker = SetupNewButtonPressTracker(4);
ButtonPressTracker *dTracker = SetupNewButtonPressTracker(5);
JoystickJoystick *joystick = new JoystickJoystick(xTracker, yTracker, swTracker);
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker);
const int JoystickCount = 2;
JoystickInterface *Joysticks[JoystickCount] = {joystick, buttons};
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
const int JoystickCount = 2;
JoystickInterface *Joysticks[JoystickCount] = {joystick, buttons};
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
const int JoystickCount = 2;
JoystickInterface *Joysticks[JoystickCount] = {joystick, buttons};
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
const int JoystickCount = 2;
JoystickInterface *Joysticks[JoystickCount] = {joystick, buttons};
#endif

void setup()
{
  Serial.begin(115200);
}
void loop()
{
  const JoystickPress *joystickPresses[JoystickCount];
  bool swPresses[JoystickCount];
  bool show = false;
  for (int i = 0; i < JoystickCount; i++)
  {
    joystickPresses[i] = Joysticks[i]->checkJoystickPress(JoystickPressAutoRepeatMillis);
    swPresses[i] = Joysticks[i]->checkSWPressed(JoystickPressAutoRepeatMillis);
    show |= joystickPresses[i] != NULL || swPresses[i];
  }
  if (show)
  {
    for (int i = 0; i < JoystickCount; i++)
    {
      const JoystickPress *joystickPress = joystickPresses[i];
      bool swPressed = swPresses[i];
      const char *representation = ToRepresentation(joystickPress, swPressed);
      Serial.print("[");
      Serial.print(representation);
      Serial.print("]");
    }
    Serial.println();
  }
  // delay(200);
}
