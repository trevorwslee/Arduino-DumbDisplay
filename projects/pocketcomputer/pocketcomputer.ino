// * youtube.com/watch?v=NTaq6f7NV5U
// * https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives

#define ONLY_BUTTONS

// const uint8_t left = PIN_A1;
// const uint8_t presS = PIN_A0;
// const uint8_t right = PIN_A4;

// Uno
const uint8_t left = 5;
const uint8_t up = 2;
const uint8_t right = 15/*3*/;
const uint8_t down = 4;
const uint8_t horizontal = A0;
const uint8_t vertical = A1;
const uint8_t presS = 3/*15*/;


// Pico
// const uint8_t left = 5;
// const uint8_t up = 2;
// const uint8_t right = 3;
// const uint8_t down = 4;
// const uint8_t horizontal = 27;
// const uint8_t vertical = 26;
// const uint8_t presS = 15;


#include "calculator.h"
#include "stopwatch.h"
#include "game.h"
#include "calendar.h"
#include "menu.h"


const char* COLOR_BG = "darkblue";
const char* COLOR_DEF = "beige";
const char* COLOR_0 = "navy";
const char* COLOR_1 = "ivory";
const uint8_t TEXT_SIZE_DEF = 9;
//const uint8_t TEXT_SIZE_MENU = 9;
const uint8_t TEXT_SIZE_2 = 24;
const int TEXT_SIZE_4 = 28;

//const uint8_t DisplayWidth = 64;
//const uint8_t DisplayHeight = 128;


//#define BLUETOOTH

#if defined(FOR_PICO)
  // GP8 => RX of HC06; GP9 => TX of HC06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  /* HC-06 connectivity */
  DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));
#elif defined (BLUETOOTH)
  #include "ssdumbdisplay.h"
  // // assume HC-05 connected; 11 => TX of HC05; 10 => RX of HC05
  // // still can connect with OTG
  // DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(11, 10), 115200, true, 115200));
  // assume HC-05 connected; 2 => TX of HC05; 3 => RX of HC05
  // still can connect with OTG
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 9600, true, 115200));
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
  pinMode(left,INPUT_PULLUP);
  pinMode(right,INPUT_PULLUP);
  pinMode(horizontal,INPUT);
  pinMode(vertical,INPUT);
  //pinMode(3,OUTPUT);

  display = dumbdisplay.createGraphicalLayer(64, 128);
  display->backgroundColor(COLOR_BG);

  display->setCursor(0, 10);
  display->print("... init ...");

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

  display->setTextColor(COLOR_DEF);

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
    //checkButtonsMenu();
    //drawMenu();
    handleMenu();
  }

  if (fase == 1) {
    //checkButtonsCalc();
    //drawCalc();
    handleCalc();
  }

  if (fase == 2) {
    //checkButtonsStop();
    //drawStop();
    handleStop();
  }

  if (fase == 3) {
    //checkColision();
    //checkButtonsGame();
    //drawGame();
    handleGame();
  }

  if (fase == 4) {
    calendarDraw();
    checkButtonsCalendar();
  }

  if (fase == 5) {
    phoneDraw();
  }



  if (digitalRead(down) == 0 && digitalRead(right/*presS*/) == 0) {
    GameReset();
    //display->setRotation(3);
    resetAll();
    fase = 0;
    delay(500);
  }
}

