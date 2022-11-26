// ***
// * joystick press better if min / max detected (by moving the joystick)
// ***

//#include <limits.h>

// #if defined(ARDUINO_AVR_NANO)
// #define VRX A2
// #define VRY A1
// #define SW A0
// #elif defined(ESP8266)
// #define UP D7
// #define DOWN D6
// #define LEFT D5
// #define RIGHT D4
// #define SW D1
// #endif

// const int JoystickPressThreshold = 100;
const long JoystickPressAutoRepeatMillis = 0; // 0 means no auto repeat

struct JoystickPress
{
  int xPressed; // -1, 0 or 1
  int yPressed; // -1, 0 or 1
};

struct JoystickPressCode : JoystickPress
{
  bool swPressed;
};

class JoystickInterface
{
public:
  static const long BlackOutMillis = 50;

protected:
  JoystickInterface()
  {
    lastCheckJoystickPress.xPressed = 0;
    lastCheckJoystickPress.yPressed = 0;
    lastJoystickPressMillis = 0;
  }

public:
  const JoystickPress *checkJoystickPress(int repeat = 0)
  {
    int xPressed = _checkPressedX(repeat);
    int yPressed = _checkPressedY(repeat);
    if (xPressed != 0 || yPressed != 0)
    {
      // Serial.print(xPressed);
      // Serial.print("/");
      // Serial.println(yPressed);
      long nowMillis = millis();
      long diffMillis = nowMillis - this->lastJoystickPressMillis;
      // Serial.println(diffMillis);
      // delay(200);
      if (diffMillis >= BlackOutMillis /*50*/)
      {
        lastCheckJoystickPress.xPressed = 0;
        lastCheckJoystickPress.yPressed = 0;
        bool pressedA = _checkPressedBypass('A');
        bool pressedB = _checkPressedBypass('B');
        bool pressedC = _checkPressedBypass('C');
        bool pressedD = _checkPressedBypass('D');
        if (pressedB)
        {
          // Serial.println("B");
          lastCheckJoystickPress.xPressed = 1;
        }
        else if (pressedD)
        {
          // Serial.println("D");
          lastCheckJoystickPress.xPressed = -1;
        }
        if (pressedA)
        {
          // Serial.println("A");
          lastCheckJoystickPress.yPressed = -1;
        }
        else if (pressedC)
        {
          // Serial.println("C");
          lastCheckJoystickPress.yPressed = 1;
        }
        lastJoystickPressMillis = nowMillis;
        return &lastCheckJoystickPress;
      }
    }
    return NULL;
  }
  inline bool checkSWPressed(int repeat = 0)
  {
    return checkButtonPressed('E', repeat);
  }
  inline bool checkAPressed(int repeat)
  {
    return checkButtonPressed('A', repeat);
  }
  inline bool checkBPressed(int repeat)
  {
    return checkButtonPressed('B', repeat);
  }
  inline bool checkCPressed(int repeat)
  {
    return checkButtonPressed('C', repeat);
  }
  inline bool checkDPressed(int repeat)
  {
    return checkButtonPressed('D', repeat);
  }

public:
  bool checkJoystickPressCode(JoystickPressCode &joystickPressCode, int repeat = 0)
  {
    joystickPressCode.xPressed = 0;
    joystickPressCode.yPressed = 0;
    joystickPressCode.swPressed = false;
    const JoystickPress *joystickPress = checkJoystickPress(repeat);
    bool swPressed = checkSWPressed(repeat);
    if (joystickPress != NULL || swPressed)
    {
      if (joystickPress != NULL)
      {
        joystickPressCode.xPressed = joystickPress->xPressed;
        joystickPressCode.yPressed = joystickPress->yPressed;
      }
      joystickPressCode.swPressed = swPressed;
      return true;
    }
    else
    {
      return false;
    }
  }

protected:
  /**
   * @param button can be 'A' to 'E'
   */
  inline bool checkButtonPressed(char button, int repeat)
  {
    return _checkPressed(button, repeat);
  }

protected:
  virtual int _checkPressedX(int repeat);
  virtual int _checkPressedY(int repeat);
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
    int reading = digitalRead(this->pin);
    return setPressed(reading == 0, repeat);
  }
  bool checkPressedBypass()
  {
    int reading = digitalRead(this->pin);
    setPressed(reading == 0);
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
        this->blackOutMillis = nowMillis + JoystickInterface::BlackOutMillis /*50*/;
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

class DecodedJoystick : public JoystickInterface
{
public:
  DecodedJoystick() : JoystickInterface()
  {
    joystickPressCodeValid = false;
  }

public:
  void decode(JoystickPressCode *joystickPressCode)
  {
    if (joystickPressCode != NULL)
    {
      this->joystickPressCode.xPressed = joystickPressCode->xPressed;
      this->joystickPressCode.yPressed = joystickPressCode->yPressed;
      this->joystickPressCode.swPressed = joystickPressCode->swPressed;
      this->joystickPressCodeValid = true;
    }
    else
    {
      this->joystickPressCodeValid = false;
    }
  }

private:
  JoystickPressCode joystickPressCode;
  bool joystickPressCodeValid;
};

class JoystickPressTracker
{
public:
  static const int DefAutoTuneThreshold = 50;
  static const int DefMinReading = DefAutoTuneThreshold;
  static const int DefMaxReading = 1023 - DefAutoTuneThreshold;

public:
  JoystickPressTracker(uint8_t pin, int minReading = DefMinReading, int maxReading = DefMaxReading)
  {
    this->pin = pin;
    this->autoTuneThreshold = -1;
    resetMinMax(minReading, maxReading, true);
  }
  /**
   * @param autoTuneThreshold -1 if no auto tune
   */
  JoystickPressTracker(uint8_t pin, bool reverseDir, int autoTuneThreshold = DefAutoTuneThreshold)
  {
    // int autoThreshold = autoTune ? 200 : -1;
    // int autoThreshold = autoTune ? 50 : -1;
    this->pin = pin;
    this->autoTuneThreshold = autoTuneThreshold;
    this->autoMin = 10000;
    this->autoMax = -1;
    int minReading = reverseDir ? DefMaxReading : DefMinReading;
    int maxReading = reverseDir ? DefMinReading : DefMaxReading;
    resetMinMax(minReading, maxReading, true);
  }

public:
  int8_t checkPressed(int repeat = 0)
  {
    int reading = analogRead(this->pin);
    if (this->autoTuneThreshold != -1)
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
        resetMinMax(this->autoMin + this->autoTuneThreshold, this->autoMax - this->autoTuneThreshold, false);
      }
    }
    return setReading(reading, repeat);
  }
  int checkPressedBypass()
  {
    int reading = analogRead(this->pin);
    setReading(reading);
    int pressed = readingToPressedDir(reading);
    //  Serial.print(reading);
    // Serial.print(">>");
    // Serial.println(pressed);
    return pressed;
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
      if (diffMillis >= JoystickInterface::BlackOutMillis)
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
        // Serial.print(reading);
        // Serial.print("=>");
        // Serial.println(oriPressedDir);
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
      // Serial.print(this->minReading);
      // Serial.print("-");
      // Serial.println(this->maxReading);
      // delay(100);
    }
  }

