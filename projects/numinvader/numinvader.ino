#include <Arduino.h>
#include "dumbdisplay.h"

#define BUTTON_LEFT PIN_A1
#define BUTTON_RIGHT PIN_A4

#define LED_LEFT PIN_A2
#define LED_RIGHT PIN_A3

#include "gameobjs.h"


DumbDisplay dumbdisplay(new DDInputOutput(115200));


// game objects, defined in gameobjs.h
LaserGun laserGun;
Invaders invaders;
Controller controller(laserGun, invaders);


void setup() {
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);

  laserGun.life7Seg = dumbdisplay.create7SegmentRowLayer(1);
  laserGun.life7Seg->segmentColor("green");
  laserGun.life7Seg->border(5, "green", "round");
  laserGun.life7Seg->padding(5);

  laserGun.laserGun7Seg = dumbdisplay.create7SegmentRowLayer(1);
  laserGun.laserGun7Seg->segmentColor("blue");
  laserGun.laserGun7Seg->backgroundColor("yellow");
  laserGun.laserGun7Seg->border(5, "blue", "round");
  laserGun.laserGun7Seg->padding(5);

  invaders.invader7Seg = dumbdisplay.create7SegmentRowLayer(MAX_INVADER_COUNT);
  invaders.invader7Seg->segmentColor("red");
  invaders.invader7Seg->border(5, "red", "round");
  invaders.invader7Seg->padding(5);

  dumbdisplay.configAutoPin(DD_AP_HORI);

  controller.initialize();
}


void loop() {
  controller.loop();
}