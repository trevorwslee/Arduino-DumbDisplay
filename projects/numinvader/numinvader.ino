#include <Arduino.h>
#include "dumbdisplay.h"

#include "buttonpresstracker.h"





#define PIN_LEFT  PIN4
#define PIN_RIGHT PIN2


DumbDisplay dumbdisplay(new DDInputOutput(115200));

SevenSegmentRowDDLayer *laserGun7Seg;
SevenSegmentRowDDLayer *invaders7Seg;




void setup() {
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_LEFT, INPUT_PULLUP);
}


void loop() {
}
