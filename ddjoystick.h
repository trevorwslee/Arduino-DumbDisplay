

#ifndef ddjoystick_h
#define ddjoystick_h


/// Struct used by the helper class like JoystickInterface
struct JoystickPress
{
  int xPressed; // -1, 0 or 1
  int yPressed; // -1, 0 or 1
};

/// Struct used by the helper class like JoystickInterface
struct ABCDPressed
{
  bool aPressed;
  bool bPressed;
  bool cPressed;
  bool dPressed;
};


/// Struct used by the helper class like JoystickInterface
struct JoystickPressCode
{
  int xPressed;
  int yPressed;
  bool swPressed;
};

/// Base Helper class for joystick input tracking. For an example use, you may want to refer to [Wireless Joystick Module Experiment With ESP8266 on ESP-Now](https://www.instructables.com/Wireless-Joystick-Module-Experiment-With-ESP8266-o/)
class JoystickInterface
{
public:
  static const long BlackOutMillis = 50;

protected:
  JoystickInterface(bool buttonsOnly)
  {
    this->buttonsOnly = buttonsOnly;
    // this->lastCheckJoystickPress.xPressed = 0;
    // this->lastCheckJoystickPress.yPressed = 0;
    this->lastCheckMillis = 0;
    // this->lastCheckABCDPressed.aPressed = false;
    // this->lastCheckABCDPressed.bPressed = false;
    // this->lastCheckABCDPressed.dPressed = false;
    // this->lastCheckABCDPressed.cPressed = false;
  }

public:
  const JoystickPress *checkJoystickPress(int repeat = 0)
  {
    int xPressed = _checkPressedX(repeat, false);
    int yPressed = _checkPressedY(repeat, false);
    if (xPressed != 0 || yPressed != 0)
    {
      // Serial.print(xPressed);
      // Serial.print("/");
      // Serial.println(yPressed);
      long nowMillis = millis();
      long diffMillis = nowMillis - this->lastCheckMillis;
      // Serial.println(diffMillis);
      // delay(200);
      if (diffMillis >= BlackOutMillis)
      {
        lastCheckJoystickPress.xPressed = 0;
        lastCheckJoystickPress.yPressed = 0;
        bool pressedA = _checkPressedBypass('A') || yPressed == -1;
        bool pressedB = _checkPressedBypass('B') || xPressed == 1;
        bool pressedC = _checkPressedBypass('C') || yPressed == 1;
        bool pressedD = _checkPressedBypass('D') || xPressed == -1;
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
        lastCheckMillis = nowMillis;
        return &lastCheckJoystickPress;
      }
    }
    return NULL;
  }
  inline bool checkSWPressed(int repeat = 0)
  {
    return _checkPressed('E', repeat);
  }
  inline bool forButtonsOnly() const {
    return this->buttonsOnly;
  }
  const ABCDPressed *checkABCDPressed(int repeat = 0)
  {
    int aPressed = _checkPressed('A', repeat);
    int bPressed = _checkPressed('B', repeat);
    int cPressed = _checkPressed('C', repeat);
    int dPressed = _checkPressed('D', repeat);
    if (aPressed || bPressed || cPressed || dPressed)
    {
      long nowMillis = millis();
      long diffMillis = nowMillis - this->lastCheckMillis;
      if (diffMillis >= BlackOutMillis)
      {
        //delay(200);  // delay a bit
        this->lastCheckABCDPressed.aPressed = _checkPressedBypass('A') || aPressed;
        this->lastCheckABCDPressed.bPressed = _checkPressedBypass('B') || bPressed;
        this->lastCheckABCDPressed.cPressed = _checkPressedBypass('C') || cPressed;
        this->lastCheckABCDPressed.dPressed = _checkPressedBypass('D') || dPressed;
        lastCheckMillis = nowMillis;
        return &lastCheckABCDPressed;
      }
    }
    return NULL;
  }
  inline bool checkAPressed(int repeat = 0)
  {
    return _checkPressed('A', repeat);
  }
  inline bool checkBPressed(int repeat = 0)
  {
    return _checkPressed('B', repeat);
  }
  inline bool checkCPressed(int repeat = 0)
  {
    return _checkPressed('C', repeat);
  }
  inline bool checkDPressed(int repeat = 0)
  {
    return _checkPressed('D', repeat);
  }

public:
  bool checkJoystickPressCode(JoystickPressCode &joystickPressCode, int repeat = 0)
  {
    joystickPressCode.xPressed = _checkPressedX(repeat, true);
    joystickPressCode.yPressed = _checkPressedY(repeat, true);
    joystickPressCode.swPressed = _checkPressed('E', repeat);
    return joystickPressCode.xPressed != 0 || joystickPressCode.yPressed != 0 || joystickPressCode.swPressed;
    // joystickPressCode.xPressed = 0;
    // joystickPressCode.yPressed = 0;
    // joystickPressCode.swPressed = false;
    // const JoystickPress *joystickPress = checkJoystickPress(repeat);
    // bool swPressed = checkSWPressed(repeat);
    // if (joystickPress != NULL || swPressed)
    // {
    //   if (joystickPress != NULL)
    //   {
    //     joystickPressCode.xPressed = joystickPress->xPressed;
    //     joystickPressCode.yPressed = joystickPress->yPressed;
    //   }
    //   joystickPressCode.swPressed = swPressed;
    //   return true;
    // }
    // else
    // {
    //   return false;
    // }
  }

protected:
  ///
  /// @param button can be 'A' to 'E'
  ///
  inline bool _checkPressed(char button, int repeat)
  {
    return _checkPressed(button, repeat, false);
  }
  inline bool _checkPressedBypass(char button) {
    return _checkPressed(button, 0, true);
  }

protected:
  virtual int _checkPressedX(int repeat, bool raw);
  virtual int _checkPressedY(int repeat, bool raw);
  virtual bool _checkPressed(char button, int repeat, bool bypass);

private:
  bool buttonsOnly;
  JoystickPress lastCheckJoystickPress;
  ABCDPressed lastCheckABCDPressed;
  long lastCheckMillis;
};

