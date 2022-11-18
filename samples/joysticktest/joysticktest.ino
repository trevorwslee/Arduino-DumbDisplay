// ***
// * joystick press better if min / max detected (by moving the joystick)
// ***

#include <limits.h>

#define TRACK_PRESS
#define TRACK_PRESSED_IS_2D


#if defined(ARDUINO_AVR_NANO)
const int VRX = A2;
const int VRY = A1;
const int SW = A0;
#elif defined(PICO_SDK_VERSION_MAJOR)
const int VRX = 26;
const int VRY = 27;
const int SW = 16;
#endif

const int XYPressThreshold = 100;
bool trackedXYPressed2D = true;
const long XYPressAutoRepeatMillis = 0; // 0 means no auto repeat

#if defined(TRACK_PRESS)
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
    // if (reading <= this->minReading)
    // {
    //   if (this->reverseDir)
    //   {
    //     this->pressedDir = 1;
    //   }
    //   else
    //   {
    //     this->pressedDir = -1;
    //   }
    // }
    // else if (reading >= this->maxReading)
    // {
    //   if (this->reverseDir)
    //   {
    //     this->pressedDir = -1;
    //   }
    //   else
    //   {
    //     this->pressedDir = 1;
    //   }
    // }
    // else
    // {
    //   this->pressedDir = 0;
    //   this->nextRepeatMillis = 0;
    //   this->autoRepeatDir = 0;
    // }
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

ButtonPressTracker *btnTracker = NULL;
JoystickPressTracker *xTracker = NULL;
JoystickPressTracker *yTracker = NULL;
Joystick2DTracker *xyTracker = NULL;
#endif

void setup()
{
  Serial.begin(115200);

  if (VRX != -1)
  {
    pinMode(VRX, INPUT);
  }
  if (VRY != -1)
  {
    pinMode(VRY, INPUT);
  }
  if (SW != -1)
  {
    pinMode(SW, INPUT_PULLUP);
  }

#if defined(TRACK_PRESS)
  if (VRX != -1)
  {
    xTracker = new JoystickPressTracker(VRX);
  }
  if (VRY != -1)
  {
    yTracker = new JoystickPressTracker(VRY);
  }
#if defined(TRACK_PRESSED_IS_2D)
  if (xTracker != NULL && yTracker != NULL)
  {
    xyTracker = new Joystick2DTracker(*xTracker, *yTracker);
  }
#endif
  if (SW != -1)
  {
    btnTracker = new ButtonPressTracker(SW);
  }
#endif
}

int minXPos = INT_MAX;
int maxXPos = -1;
int minYPos = INT_MAX;
int maxYPos = -1;
long lastShowMillis = 0;

void loop()
{
  int xPos = 0;
  if (VRX != -1)
  {
    xPos = analogRead(VRX);
  }
  int yPos = 0;
  if (VRY != -1)
  {
    yPos = analogRead(VRY);
  }
  int btnState = 0;
   if (SW != -1)
  {
    btnState = digitalRead(SW);
  }
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

  long nowMillis = millis();
  bool show = (nowMillis - lastShowMillis) >= 500;

#if defined(TRACK_PRESS)
  if ((maxXPos - minXPos) > 800)
  {
    xTracker->setMinMax(minXPos + XYPressThreshold, maxXPos - XYPressThreshold);
  }
  if ((maxYPos - minYPos) > 800)
  {
    yTracker->setMinMax(minYPos + XYPressThreshold, maxYPos - XYPressThreshold);
  }
  bool btnPressed = btnTracker != NULL && btnTracker->checkPressed(XYPressAutoRepeatMillis);
  if (btnPressed)
  {
    show = true;
  }
  int xPressed = 0;
  int yPressed = 0;
  if (xyTracker != NULL)
  {
    if (xyTracker->checkPressed())
    {
      xPressed = xyTracker->checkResultXPressed;
      yPressed = xyTracker->checkResultYPressed;
      show = true;
    }
  }
  else
  {
    xPressed = xTracker->checkPressed(XYPressAutoRepeatMillis);
    yPressed = yTracker->checkPressed(XYPressAutoRepeatMillis);
    if (xPressed || yPressed)
    {
      show = true;
    }
  }
  if (show)
  {
    Serial.print("[ ");
    if (btnPressed)
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
#endif

  if (show)
  {
    Serial.print("X: ");
    Serial.print(xPos);
    Serial.print(" (");
    Serial.print(minXPos);
    Serial.print("-");
    Serial.print(maxXPos);
    Serial.print(")");
    Serial.print(" / Y: ");
    Serial.print(yPos);
    Serial.print(" (");
    Serial.print(minYPos);
    Serial.print("-");
    Serial.print(maxYPos);
    Serial.print(")");
    Serial.print(" / BTN: ");
    Serial.println(btnState);

    lastShowMillis = nowMillis;
  }
}
