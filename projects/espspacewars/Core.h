#if defined(SAVE_IMAGES)
#include "rocket.h"
#include "brod1.h"
#include "bulet.h"
#include "ebullet.h"
#include "life.h"
#include "rover.h"
#include "earth.h"
#include "ex.h"
#include "ex2.h"
#include "ricon.h"
#include "back2.h"
#include "sens.h"
#include "buum.h"
#include "gameOver.h"
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

#define NOTE_A4 466
#define NOTE_B4 523 
#define NOTE_C4 277 
#define NOTE_D4 311 
#define NOTE_E4 349 
#define NOTE_F4 370 
#define NOTE_G4 415 
#define NOTE_C5 554 
#define NOTE_G5 831

#define TFT_BLACK "black"
#define TFT_GREEN "green"
#define TFT_GREY DD_HEX_COLOR(0x5AEB)
#define lightblue DD_HEX_COLOR(0x2D18)
#define orange DD_HEX_COLOR(0xFB60)
#define purple DD_HEX_COLOR(0xFB9B)






//#include "ddjoystick.h"
#include "Misc.h"

// // JoystickInterface* buttons = new ButtonsOnly(new ButtonPressTracker(BTN_A),
// //                                              new ButtonPressTracker(BTN_B),
// //                                              NULL, NULL);

// #if defined(HORIZONTAL)
// // JoystickInterface* joystick = new JoystickJoystick(new JoystickPressTracker(HORIZONTAL, joystickReverseHoriDir),
// //                                                    new JoystickPressTracker(VERTICAL, joystickReverseVertDir),
// //                                                    NULL);
// #endif


const int LevelCount = 6;

const int BuletCount = 10;
const int RocketCount = 4;
const int EbuletCount = 10;

#if defined SHOW_SPACE
const int StarCount = 15;
const int SpaceLayerCount = 6;
#endif

const float init_sped = 4;
const float buletSpeed = 5;
const float init_es = 1.5;
const float init_rocketSpeed = 3;
const float init_EbuletSpeed = buletSpeed;


FrameControl frameControl;

int brojac;

Position buletXY[BuletCount];
PositionGroup buletGroup(buletXY, BuletCount);

Position EbuletXY[EbuletCount];
PositionGroup EbulletGroup(EbuletXY, EbuletCount);

Position rocketXY[RocketCount];
PositionGroup rocketGroup(rocketXY, RocketCount);

float rocketSpeed = init_rocketSpeed;

int rockets;

int counter;
int rcounter;
int Ecounter;
int level;

Position xy(10, 20);

Position exy(170, 18);

float es = init_es;

float sped = init_sped;
int eHealth;
int mHealth;
int lives;
int fireTime = 100;
int fireCount = 0;
float EbuletSpeed = init_EbuletSpeed;
int rDamage;

bool sound = 1; // sound on or off

int fase = 0; // fase 0=start screen,//fase 1=playing fase //fase 3=game over

GraphicalDDLayer *bg_layer;
GraphicalDDLayer *main_layer;
GraphicalDDLayer *bulet_layer;
GraphicalDDLayer *rocket_layer;
GraphicalDDLayer *Ebulet_layer;
GraphicalDDLayer *top_layer;
#if defined(SHOW_SPACE)
GraphicalDDLayer *space_layers[SpaceLayerCount];
#endif
#if defined(DOWNLOAD_IMAGES)
SimpleToolDDTunnel *download_tunnel;
#endif


void resetScreen()
{
#if defined(DEBUG_LED_PIN)
  digitalWrite(DEBUG_LED_PIN, 0);
#endif
  bulet_layer->clear();
  rocket_layer->clear();
  Ebulet_layer->clear();
  top_layer->clear();
#if defined SHOW_SPACE
  for (int i = 0; i < SpaceLayerCount; i++)
  {
    space_layers[i]->clear();
  }
#endif
  main_layer->clear();
}

void drawTop()
{
  top_layer->clear();
#if defined(SHOW_LIVES)
  for (int i = 0; i < lives; i++)
  {
    top_layer->drawImageFile(IF_BROD1, 5 + i * 12, 3, 12);
  }
#endif
  top_layer->setCursor(200, 0);
  top_layer->print(String(brojac));
  // eHealth--;
  int tr = map(eHealth, 0, mHealth, 0, 70);
  top_layer->fillRect(120, 3, 70, 7, TFT_GREEN);
  top_layer->drawRect(119, 2, 72, 9, TFT_GREY);
  top_layer->fillRect(120, 3, 70, 7, TFT_BLACK);
  top_layer->fillRect(120, 3, tr, 7, TFT_GREEN);
}