/// Helper class for joystick input tracking. See JoystickInterface.
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
    if (true) {
    } else {
      setPressed(reading == 0);
    }
    return pressed;
  }
  inline bool checkPressed(int repeat, bool bypass) {
    if (bypass) {
      return checkPressedBypass();
    } else {
      return checkPressed(repeat);
    }
  }
//   inline bool debug_checkPressed(int repeat, bool bypass, char button) {
//     if (bypass) {
// //Serial.println("BYPASS");
//       return checkPressedBypass();
//     } else {
// // Serial.print(button);
// // Serial.print(">");      
// // Serial.println(repeat);
//       return checkPressed(repeat);
//     }
//     //delay(50);
//   }


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


/// Helper class for joystick input tracking. See JoystickInterface.
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
  ///
  /// @param autoTuneThreshold -1 if no auto tune
  ///
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
    if (true) {
    } else {
      setReading(reading);
    }
    int pressed = readingToPressedDir(reading);
    //  Serial.print(reading);
    // Serial.print(">>");
    // Serial.println(pressed);
    return pressed;
  }
  int8_t checkPressed(int repeat, bool bypass) {
    if (bypass) {
      return checkPressedBypass();
    } else {
      return checkPressed(repeat);
    }
  }
  inline int one() {
    return this->reverseDir ? -1 : 1;
  }
  inline int minusOne() {
    return this->reverseDir ? 1 : -1;
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
// Serial.println(reading);    
// delay(100);
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
        // Serial.print(repeat);
        // Serial.print(":");
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

/// Helper class for joystick input tracking. See JoystickInterface.
class JoystickJoystick : public JoystickInterface
{
public:
  JoystickJoystick(JoystickPressTracker *xTracker, JoystickPressTracker *yTracker, ButtonPressTracker *swTracker) : JoystickInterface(false)
  {
    this->xTracker = xTracker;
    this->yTracker = yTracker;
    this->swTracker = swTracker;
  }

protected:
  virtual int _checkPressedX(int repeat, bool raw)
  {
    // Serial.println(repeat);
    // delay(100);
    int pressed = xTracker != NULL ? xTracker->checkPressed(repeat) : 0;
    if (pressed != 0) {
      // Serial.print("%X=");
      // Serial.println(pressed);
    }
    return pressed;
  }
  virtual int _checkPressedY(int repeat, bool raw)
  {
    return yTracker != NULL ? yTracker->checkPressed(repeat) : 0;
  }
  virtual bool _checkPressed(char button, int repeat, bool bypass)
  {
    if (button == 'A')
    {
      return yTracker != NULL && (yTracker->checkPressed(repeat, bypass) == -1);
    }
    else if (button == 'B')
    {
      return xTracker != NULL && (xTracker->checkPressed(repeat, bypass) == 1);
    }
    else if (button == 'C')
    {
      return yTracker != NULL && (yTracker->checkPressed(repeat, bypass) == 1);
    }
    else if (button == 'D')
    {
      return xTracker != NULL && (xTracker->checkPressed(repeat, bypass) == -1);
    }
    else if (button == 'E')
    {
      bool pressed = swTracker != NULL && swTracker->checkPressed(repeat, bypass);
//if (pressed && repeat != 0 && !bypass) { Serial.print(repeat); Serial.println(" <SW>"); }
      return pressed;
    }
    else
    {
      return false;
    }
  }
  // virtual bool _checkPressedBypass(char button)
  // {
  //   if (button == 'A')
  //   {
  //     // Serial.println(yTracker->checkPressedBypass());
  //     // delay(200);
  //     return yTracker != NULL && (yTracker->checkPressedBypass() == -1);
  //   }
  //   else if (button == 'B')
  //   {
  //     return xTracker != NULL && (xTracker->checkPressedBypass() == 1);
  //   }
  //   else if (button == 'C')
  //   {
  //     return yTracker != NULL && (yTracker->checkPressedBypass() == 1);
  //   }
  //   else if (button == 'D')
  //   {
  //     return xTracker != NULL && (xTracker->checkPressedBypass() == -1);
  //   }
  //   else if (button == 'E')
  //   {
  //     return swTracker != NULL && swTracker->checkPressedBypass();
  //   }
  //   else
  //   {
  //     return false;
  //   }
  // }

private:
  JoystickPressTracker *xTracker;
  JoystickPressTracker *yTracker;
  ButtonPressTracker *swTracker;
};

/// Helper class for joystick input tracking. See JoystickInterface.
class ButtonJoystickBasic : public JoystickInterface
{
protected:
  ButtonJoystickBasic(bool buttonsOnly) : JoystickInterface(buttonsOnly)
  {
  }

protected:
  virtual int _checkPressedX(int repeat, bool raw)
  {
//if (!raw) { Serial.print(repeat); Serial.println(); delay(100); }
    bool pressedB = _checkPressed('B', repeat);
    bool pressedD = _checkPressed('D', repeat);
    if (pressedB)
    {
//Serial.println("*B");
      return raw && pressedD ? 2 : 1;
    }
    else if (pressedD)
    {
//Serial.println("*D");
      return -1;
    }
    else
    {
      return 0;
    }
  }
  virtual int _checkPressedY(int repeat, bool raw)
  {
    bool pressedA = _checkPressed('A', repeat);
    bool pressedC = _checkPressed('C', repeat);
    if (pressedA)
    {
//Serial.println("*A");
      return raw && pressedC ? 2 : -1;
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
  // virtual bool _checkPressed(char button, int repeat)
  // {
  //   if (button == 'A')
  //   {
  //     return upTracker != NULL && upTracker->checkPressed(repeat);
  //   }
  //   else if (button == 'B')
  //   {
  //     return rightTracker != NULL && rightTracker->checkPressed(repeat);
  //   }
  //   else if (button == 'C')
  //   {
  //     return downTracker != NULL && downTracker->checkPressed(repeat);
  //   }
  //   else if (button == 'D')
  //   {
  //     return leftTracker != NULL && leftTracker->checkPressed(repeat);
  //   }
  //   else if (button == 'E')
  //   {
  //     return midTracker != NULL && midTracker->checkPressed(repeat);
  //   }
  //   else
  //   {
  //     return false;
  //   }
  // }
  // virtual bool _checkPressedBypass(char button)
  // {
  //   if (button == 'A')
  //   {
  //     return upTracker != NULL && upTracker->checkPressedBypass();
  //   }
  //   else if (button == 'B')
  //   {
  //     return rightTracker != NULL && rightTracker->checkPressedBypass();
  //   }
  //   else if (button == 'C')
  //   {
  //     return downTracker != NULL && downTracker->checkPressedBypass();
  //   }
  //   else if (button == 'D')
  //   {
  //     return leftTracker != NULL && leftTracker->checkPressedBypass();
  //   }
  //   else if (button == 'E')
  //   {
  //     return midTracker != NULL && midTracker->checkPressedBypass();
  //   }
  //   else
  //   {
  //     return false;
  //   }
  // }

};


/// Helper class for joystick input tracking. See JoystickInterface.
class DecodedJoystick : public ButtonJoystickBasic
{
public:
  DecodedJoystick(bool buttonsOnly) : ButtonJoystickBasic(buttonsOnly)
  {
    aValid = false;
    bValid = false;
    cValid = false;
    dValid = false;
    eValid = false;
  }

public:
  void decode(JoystickPressCode &joystickPressCode)
  {
    this->joystickPressCode.xPressed = joystickPressCode.xPressed;
    this->joystickPressCode.yPressed = joystickPressCode.yPressed;
    this->joystickPressCode.swPressed = joystickPressCode.swPressed;
    this->aValid = true;
    this->bValid = true;
    this->cValid = true;
    this->dValid = true;
    this->eValid = true;
  }

protected:
  virtual bool _checkPressed(char button, int repeat, bool bypass)
  {
    bool res = false;
    if (button == 'A')
    {
      if (aValid || bypass)
      {
        res = joystickPressCode.yPressed == -1 || joystickPressCode.yPressed == 2;
        aValid = false;
      }
    }
    else if (button == 'B')
    {
      if (bValid || bypass)
      {
        res = joystickPressCode.xPressed == 1 || joystickPressCode.xPressed == 2;
        bValid = false;
      }
    }
    else if (button == 'C')
    {
      if (cValid || bypass)
      {
        res = joystickPressCode.yPressed == 1 || joystickPressCode.yPressed == 2;
        cValid = false;
      }
    }
    else if (button == 'D')
    {
      if (dValid || bypass)
      {
        res = joystickPressCode.xPressed == -1 || joystickPressCode.xPressed == 2;
        dValid = false;
      }
    }
    else if (button == 'E')
    {
      if (eValid || bypass)
      {
        res = joystickPressCode.swPressed;
        eValid = false;
      }
    }
    return res;
  }

private:
  JoystickPressCode joystickPressCode;
  bool aValid;
  bool bValid;
  bool cValid;
  bool dValid;
  bool eValid;
};


/// Helper class for joystick input tracking. See JoystickInterface.
class ButtonJoystick : public ButtonJoystickBasic
{
public:
  ButtonJoystick(ButtonPressTracker *upTracker, ButtonPressTracker *rightTracker, ButtonPressTracker *downTracker, ButtonPressTracker *leftTracker, ButtonPressTracker *midTracker) : ButtonJoystick(upTracker, rightTracker, downTracker, leftTracker, midTracker, false)
  {
  }

protected:
  ButtonJoystick(ButtonPressTracker *upTracker, ButtonPressTracker *rightTracker, ButtonPressTracker *downTracker, ButtonPressTracker *leftTracker, ButtonPressTracker *midTracker, bool buttonsOnly) : ButtonJoystickBasic(buttonsOnly)
  {
    this->leftTracker = leftTracker;
    this->rightTracker = rightTracker;
    this->upTracker = upTracker;
    this->downTracker = downTracker;
    this->midTracker = midTracker;
    //this->buttonsOnly = buttonsOnly;
  }

protected:
  // virtual int _checkPressedX(int repeat, bool raw)
  // {
  //   // Serial.print("?");
  //   bool pressedB = _checkPressed('B', repeat);
  //   bool pressedD = _checkPressed('D', repeat);
  //   if (pressedB)
  //   {
  //     // Serial.println("*B");
  //     return raw && pressedD ? 2 : -1;
  //   }
  //   else if (pressedD)
  //   {
  //     // Serial.println("*D");
  //     return 1;
  //   }
  //   else
  //   {
  //     return 0;
  //   }
  // }
  // virtual int _checkPressedY(int repeat, bool raw)
  // {
  //   bool pressedA = _checkPressed('A', repeat);
  //   bool pressedC = _checkPressed('C', repeat);
  //   if (pressedA)
  //   {
  //     // Serial.println("*A");
  //     return raw && pressedC ? 2 : -1;
  //   }
  //   else if (pressedC)
  //   {
  //     // Serial.println("*C");
  //     return 1;
  //   }
  //   else
  //   {
  //     return 0;
  //   }
  // }
  virtual bool _checkPressed(char button, int repeat, bool bypass)
  {
    if (button == 'A')
    {
      bool pressed = upTracker != NULL && upTracker->checkPressed(repeat, bypass);
//if (pressed && repeat != 0 && !bypass) { Serial.print(repeat); Serial.println(" <A>"); }
      return pressed;
    }
    else if (button == 'B')
    {
      return rightTracker != NULL && rightTracker->checkPressed(repeat, bypass);
    }
    else if (button == 'C')
    {
      return downTracker != NULL && downTracker->checkPressed(repeat, bypass);
    }
    else if (button == 'D')
    {
      return leftTracker != NULL && leftTracker->checkPressed(repeat, bypass);
    }
    else if (button == 'E')
    {
      bool pressed =  midTracker != NULL && midTracker->checkPressed(repeat, bypass);
//if (pressed && !bypass) { Serial.print(repeat); Serial.println(" <E>"); }
      return pressed;
    }
    else
    {
      return false;
    }
  }
  // virtual bool _checkPressedBypass(char button)
  // {
  //   if (button == 'A')
  //   {
  //     return upTracker != NULL && upTracker->checkPressedBypass();
  //   }
  //   else if (button == 'B')
  //   {
  //     return rightTracker != NULL && rightTracker->checkPressedBypass();
  //   }
  //   else if (button == 'C')
  //   {
  //     return downTracker != NULL && downTracker->checkPressedBypass();
  //   }
  //   else if (button == 'D')
  //   {
  //     return leftTracker != NULL && leftTracker->checkPressedBypass();
  //   }
  //   else if (button == 'E')
  //   {
  //     return midTracker != NULL && midTracker->checkPressedBypass();
  //   }
  //   else
  //   {
  //     return false;
  //   }
  // }

private:
  ButtonPressTracker *leftTracker;
  ButtonPressTracker *rightTracker;
  ButtonPressTracker *upTracker;
  ButtonPressTracker *downTracker;
  ButtonPressTracker *midTracker;
  //bool buttonsOnly;
};

/// Helper class for joystick input tracking. See JoystickInterface.
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


#endif