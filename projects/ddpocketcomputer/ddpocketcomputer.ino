// *** 
// * adapted from: youtube.com/watch?v=NTaq6f7NV5U
// *** 


// if using Arduino UNO (assume with Joy Stick Shield)
// otherwise, assume Arduino Nano
#if defined(ARDUINO_AVR_UNO)
  #define WITH_JOYSTICK
  const uint8_t left = 5;
  const uint8_t right = 3;
  const uint8_t presS = 2;
  const uint8_t horizontal = A0;
  const uint8_t vertical = A1;
#elif defined(ARDUINO_AVR_NANO)
  const uint8_t left = PIN_A1;
  const uint8_t right = PIN_A4;
  const uint8_t presS = PIN_A2;
#elif defined(PICO_SDK_VERSION_MAJOR)
  const uint8_t left = 15;
  const uint8_t right = 16; 
  const uint8_t presS = 14;
#else
  #error not configured for board yet
#endif




const char* COLOR_BG = "darkblue";
const char* COLOR_DEF = "beige";
const char* COLOR_0 = "navy";
const char* COLOR_1 = "ivory";
const uint8_t TEXT_SIZE_DEF = 9;
const uint8_t TEXT_SIZE_2 = 24;
const int TEXT_SIZE_4 = 28;



#if defined(PICO_SDK_VERSION_MAJOR)
  // GP8 => RX of HC06; GP9 => TX of HC06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


GraphicalDDLayer *display;


#include "core.h"
#include "pgm_util.h"


void setup() {

  dumbdisplay.connect();
  dumbdisplay.writeComment("initializing ...");

  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(presS, INPUT_PULLUP);

#ifdef WITH_JOYSTICK  
  pinMode(horizontal, INPUT);
  pinMode(vertical, INPUT);
#endif  

  display = dumbdisplay.createGraphicalLayer(64, 128);
  
  display->backgroundColor(COLOR_BG);
  display->setTextColor(COLOR_DEF);

  display->setCursor(0, 10);
  display->print("...");

  unsigned char buffer[240];
  display->cachePixelImage("logo.png", PgmCopyBytes(epd_bitmap_logo, sizeof(epd_bitmap_logo), buffer), 64, 30, COLOR_1, DD_COMPRESS_BA_0);
  display->cachePixelImage("calc.png", PgmCopyBytes(myBitmapcalc, sizeof(myBitmapcalc), buffer), 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  display->cachePixelImage("stop.png", PgmCopyBytes(myBitmapstop, sizeof(myBitmapstop), buffer), 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  display->cachePixelImage("game.png", PgmCopyBytes(myBitmapgam, sizeof(myBitmapgam), buffer), 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  display->cachePixelImage("calen.png", PgmCopyBytes(myBitmapcalen, sizeof(myBitmapcalen), buffer), 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  display->cachePixelImage("phone.png", PgmCopyBytes(myBitmapphone, sizeof(myBitmapphone), buffer), 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("logo.png", epd_bitmap_logo, 64, 30, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("calc.png", myBitmapcalc, 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("stop.png", myBitmapstop, 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("game.png", myBitmapgam, 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("calen.png", myBitmapcalen, 24, 24, COLOR_1, DD_COMPRESS_BA_0);
  // display->cachePixelImage("phone.png", myBitmapphone, 24, 24, COLOR_1, DD_COMPRESS_BA_0);

  display->setTextFont("MONOSPACE");
  display->clear();

  dumbdisplay.writeComment("... done initialization");

  randomSeed(millis());

  GameReset();
}

void loop() {

  if (fase == 0) {
    handleMenu();
  } else {
    if (checkReset()) {
      return;
    }
  }

  if (fase == 1) {
    handleCalc();
  }

  if (fase == 2) {
    handleStop();
  }

  if (fase == 3) {
    handleGame();
  }

  if (fase == 4) {
    handleCalendar();
  }

  if (fase == 5) {
    handlePhone();
  }
}