private:
  int maxReading;
  int minReading;
  bool reverseDir;
  int autoTuneThreshold; // -1 if not auto tune
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

class JoystickJoystick : public JoystickInterface
{
public:
  JoystickJoystick(JoystickPressTracker *xTracker, JoystickPressTracker *yTracker, ButtonPressTracker *swTracker) : JoystickInterface()
  {
    this->xTracker = xTracker;
    this->yTracker = yTracker;
    this->swTracker = swTracker;
  }

protected:
  virtual int _checkPressedX(int repeat)
  {
    return xTracker != NULL ? xTracker->checkPressed(repeat) : 0;
  }
  virtual int _checkPressedY(int repeat)
  {
    return yTracker != NULL ? yTracker->checkPressed(repeat) : 0;
  }
  virtual bool _checkPressed(char button, int repeat)
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
  virtual bool _checkPressedBypass(char button)
  {
    if (button == 'A')
    {
      // Serial.println(yTracker->checkPressedBypass());
      // delay(200);
      return yTracker != NULL && (yTracker->checkPressedBypass() == -1);
    }
    else if (button == 'B')
    {
      return xTracker != NULL && (xTracker->checkPressedBypass() == 1);
    }
    else if (button == 'C')
    {
      return yTracker != NULL && (yTracker->checkPressedBypass() == 1);
    }
    else if (button == 'D')
    {
      return xTracker != NULL && (xTracker->checkPressedBypass() == -1);
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

class ButtonJoystick : public JoystickInterface
{
public:
  ButtonJoystick(ButtonPressTracker *upTracker, ButtonPressTracker *rightTracker, ButtonPressTracker *downTracker, ButtonPressTracker *leftTracker, ButtonPressTracker *midTracker) : ButtonJoystick(upTracker, rightTracker, downTracker, leftTracker, midTracker, false)
  {
  }

protected:
  ButtonJoystick(ButtonPressTracker *upTracker, ButtonPressTracker *rightTracker, ButtonPressTracker *downTracker, ButtonPressTracker *leftTracker, ButtonPressTracker *midTracker, bool buttonsOnly) : JoystickInterface()
  {
    this->leftTracker = leftTracker;
    this->rightTracker = rightTracker;
    this->upTracker = upTracker;
    this->downTracker = downTracker;
    this->midTracker = midTracker;
    this->buttonsOnly = buttonsOnly;
  }

protected:
  virtual int _checkPressedX(int repeat)
  {
    // Serial.print("?");
    bool pressedB = _checkPressed('B', repeat);
    bool pressedD = _checkPressed('D', repeat);
    if (pressedB)
    {
      // Serial.println("*B");
      return -1;
    }
    else if (pressedD)
    {
      // Serial.println("*D");
      return 1;
    }
    else
    {
      return 0;
    }
  }
  virtual int _checkPressedY(int repeat)
  {
    bool pressedA = _checkPressed('A', repeat);
    bool pressedC = _checkPressed('C', repeat);
    if (pressedA)
    {
      // Serial.println("*A");
      return -1;
    }
    else if (pressedC)
    {
      // Serial.println("*C");
      return 1;
    }
    else
    {
      return 0;
    }
  }
  virtual bool _checkPressed(char button, int repeat)
  {
    if (button == 'A')
    {
      return upTracker != NULL && upTracker->checkPressed(repeat);
    }
    else if (button == 'B')
    {
      return rightTracker != NULL && rightTracker->checkPressed(repeat);
    }
    else if (button == 'C')
    {
      return downTracker != NULL && downTracker->checkPressed(repeat);
    }
    else if (button == 'D')
    {
      return leftTracker != NULL && leftTracker->checkPressed(repeat);
    }
    else if (button == 'E')
    {
      return midTracker != NULL && midTracker->checkPressed(repeat);
    }
    else
    {
      return false;
    }
  }
  virtual bool _checkPressedBypass(char button)
  {
    if (button == 'A')
    {
      return upTracker != NULL && upTracker->checkPressedBypass();
    }
    else if (button == 'B')
    {
      return rightTracker != NULL && rightTracker->checkPressedBypass();
    }
    else if (button == 'C')
    {
      return downTracker != NULL && downTracker->checkPressedBypass();
    }
    else if (button == 'D')
    {
      return leftTracker != NULL && leftTracker->checkPressedBypass();
    }
    else if (button == 'E')
    {
      return midTracker != NULL && midTracker->checkPressedBypass();
    }
    else
    {
      return false;
    }
  }

private:
  ButtonPressTracker *leftTracker;
  ButtonPressTracker *rightTracker;
  ButtonPressTracker *upTracker;
  ButtonPressTracker *downTracker;
  ButtonPressTracker *midTracker;
  bool buttonsOnly;
};

class ButtonsOnly : public ButtonJoystick
{
public:
  ButtonsOnly(ButtonPressTracker *aTracker, ButtonPressTracker *bTracker, ButtonPressTracker *cTracker, ButtonPressTracker *dTracker) : ButtonJoystick(aTracker, bTracker, cTracker, dTracker, NULL, true)
  {
  }
  // public:
  //   inline bool checkButtonAPressed(int repeat) {
  //     return checkButtonPressed('A', repeat);
  //   }
  //   inline bool checkButtonBPressed(int repeat) {
  //     return checkButtonPressed('B', repeat);
  //   }
  //   inline bool checkButtonCPressed(int repeat) {
  //     return checkButtonPressed('C', repeat);
  //   }
  //   inline bool checkButtonDPressed(int repeat) {
  //     return checkButtonPressed('D', repeat);
  //   }
};

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
ButtonsOnly *buttons = new ButtonsOnly(aTracker, bTracker, cTracker, dTracker;
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
    swPresses[i] = Joysticks[i]->checkSWPressed();
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