void restart()
{
  frameControl.reset();
  counter = 0;
  rcounter = 0;
  Ecounter = 0;
  level = 1;
  xy.reset(10, 20);
  exy.reset(170, 18);
  es = init_es;

  rockets = 3;
  rDamage = 8;
  lives = 4;
  brojac = 0;
  exy.reset(exy.getX(), 44);
  sped = init_sped;
  eHealth = 50;
  mHealth = eHealth;
  EbuletSpeed = init_EbuletSpeed;
  rocketSpeed = init_rocketSpeed;

  buletGroup.resetAll(-20, -20);
  rocketGroup.resetAll(-20, -20);
}

void newLevel()
{
  frameControl.reset();
  level++;
  sped = init_sped + level / 3;
  EbuletSpeed = EbuletSpeed + init_EbuletSpeed / 3;
  eHealth = 50 + (level * 5);
  mHealth = eHealth;
  es = init_es + (init_es / 3 * level);

  rockets = 3;
  rDamage = 8 + (level * 2);
  rocketSpeed = init_rocketSpeed + (init_rocketSpeed / 3 * level);

  exy.reset(exy.getX(), 44);

  buletGroup.resetAll(-20, -20);
  rocketGroup.resetAll(-20, -20);
  EbulletGroup.resetAll(-20, -20);

  resetScreen();

  main_layer->setCursor(0, 0);
  main_layer->print("Level " + String(level));
  main_layer->setCursor(0, 22);

  main_layer->println("Enemy speed : " + String(es));
  main_layer->println("Enemy health : " + String(eHealth));
  main_layer->println("Enemy bullet speed : " + String(EbuletSpeed));
  main_layer->println("Remaining lives: " + String(lives));
  main_layer->println("My speed : " + String(sped));
  main_layer->println("Rocket damage : " + String(rDamage));
  main_layer->println("Rocket speed : " + String(rocketSpeed));

  main_layer->drawImageFile(IF_EARTH(level), 170, 5);
  main_layer->drawImageFile(IF_SENS, 170, 61);
  delay(2600);

  // while (!btnATracker.checkPressed())
  //   ;
  while (!buttons->checkAPressed()) {
    OnIdle();
  }

  resetScreen();

  main_layer->drawLine(0, 16, 240, 16, lightblue);
  main_layer->drawLine(0, 134, 240, 134, lightblue);

  drawTop();
}

void handleRestart()
{
  restart();
  resetScreen();
  main_layer->drawImageFile(IF_BACK2);
  // while (!btnATracker.checkPressed())
  //   ;
  while (!buttons->checkAPressed()) {
    OnIdle();
  }
  resetScreen();
  main_layer->setCursor(0, 0);
  main_layer->print("Level " + String(level));
  main_layer->setCursor(0, 22);

  main_layer->println("Enemy speed : " + String(es));
  main_layer->println("Enemy health : " + String(eHealth));
  main_layer->println("Enemy bullet speed : " + String(EbuletSpeed));
  main_layer->println("Remaining lives: " + String(lives));
  main_layer->println("My speed : " + String(sped));
  main_layer->println("Rocket damage : " + String(rDamage));
  main_layer->println("Rocket speed : " + String(rocketSpeed));

  main_layer->drawImageFile(IF_EARTH(level), 170, 5);
  main_layer->drawImageFile(IF_SENS, 170, 61);

  // while (!btnATracker.checkPressed())
  //   ;
  while (!buttons->checkAPressed()) {
    OnIdle();
  }

  resetScreen();

  main_layer->drawLine(0, 16, 240, 16, lightblue);
  main_layer->drawLine(0, 134, 240, 134, lightblue);

  drawTop();

#if defined(SHOW_SPACE)
  for (int i = 0; i < SpaceLayerCount; i++)
  {
    space_layers[i]->transparent(random(0, 2));
    for (int j = 0; j < StarCount; j++)
    {
      space_layers[i]->drawPixel(random(5, 235), random(18, 132), TFT_GREY);
    }
  }
#endif

  fase = 1;
}

