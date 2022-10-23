class ButtonPressTracker {
public:
  ButtonPressTracker(uint8_t pin) {
    this->pin = pin;
    this->pressed = false;  // assume initially not pressed
    this->blackOutMillis = 0;
  }
  bool checkPressed() {
    return setPressed(digitalRead(this->pin) == 0);
  }
  bool checkPressedBypass() {
    setPressed(digitalRead(this->pin) == 0);
    return pressed;
  }
private:
  bool setPressed(bool pressed) {
    long nowMillis = millis();
    if (blackOutMillis != 0) {
      long diff = blackOutMillis - nowMillis;
      if (diff < 0) {
        blackOutMillis = 0;
      }
    }
    if (blackOutMillis == 0) {
      if (pressed != this->pressed) {
        this->pressed = pressed;
        blackOutMillis = nowMillis + 50;
        return this->pressed;
      }
    }
    return false;
  }
private:
  uint8_t pin;
  bool pressed;
  long blackOutMillis;
};

class JoyStickPressTracker {
public:
  JoyStickPressTracker(uint8_t pin) {
    this->maxReading = 1023;
    this->threshold = 10;
    this->pin = pin;  
    //this->reading = 0;
    this->pressedDir = 0;
    this->pressedMillis = 0;
    this->needReset = false;
  }
public:
  int8_t checkPressed() {
    return setReading(analogRead(this->pin));
  }  
private:
  int8_t setReading(int reading) {
    //int oriReading = this->reading;
    int8_t oriPressedDir = this->pressedDir;
    //this->reading = reading;
    if ((reading - threshold) <= 0) {
      this->pressedDir = -1;
    } else if ((reading + threshold) >= maxReading) {
      this->pressedDir = 1;
    } else {
      this->pressedDir = 0;
    }
    if (!this->needReset && this->pressedMillis != 0 && (this->pressedDir == oriPressedDir)) {
      long diffMillis = millis() - this->pressedMillis;
      if (diffMillis > 50) {
        this->pressedDir = 0;
        this->pressedMillis = 0;
        this->needReset = true;
        return oriPressedDir;
      } 
    } else {
      if (this->pressedDir != 0) {
        if (this->pressedMillis == 0) {
          this->pressedMillis = millis();
        }
      } else {
        this->pressedMillis = 0;
        this->needReset = false;
      }
    }
    return 0;
  }
private:
  int maxReading;
  int threshold;  
private:
  //int reading;  
  uint8_t pin;
  int pressedDir;
  long pressedMillis;
  bool needReset;
};
