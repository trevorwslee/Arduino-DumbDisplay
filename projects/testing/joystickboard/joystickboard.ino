// ***
// * joystick press better if min / max detected (by moving the joystick)
// ***

#include <limits.h>

#if defined(ARDUINO_AVR_NANO)
#define VRX A2
#define VRY A1
#define SW A0
#elif defined(ESP8266)
#define BUTTONS_ONLY
#define UP D7
#define DOWN D6
#define LEFT D5
#define RIGHT D4
#define SW D1
#endif

const int JoystickPressThreshold = 100;
const long JoystickPressAutoRepeatMillis = 200; // 0 means no auto repeat

struct JoystickPress
{
  int pressedX; // -1, 0 or 1
  int pressedY; // -1, 0 or 1
};

class JoystickInterface
{
protected:
  JoystickInterface() {}

public:
  const JoystickPress *checkJoystickPress(int repeat = 0)
  {
    int xPressed = _checkXPressed(repeat);
    int yPressed = _checkYPressed(repeat);
    if (xPressed != 0 || yPressed != 0)
    {
      long nowMillis = millis();
      long diffMillis = nowMillis - this->lastJoystickPressMillis;
      if (diffMillis > 50)
      {
        bool pressedA = _checkPressedBypass('A');
        bool pressedB = _checkPressedBypass('B');
        bool pressedC = _checkPressedBypass('C');
        bool pressedD = _checkPressedBypass('D');
        if (pressedB)
        {
          lastCheckJoystickPress.pressedX = 1;
        }
        else if (pressedD)
        {
          lastCheckJoystickPress.pressedX = -1;
        }
        if (pressedA)
        {
          lastCheckJoystickPress.pressedY = -1;
        }
        else if (pressedC)
        {
          lastCheckJoystickPress.pressedY = -1;
        }
        return &lastCheckJoystickPress;
      }
    }
    else
    {
      return NULL;
    }
  }
  /**
   * @param button can be 'A' to 'E'
   */
  bool checkButtonPressed(char button, int repeat = 0)
  {
    return _checkPressed(button, repeat);
  }

protected:
  virtual int _checkXPressed(int repeat);
  virtual int _checkYPressed(int repeat);
  virtual bool _checkPressed(char button, int repeat);
  virtual bool _checkPressedBypass(char button);

private:
  JoystickPress lastCheckJoystickPress;
  long lastJoystickPressMillis;
};

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
    this->autoThreshold = -1;
    resetMinMax(minReading, maxReading, true);
  }
  JoystickPressTracker(uint8_t pin, bool reverseDir, bool autoTune)
  {
    int autoThreshold = autoTune ? 200 : -1;
    this->pin = pin;
    this->autoThreshold = autoThreshold;
    this->autoMin = 10000;
    this->autoMax = -1;
    int minReading = reverseDir ? 1013 : 10;
    int maxReading = reverseDir ? 10 : 1013;
    resetMinMax(minReading, maxReading, true);
  }

public:
  int8_t checkPressed(int repeat = 0)
  {
    int reading = analogRead(this->pin);
    if (this->autoThreshold != -1)
    {
      if (reading < this->autoMin)
      {
        this->autoMin = reading;
      }
      if (reading > this->autoMax)
      {
        this->autoMax = reading;
      }
      if ((this->autoMax - this->autoMin) >= 800)
      {
        resetMinMax(this->autoMin + this->autoThreshold, this->autoMax - this->autoThreshold, false);
      }
    }
    return setReading(reading, repeat);
  }
  int checkPressedBypass()
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
  void resetMinMax(int minReading, int maxReading, bool forceReset)
  {
    if (forceReset || this->minReading != minReading || this->maxReading != maxReading)
    {
      if (forceReset)
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

private:
  int maxReading;
  int minReading;
  bool reverseDir;
  int autoThreshold; // -1 if not auto tune
  int autoMin;
  int autoMax;

private:
  uint8_t pin;
  int pressedDir;
  long pressedMillis;
  bool needReset;
  long nextRepeatMillis;
  int autoRepeatDir;
};

class JoystickJoystick : JoystickInterface
{
public:
  JoystickJoystick(JoystickPressTracker *xTracker, JoystickPressTracker *yTracker, ButtonPressTracker *swTracker) : JoystickInterface()
  {
    this->xTracker = xTracker;
    this->yTracker = yTracker;
    this->swTracker = swTracker;
  }

protected:
  bool _checkPressed(char button, int repeat)
  {
    if (button == 'A')
    {
      return yTracker != NULL && (yTracker->checkPressed(repeat) == -1);
    }
    else if (button == 'B')
    {
      return xTracker != NULL && (xTracker->checkPressed(repeat) == 1);
    }
    else if (button == 'C')
    {
      return yTracker != NULL && (yTracker->checkPressed(repeat) == -1);
    }
    else if (button == 'D')
    {
      return xTracker != NULL && (xTracker->checkPressed(repeat) == 1);
    }
    else if (button == 'E')
    {
      return swTracker != NULL && swTracker->checkPressed(repeat);
    }
    else
    {
      return false;
    }
  }
  bool _checkPressedBypass(char button)
  {
    if (button == 'A')
    {
      return yTracker != NULL && (yTracker->checkPressedBypass() == -1);
    }
    else if (button == 'B')
    {
      return xTracker != NULL && (xTracker->checkPressedBypass() == 1);
    }
    else if (button == 'C')
    {
      return yTracker != NULL && (yTracker->checkPressedBypass() == -1);
    }
    else if (button == 'D')
    {
      return xTracker != NULL && (xTracker->checkPressedBypass() == 1);
    }
    else if (button == 'E')
    {
      return swTracker != NULL && swTracker->checkPressedBypass();
    }
    else
    {
      return false;
    }
  }

private:
  JoystickPressTracker *xTracker;
  JoystickPressTracker *yTracker;
  ButtonPressTracker *swTracker;
};

void setup()
{
}
void loop()
{
}
