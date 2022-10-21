class ButtonPressTracker {
public:
  ButtonPressTracker() {
    this->pressed = false;  // assume initially not pressed
    this->blackOutMillis = 0;
  }
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
  bool pressed;
  long blackOutMillis;
};

class JoyStickPressTracker {
public:
  JoyStickPressTracker() {
    this->reading = 0;
    this->pressedDir = 0;
    this->pressedMillis = 0;
    this->needReset = false;
  }
  int8_t setReading(int reading) {
    //int oriReading = this->reading;
    int8_t oriPressedDir = this->pressedDir;
    this->reading = reading;
    if ((reading - threshold) < 0) {
      this->pressedDir = -1;
    } else if (reading + threshold > maxReading) {
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
  const int maxReading = 1024;
  //const int threshold = 10;  
  const int threshold = 5;  
private:
  int reading;  
  int8_t pressedDir;
  long pressedMillis;
  bool needReset;
};
