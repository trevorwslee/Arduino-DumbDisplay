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
    int autoThreshold = autoTune ? 10 : -1;
    this->pin = pin;
    this->autoThreshold = autoThreshold;
    this->autoMin = 10000;
    this->autoMax = -1;
    int minReading = reverseDir ? 1013 : 10;
    int maxReading = reverseDir ? 10 : 1013;
    resetMinMax(minReading, maxReading, true);
  }

public:
  // void setMinMax(int minReading, int maxReading)
  // {
  //   resetMinMax(minReading, maxReading, false);
  // }

public:
  int8_t checkPressed(int repeat = 0)
  {
    int reading = analogRead(this->pin);
    if (this->autoThreshold != -1) {
      if (reading < this->autoMin) {
        this->autoMin = reading;
      }
      if (reading > this->autoMax) {
        this->autoMax = reading;
      }
      if ((this->autoMax - this->autoMin) >= 800) {
        resetMinMax(this->autoMin + this->autoThreshold, this->autoMax - this->autoThreshold, false);
      }
    }
    return setReading(reading, repeat);
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
  void resetMinMax(int minReading, int maxReading, bool forceReset)
  {
    if (forceReset || this->minReading != minReading || this->maxReading != maxReading)
    {
      if (forceReset) { 
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
