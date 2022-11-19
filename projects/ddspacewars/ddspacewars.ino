
// *** 
// * adapted from: TTGO T Display (ESP32) - Space Shooter game- free code
// * -- https://www.youtube.com/watch?v=KZMkGDyGjxQ&t=310s
// *** 



#if defined(ARDUINO_AVR_UNO)
  // *** config for Arduino UNO, with Joystick Shield

  //#define DOWNLOAD_IMAGES
  #define DEBUG_LED_PIN 13
  #define BTN_A 3
  #define BTN_B 2
  #define HORIZONTAL A0
  #define VERTICAL A1
  const bool joystickReverseHoriDir = false;
  const bool joystickReverseVertDir = false;
  const bool joystickAutoTune = true;

#elif defined(PICO_SDK_VERSION_MAJOR)
  // *** config for Raspberry Pi Pico, with Joystick and buttons

  //#define SAVE_IMAGES
  #define DOWNLOAD_IMAGES
  #define SHOW_SPACE
  #define DEBUG_LED_PIN 1
  #define BTN_A 21
  #define BTN_B 18
  #define HORIZONTAL 26
  #define VERTICAL 27
  const bool joystickReverseHoriDir = true;
  const bool joystickReverseVertDir = false;
  const bool joystickAutoTune = true;

#else

#error not configured for board yet

#endif


#define SHOW_LIVES

#define IF_BACK2 "BA"
#define IF_SENS "SE"
#define IF_GAMEOVER "GO"
#define IF_BROD1 "BR"
#define IF_BULET "BU"
#define IF_ROCKET "RO"
#define IF_EX2 "EX"
#define IF_EXPLOSION "EXP"
#define IF_BUUM "BUM"
#define IF_EBULLET "EB"
#define IF_EARTH(level) ("E-" + String(level))
#define IF_SPACEWARS_IMGS "spacewarsimgs"

const int NOTE_A4 = 466;
const int NOTE_B4 = 523;
const int NOTE_C4 = 277;
const int NOTE_D4 = 311;
const int NOTE_E4 = 349;
const int NOTE_F4 = 370;
const int NOTE_G4 = 415;
const int NOTE_C5 = 554;
const int NOTE_G5 = 831;

#define TFT_BLACK "black"
#define TFT_GREEN "green"
#define TFT_GREY DD_HEX_COLOR(0x5AEB)
#define lightblue DD_HEX_COLOR(0x2D18)
#define orange DD_HEX_COLOR(0xFB60)
#define purple DD_HEX_COLOR(0xFB9B)


#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#include "Core.h"

void setup(void)
{
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
#if defined(HORIZONTAL)
  pinMode(HORIZONTAL, INPUT);
  pinMode(VERTICAL, INPUT);
#endif

#if defined(DEBUG_LED_PIN)
  pinMode(DEBUG_LED_PIN, OUTPUT);
  digitalWrite(DEBUG_LED_PIN, 0);
#endif
}

