// ***
// * joystick press better if min / max detected (by moving the joystick)
// ***

#include <limits.h>


#if defined(ARDUINO_AVR_UNO)
#define VRX  A0
#define VRY  A1
#define SW   8
#elif defined(ARDUINO_AVR_NANO)
#define VRX  A2
#define VRY  A1
#define SW   A0
#elif defined(PICO_SDK_VERSION_MAJOR)
#define VRX 26
#define VRY 27
#define SW  16
#elif defined(ESP32)
#define VRX  12
#define VRY  14
#define SW   13
#elif defined(ESP8266)
#define BUTTONS_ONLY 
#define UP    D7
#define DOWN  D6
#define LEFT  D5
#define RIGHT D4
#define SW    D3
#endif

const int XYPressThreshold = 100;
bool trackedXYPressed2D = true;
const long XYPressAutoRepeatMillis = 0; // 0 means no auto repeat

class ButtonPressTracker
{
public:
  ButtonPressTracker(uint8_t pin)
  {
    this->pin = pin;
    this->pressed = false;
    this->blackOutMillis = 0;
    this->nextRepeatMillis = 0;
  }
  bool checkPressed(int repeat = 0)
  {
    return setPressed(digitalRead(this->pin) == 0, repeat);
  }
  bool checkPressedBypass()
  {
    setPressed(digitalRead(this->pin) == 0);
    return pressed;
  }
private:
  bool setPressed(bool pressed, int repeat = 0)
  {
    if (repeat == 0)
    {
      this->nextRepeatMillis = 0;
    }
    long nowMillis = millis();
    if (this->blackOutMillis != 0)
    {
      long diff = this->blackOutMillis - nowMillis;
      if (diff < 0)
      {
        this->blackOutMillis = 0;
      }
    }
    if (this->blackOutMillis == 0)
    {
      if (pressed != this->pressed)
      {
        this->pressed = pressed;
        this->blackOutMillis = nowMillis + 50;
        if (repeat != 0 && this->pressed)
        {
          this->nextRepeatMillis = nowMillis + repeat;
        }
        else
        {
          this->nextRepeatMillis = 0;
        }
        return this->pressed;
      }
    }
    if (this->nextRepeatMillis != 0)
    {
      long diff = this->nextRepeatMillis - nowMillis;
      if (diff < 0)
      {
        this->nextRepeatMillis = nowMillis + repeat;
        return true;
      }
    }
    return false;
  }
private:
  uint8_t pin;
  bool pressed;
  long blackOutMillis;
  long nextRepeatMillis;
};
class JoystickPressTracker
{
public:
  JoystickPressTracker(uint8_t pin, int minReading = 10, int maxReading = 1013)
  {
    this->pin = pin;
    setMinMax(minReading, maxReading, true);
  }
public:
  void setMinMax(int minReading, int maxReading, bool forceReset = false)
  {
    if (forceReset || this->minReading != minReading || this->maxReading != maxReading)
    {
      if (maxReading < minReading)
      {
        int temp = maxReading;
        maxReading = minReading;
        minReading = temp;
        this->reverseDir = true;
      }
      else
      {
        this->reverseDir = false;
      }
      this->maxReading = maxReading;
      this->minReading = minReading;
      this->pressedDir = 0;
      this->pressedMillis = 0;
      this->needReset = false;
      this->nextRepeatMillis = 0;
      this->autoRepeatDir = 0;
    }
  }
public:
  int8_t checkPressed(int repeat = 0)
  {
    return setReading(analogRead(this->pin), repeat);
  }
  int readPressedBypass()
  {
    int reading = analogRead(this->pin);
    setReading(reading);
    return readingToPressedDir(reading);
  }
  int readBypass()
  {
    int reading = analogRead(this->pin);
    setReading(reading);
    return reading;
  }
private:
  int readingToPressedDir(int reading)
  {
    if (reading <= this->minReading)
    {
      if (this->reverseDir)
      {
        return 1;
      }
      else
      {
        return -1;
      }
    }
    if (reading >= this->maxReading)
    {
      if (this->reverseDir)
      {
        return -1;
      }
      else
      {
        return 1;
      }
    }
    return 0;
  }
  int8_t setReading(int reading, int repeat = 0)
  {
    if (repeat == 0)
    {
      this->nextRepeatMillis = 0;
      this->autoRepeatDir = 0;
    }
    long nowMillis = millis();
    int8_t oriPressedDir = this->pressedDir;
    int pressedDir = readingToPressedDir(reading);
    if (pressedDir != 0)
    {
      this->pressedDir = pressedDir;
    }
    else
    {
      this->pressedDir = 0;
      this->nextRepeatMillis = 0;
      this->autoRepeatDir = 0;
    }
    if (!this->needReset && this->pressedMillis != 0 && (this->pressedDir == oriPressedDir))
    {
      long diffMillis = nowMillis - this->pressedMillis;
      if (diffMillis > 50)
      {
        this->pressedDir = 0;
        this->pressedMillis = 0;
        this->needReset = true;
        if (repeat != 0 && oriPressedDir != 0)
        {
          this->nextRepeatMillis = nowMillis + repeat;
          this->autoRepeatDir = oriPressedDir;
        }
        else
        {
          this->nextRepeatMillis = 0;
          this->autoRepeatDir = 0;
        }
        return oriPressedDir;
      }
    }
    else
    {
      if (this->pressedDir != 0)
      {
        if (this->pressedMillis == 0)
        {
          this->pressedMillis = millis();
        }
      }
      else
      {
        this->pressedMillis = 0;
        this->needReset = false;
      }
    }
    if (this->nextRepeatMillis != 0)
    {
      long diff = this->nextRepeatMillis - nowMillis;
      if (diff < 0)
      {
        this->nextRepeatMillis = nowMillis + repeat;
        return this->autoRepeatDir;
      }
    }
    return 0;
  }
private:
  int maxReading;
  int minReading;
  bool reverseDir;
private:
  uint8_t pin;
  int pressedDir;
  long pressedMillis;
  bool needReset;
  long nextRepeatMillis;
  int autoRepeatDir;
};
class Joystick2DTracker
{
public:
  Joystick2DTracker(JoystickPressTracker &xTracker, JoystickPressTracker &yTracker) : xTracker(xTracker), yTracker(yTracker)
  {
    this->lastPressedMillis = 0;
  }

public:
  bool checkPressed(int repeat = 0)
  {
    int xPressed = xTracker.checkPressed(repeat);
    int yPressed = yTracker.checkPressed(repeat);
    if (xPressed != 0 || yPressed != 0)
    {
      long nowMillis = millis();
      long diffMillis = nowMillis - this->lastPressedMillis;
      if (diffMillis > 50)
      {
        this->checkResultXPressed = xTracker.readPressedBypass();
        this->checkResultYPressed = yTracker.readPressedBypass();
        this->lastPressedMillis = nowMillis;
        return true;
      }
    }
    else
    {
      return 0;
    }
  }
private:
  JoystickPressTracker &xTracker;
  JoystickPressTracker &yTracker;
public:
  int checkResultXPressed;
  int checkResultYPressed;
  long lastPressedMillis;
};
const char *JoystickPressedToStr(int xPressed, int yPressed)
{
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


#if defined(BUTTONS_ONLY)
ButtonPressTracker *upTracker;
ButtonPressTracker *downTracker;
ButtonPressTracker *leftTracker;
ButtonPressTracker *rightTracker;
#else
JoystickPressTracker *xTracker;
JoystickPressTracker *yTracker;
Joystick2DTracker *xyTracker;
#endif
ButtonPressTracker *swTracker;

void setup()
{
  Serial.begin(115200);

#if defined(BUTTONS_ONLY)
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(LEFT, INPUT_PULLUP);
    pinMode(RIGHT, INPUT_PULLUP);
#else
    pinMode(VRX, INPUT);
    pinMode(VRY, INPUT);
#endif
    pinMode(SW, INPUT_PULLUP);

#if defined(BUTTONS_ONLY)
  upTracker = new ButtonPressTracker(UP);
  downTracker = new ButtonPressTracker(DOWN);
  leftTracker = new ButtonPressTracker(LEFT);
  rightTracker = new ButtonPressTracker(RIGHT);
#else 
  xTracker = new JoystickPressTracker(VRX);
  yTracker = new JoystickPressTracker(VRY);
  xyTracker = new Joystick2DTracker(*xTracker, *yTracker);
#endif
  swTracker = new ButtonPressTracker(SW);
}

#if !defined(BUTTONS_ONLY)
int minXPos = INT_MAX;
int maxXPos = -1;
int minYPos = INT_MAX;
int maxYPos = -1;
#endif
long lastShowMillis = 0;

void loop()
{
#if defined(BUTTONS_ONLY)
  bool upPressed = upTracker->checkPressed(XYPressAutoRepeatMillis);
  bool downPressed = downTracker->checkPressed(XYPressAutoRepeatMillis);  
  bool leftPressed = leftTracker->checkPressed(XYPressAutoRepeatMillis);
  bool rightPressed = rightTracker->checkPressed(XYPressAutoRepeatMillis);  
  int xPos = 512;
  int yPos = 512;
  int xPressed = 0;
  int yPressed = 0;
  if (upPressed) {
    yPos = 0;
    yPressed = -1;
  } 
  if (downPressed) {
    yPos = 1023;
    yPressed = 1;
  }
  if (leftPressed) {
    xPos = 0;
    xPressed = -1;
  } 
  if (rightPressed) {
    xPos = 1023;
    xPressed = 1;
  }
#else
  int xPos = analogRead(VRX);
  int yPos = analogRead(VRY);
  if (xPos < minXPos)
  {
    minXPos = xPos;
  }
  if (xPos > maxXPos)
  {
    maxXPos = xPos;
  }
  if (yPos < minYPos)
  {
    minYPos = yPos;
  }
  if (yPos > maxYPos)
  {
    maxYPos = yPos;
  }
  if ((maxXPos - minXPos) > 800)
  {
    xTracker->setMinMax(minXPos + XYPressThreshold, maxXPos - XYPressThreshold);
  }
  if ((maxYPos - minYPos) > 800)
  {
    yTracker->setMinMax(minYPos + XYPressThreshold, maxYPos - XYPressThreshold);
  }
  int xPressed = 0;
  int yPressed = 0;
  if (xyTracker->checkPressed())
  {
    xPressed = xyTracker->checkResultXPressed;
    yPressed = xyTracker->checkResultYPressed;
  }
#endif

  bool swPressed = swTracker->checkPressed(XYPressAutoRepeatMillis);

  long nowMillis = millis();
  bool show = (nowMillis - lastShowMillis) >= 1000 || xPressed != 0 || yPressed != 0 || swPressed;

  int swState = digitalRead(SW);

  if (show)
  {
    Serial.print("[ ");
    if (swPressed)
    {
      Serial.print("#");
    }
    else
    {
      const char *pressedChar = JoystickPressedToStr(xPressed, yPressed);
      Serial.print(pressedChar == NULL ? "." : pressedChar);
    }
    Serial.print(" ] -- ");
    if (!show)
    {
      Serial.println();
    }
  }

  if (show)
  {
    Serial.print("X: ");
    Serial.print(xPos);
#if !defined(BUTTONS_ONLY)
    Serial.print(" (");
    Serial.print(minXPos);
    Serial.print("-");
    Serial.print(maxXPos);
    Serial.print(")");
#endif
    Serial.print(" / Y: ");
    Serial.print(yPos);
#if !defined(BUTTONS_ONLY)
    Serial.print(" (");
    Serial.print(minYPos);
    Serial.print("-");
    Serial.print(maxYPos);
    Serial.print(")");
#endif
    Serial.print(" / BTN: ");
    Serial.println(swState);

    lastShowMillis = nowMillis;
  }
}