void handlePlay()
{
  bool frameDue = frameControl.checkDue();
  long frameNum = frameControl.getFrameNum();

  const JoystickPress* joystickPress = joystick->checkJoystickPress(50);
  int horizontalPress = 0;
  int verticalPress = 0;
  if (joystickPress != NULL) {
    horizontalPress = joystickPress->xPressed;
    verticalPress = joystickPress->yPressed;
  }
  //int8_t horizontalPress = horizontalTracker.checkPressed(50);
  //int8_t verticalPress = verticalTracker.checkPressed(50);

  int x = xy.getX();
  int y = xy.getY();

  if (verticalPress == 1 && y < 94) // Move down
    y = y + sped;

  if (verticalPress == -1 && y > 18) // Move up
    y = y - sped;

  if (horizontalPress == 1 && x < 125) // Move right
    x = x + sped;

  if (horizontalPress == -1 && x > 0) // Move left
    x = x - sped;

  xy.moveTo(x, y);

  // const ABCDPressed* abcdPressed = buttons->checkABCDPressed();
  // bool aPressed = false;
  // bool bPressed = false;
  // if (abcdPressed != NULL) {
  //   aPressed = abcdPressed->aPressed;
  //   bPressed = abcdPressed->bPressed;
  // }
  bool aPressed = buttons->checkAPressed();
  bool bPressed = buttons->checkBPressed();

  if (aPressed/*btnATracker.checkPressed()*/)
  {
    buletXY[counter].moveTo(xy.getX() + 34, xy.getY() + 15);
    counter = counter + 1;
  }

  if (bPressed/*btnBTracker.checkPressed()*/ && rockets > 0)
  {
    rockets--;
    rocketXY[rcounter].moveTo(xy.getX() + 34, xy.getY() + 14);
    rcounter = rcounter + 1;
  }

#if defined(SHOW_SPACE)
  if (frameDue && frameNum % 15 == 0)
  {
    for (int i = 0; i < SpaceLayerCount; i++)
    {
      space_layers[i]->transparent(random(0, 2));
    }
  }
#endif

  bool refreshMain = xy.checkMoved() || exy.checkMoved();
  bool refreshBulets = buletGroup.checkAnyMoved();
  bool refreshRockets = rocketGroup.checkAnyMoved();
  bool refreshEbullets = EbulletGroup.checkAnyMoved();

  bool freezeScreen = refreshMain || refreshBulets || refreshRockets || refreshEbullets;
  if (freezeScreen)
  {
    dumbdisplay.recordLayerCommands();
  }

  if (refreshMain)
  {
    main_layer->clear();
    main_layer->drawImageFile(IF_BROD1, xy.getX(), xy.getY());
    main_layer->drawImageFile(IF_EARTH(level), exy.getX(), exy.getY());
  }

  if (refreshBulets)
  {
    bulet_layer->clear();
  }
  for (int i = 0; i < BuletCount; i++)
  { // firing buletts
    if (buletXY[i].getX() > 0)
    {
      if (refreshBulets)
      {
        bulet_layer->drawImageFile(IF_BULET, buletXY[i].getX(), buletXY[i].getY());
      }
      if (frameDue)
      {
        buletXY[i].moveBy(buletSpeed /*0.6*/, 0);
      }
    }
    if (buletXY[i].getX() > 240)
      buletXY[i].moveXTo(-30);
  }

  if (refreshRockets)
  {
    rocket_layer->clear();
  }
  for (int i = 0; i < RocketCount; i++)
  { // firing rockets
    if (rocketXY[i].getX() > 0)
    {
      if (refreshRockets)
      {
        rocket_layer->drawImageFile(IF_ROCKET, rocketXY[i].getX(), rocketXY[i].getY());
      }
      if (frameDue)
      {
        rocketXY[i].moveBy(rocketSpeed, 0);
      }
    }
    if (rocketXY[i].getX() > 240)
      rocketXY[i].moveXTo(-30);
  }

  for (int j = 0; j < BuletCount; j++) // did my bulet hit enemy
  {
    if (buletXY[j].getX() > exy.getX() + 20 && buletXY[j].getY() > exy.getY() + 2 && buletXY[j].getY() < exy.getY() + 52)
    {
      main_layer->drawImageFile(IF_EX2, buletXY[j].getX(), buletXY[j].getY());
      if (sound == 1)
      {
        dumbdisplay.tone(NOTE_C5, 120);
      }
      delay(120);
      main_layer->fillRect(buletXY[j].getX(), buletXY[j].getY(), 12, 12, TFT_BLACK);
      buletXY[j].moveXTo(-50);
      brojac = brojac + 1;
      eHealth--;
      drawTop();
    }
  }

  for (int j = 0; j < RocketCount; j++) // did my ROCKET hit enemy
  {
    if (rocketXY[j].getX() + 18 > exy.getX() && rocketXY[j].getY() > exy.getY() + 2 && rocketXY[j].getY() < exy.getY() + 52)
    {
      main_layer->drawImageFile(IF_EXPLOSION, rocketXY[j].getX(), rocketXY[j].getY());
      if (sound == 1)
      {
        dumbdisplay.tone(NOTE_C4, 400);
      }
      delay(400);
      main_layer->fillRect(rocketXY[j].getX(), rocketXY[j].getY(), 24, 24, TFT_BLACK);

      rocketXY[j].moveXTo(-50);
      brojac = brojac + 12;
      eHealth = eHealth - rDamage;
      drawTop();
    }
  }

  for (int j = 0; j < EbuletCount; j++) // Am I hit
  {
    if (EbuletXY[j].getX() < xy.getX() + 30 && EbuletXY[j].getX() > xy.getX() + 4 && EbuletXY[j].getY() > xy.getY() + 4 && EbuletXY[j].getY() < xy.getY() + 36)
    {
      EbuletXY[j].moveXTo(-50);
      main_layer->fillRect((lives - 1) * 14, 0, 14, 14, TFT_BLACK);
      lives--;
#if defined(SHOW_LIVES)
      drawTop();
#endif
      if (sound == 1)
      {
        dumbdisplay.tone(NOTE_C5, 400);
      }
      delay(400);
    }
  }

  if (frameDue)
  {
    exy.moveBy(0, es);
    if (exy.getY() > 80)
      es = es * -1;

    if (exy.getY() < 18)
      es = es * -1;
  }

  if (refreshEbullets)
  {
    Ebulet_layer->clear();
  }
  for (int i = 0; i < EbuletCount; i++)
  { // enemy shoots
    if (EbuletXY[i].getX() > -10)
    {
      if (refreshEbullets)
      {
        Ebulet_layer->drawImageFile(IF_EBULLET, EbuletXY[i].getX(), EbuletXY[i].getY());
      }
      if (frameDue)
      {
        EbuletXY[i].moveBy(-EbuletSpeed, 0);
      }
    }
  }

  if (frameDue)
  {
    fireCount += 2;
    if (fireTime <= fireCount)
    {
      EbuletXY[Ecounter].moveTo(exy.getX() + 5, exy.getY() + 24);
      fireCount = 0;
      fireTime = random(110 - (level * 15), 360 - (level * 30));
      Ecounter++;
    }
  }

  if (freezeScreen)
  {
    dumbdisplay.playbackLayerCommands();
  }

  if (eHealth <= 0)
  {
    main_layer->drawImageFile(IF_BUUM, exy.getX(), exy.getY());
    dumbdisplay.tone(NOTE_E4, 100);
    dumbdisplay.tone(NOTE_D4, 80);
    dumbdisplay.tone(NOTE_G5, 100);
    dumbdisplay.tone(NOTE_C4, 80);
    dumbdisplay.tone(NOTE_F4, 280);
    delay(700);
    newLevel();
  }
  if (lives == 0)
  {
    main_layer->drawImageFile(IF_BUUM, xy.getX(), xy.getY());
    dumbdisplay.tone(NOTE_G4, 100);
    dumbdisplay.tone(NOTE_B4, 80);
    dumbdisplay.tone(NOTE_C5, 100);
    dumbdisplay.tone(NOTE_A4, 80);
    dumbdisplay.tone(NOTE_F4, 280);
    delay(500);
    resetScreen();
    fase = 2;
  }

  if (counter == BuletCount)
    counter = 0;

  if (rcounter == RocketCount)
    rcounter = 0;

  if (Ecounter == EbuletCount)
    Ecounter = 0;
}

void handleGameOver()
{
  resetScreen();
  main_layer->drawImageFile(IF_GAMEOVER);
  main_layer->setCursor(24, 54);
  main_layer->print("Score : " + String(brojac));
  main_layer->setCursor(24, 69);
  main_layer->print("Level : " + String(level));
  // while (!btnATracker.checkPressed())
  //   ;
  while (!buttons->checkAPressed()) {
    OnIdle();
  }

  fase = 0;
}


void gameLoop()
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
    return;
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
    main_layer->drawImageFile(IF_BACK2);
    readyStage = 2;
    return;
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