int readyStage = 0;
void loop()
{
  if (readyStage == 0) {
    main_layer = dumbdisplay.createGraphicalLayer(240, 135);
    main_layer->noBackgroundColor();
#if defined(SAVE_IMAGES)
    dumbdisplay.writeComment("start caching ...");
    dumbdisplay.writeComment("... caching back2 ...");
    main_layer->cachePixelImage16(IF_BACK2, back2, 240, 135, "", DD_COMPRESS_BA_0);
    if (true)
    {
      dumbdisplay.recordLayerCommands();
      main_layer->drawImageFile(IF_BACK2);
      main_layer->fillRect(0, 78, 120, 25, TFT_BLACK);
      dumbdisplay.playbackLayerCommands();
    }
    dumbdisplay.writeComment("... cachine sens ...");
    main_layer->cachePixelImage16(IF_SENS, sens, 72, 72, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine gameOver ...");
    main_layer->cachePixelImage16(IF_GAMEOVER, gameOver, 240, 135, "", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine brod1 ...");
    main_layer->cachePixelImage16(IF_BROD1, brod1, 49, 40, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine bulet ...");
    main_layer->cachePixelImage16(IF_BULET, bulet, 8, 8, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine rocket ...");
    main_layer->cachePixelImage16(IF_ROCKET, rocket, 24, 12, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine ex2 ...");
    main_layer->cachePixelImage16(IF_EX2, ex2, 12, 12, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine explosion ...");
    main_layer->cachePixelImage16(IF_EXPLOSION, explosion, 24, 24, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine buum ...");
    main_layer->cachePixelImage16(IF_BUUM, buum, 55, 55, "0>a0", DD_COMPRESS_BA_0);
    dumbdisplay.writeComment("... cachine ebullet ...");
    main_layer->cachePixelImage16(IF_EBULLET, ebullet, 7, 7, "0>a0", DD_COMPRESS_BA_0);
    for (int i = 0; i < LevelCount; i++)
    {
      int level = i + 1;
      dumbdisplay.writeComment("... caching earth-" + String(level - 1) + " ...");
      main_layer->cachePixelImage16(IF_EARTH(level), earth[level - 1], 55, 54, "0>a0", DD_COMPRESS_BA_0);
    }
    dumbdisplay.writeComment("... done caching");
    main_layer->saveCachedImageFiles(IF_SPACEWARS_IMGS);
#endif
#if defined(DOWNLOAD_IMAGES)
    dumbdisplay.writeComment("download images ...");
    // image URL: https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/spacewarsimgs.png
    download_tunnel = dumbdisplay.createImageDownloadTunnel("https://${DDSS}/spacewarsimgs.png", IF_SPACEWARS_IMGS, false);
    dumbdisplay.writeComment("... ...");
#endif
    readyStage = 1;
  }
  if (readyStage == 1)
  {
#if defined(DOWNLOAD_IMAGES)
    int download_res = download_tunnel->checkResult();
    if (download_res == 0)
    {
      return;
    }
    if (download_res == -1)
    {
      dumbdisplay.writeComment("... failed to download images");
      delay(2000);
      return;
    }
    dumbdisplay.writeComment("... done download images");
    dumbdisplay.deleteTunnel(download_tunnel);
#endif
  top_layer = dumbdisplay.createGraphicalLayer(240, 135);
  top_layer->noBackgroundColor();
  Ebulet_layer = dumbdisplay.createGraphicalLayer(240, 135);
  Ebulet_layer->noBackgroundColor();
  rocket_layer = dumbdisplay.createGraphicalLayer(240, 135);
  rocket_layer->noBackgroundColor();
  bulet_layer = dumbdisplay.createGraphicalLayer(240, 135);
  bulet_layer->noBackgroundColor();
#if defined(SHOW_SPACE)
  for (int i = 0; i < SpaceLayerCount; i++)
  {
    space_layers[i] = dumbdisplay.createGraphicalLayer(240, 135);
    space_layers[i]->noBackgroundColor();
  }
#endif
  bg_layer = dumbdisplay.createGraphicalLayer(240, 135);
  bg_layer->backgroundColor(TFT_BLACK);
    int x = 0;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 240, 135, IF_BACK2);
    x += 240;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 72, 72, IF_SENS);
    x += 72;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 240, 135, IF_GAMEOVER);
    x += 240;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 49, 40, IF_BROD1);
#if defined(SHOW_LIVES)
    top_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 49, 40, IF_BROD1);
#endif
    x += 49;
    bulet_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 8, 8, IF_BULET);
    x += 8;
    rocket_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 24, 12, IF_ROCKET);
    x += 24;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 12, 12, IF_EX2);
    x += 12;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 24, 24, IF_EXPLOSION);
    x += 24;
    main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 55, 55, IF_BUUM);
    x += 55;
    Ebulet_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 7, 7, IF_EBULLET);
    x += 7;
    for (int i = 0; i < LevelCount; i++)
    {
      int level = i + 1;
      main_layer->loadImageFileCropped(IF_SPACEWARS_IMGS, x + i * 55, 0, 55, 54, IF_EARTH(level));
    }
    readyStage = 2;
  }

  if (fase == 0)
  {
    handleRestart();
  }
  else if (fase == 1)
  {
    handlePlay();
  }
  else if (fase == 2)
  {
    handleGameOver();
  }
}
