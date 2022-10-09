#include <Arduino.h>
#include "dumbdisplay.h"


// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));


// declare a 7-segment layer object, to be created in setup()
SevenSegmentRowDDLayer *sevenSeg;


#define PIN_LEFT PIN4
#define PIN_RIGHT PIN2



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



ButtonPressTracker leftTracker;
ButtonPressTracker rightTracker;
int count = 0;


void setup() {
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);

  // create the 7-segment layer object, with 3 7-segment digits
  sevenSeg = dumbdisplay.create7SegmentRowLayer(3);
  sevenSeg->border(5, "black", "round");
  sevenSeg->padding(5);
  sevenSeg->resetSegmentOffColor(DD_RGB_COLOR(0xff, 0xee, 0xff));
  sevenSeg->showNumber(count);
}


void loop() {
  int left = digitalRead(PIN_LEFT);
  int right = digitalRead(PIN_RIGHT);
  bool rightPressed = rightTracker.setPressed(right == 0);
  bool leftPressed = leftTracker.setPressed(left == 0);

  int preCount = count;
  if (leftPressed) {
    count -= 1;
  }
  if (rightPressed) {
    count += 1;
  }
  if (count != preCount) {
    sevenSeg->showNumber(count);
  }
}