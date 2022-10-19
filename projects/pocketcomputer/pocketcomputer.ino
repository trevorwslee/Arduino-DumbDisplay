// * youtube.com/watch?v=NTaq6f7NV5U
// * https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives


#include "calculator.h"
#include "stopwatch.h"
#include "game.h"
#include "calendar.h"
#include "menu.h"


const char* COLOR_BG = "darkblue";
const char* COLOR_0 = "navy";
const char* COLOR_1 = "ivory";
const uint8_t TEXT_SIZE_MENU = 9;
const uint8_t TEXT_SIZE_2 = 11;
const int TEXT_SIZE_4 = 9;


#define BLUETOOTH
#ifdef BLUETOOTH
#include "ssdumbdisplay.h"
// assume HC-05 connected; 2 => TX of HC05; 3 => RX of HC05
// still can connect with OTG
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));
#else
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif



GraphicalDDLayer *display;


#include "main.h"


#include "pgm_util.h"

void setup() {

  pinMode(up,INPUT_PULLUP);
  pinMode(presS,INPUT_PULLUP);
  pinMode(down,INPUT_PULLUP);
  //pinMode(3,OUTPUT);

  display = dumbdisplay.createGraphicalLayer(64, 128);
  display->backgroundColor(COLOR_BG);

  display->setCursor(0, 10);
  display->print("... init ...");

  // display->cachePixelImage("logo.png", epd_bitmap_logo, 64, 30, COLOR_1);
  // display->cachePixelImage("calc.png", myBitmapcalc, 24, 24, COLOR_1);
  // display->cachePixelImage("stop.png", myBitmapstop, 24, 24, COLOR_1);
  // display->cachePixelImage("game.png", myBitmapgam, 24, 24, COLOR_1);
  // display->cachePixelImage("calen.png", myBitmapcalen, 24, 24, COLOR_1);
  // display->cachePixelImage("phone.png", myBitmapphone, 24, 24, COLOR_1);
  unsigned char buffer[240];
  display->cachePixelImage("logo.png", PgmCopyBytes(epd_bitmap_logo, sizeof(epd_bitmap_logo), buffer), 64, 30, COLOR_1);
  display->cachePixelImage("calc.png", PgmCopyBytes(myBitmapcalc, sizeof(myBitmapcalc), buffer), 24, 24, COLOR_1);
  display->cachePixelImage("stop.png", PgmCopyBytes(myBitmapstop, sizeof(myBitmapstop), buffer), 24, 24, COLOR_1);
  display->cachePixelImage("game.png", PgmCopyBytes(myBitmapgam, sizeof(myBitmapgam), buffer), 24, 24, COLOR_1);
  display->cachePixelImage("calen.png", PgmCopyBytes(myBitmapcalen, sizeof(myBitmapcalen), buffer), 24, 24, COLOR_1);
  display->cachePixelImage("phone.png", PgmCopyBytes(myBitmapphone, sizeof(myBitmapphone), buffer), 24, 24, COLOR_1);

  playerX = random(10, 50);

  //display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //display.display();

  display->clear();
  //display->setRotation(3);
  playerX = random(10, 50);
  //display.clearDisplay();
  //display.setFont(0);
  //display.setTextColor(WHITE);
  //display.display();
}

void loop() {

  if (fase == 0) {
    checkButtonsMenu();
    drawMenu();
  }

  if (fase == 1) {
    checkButtonsCalc();
    drawCalc();
  }

  if (fase == 2) {
    checkButtonsStop();
    drawStop();
  }

  if (fase == 3) {
    checkColision();
    checkButtonsGame();
    drawGame();
  }

  if (fase == 4) {
    calendarDraw();
    checkButtonsCalendar();
  }

  if (fase == 5) {
    phoneDraw();
  }



  if (digitalRead(down) == 0 && digitalRead(presS) == 0) {
    GameReset();
    //display->setRotation(3);
    resetAll();
    fase = 0;
    delay(500);
  }
}

