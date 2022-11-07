#if defined (ARDUINO_AVR_UNO)
  #define WITH_JOYSTICK
  const uint8_t LEFT = 5;
  const uint8_t RIGHT = 3;
  //const uint8_t ENTER = 2;
  const uint8_t HORIZONTAL = A0;
  const uint8_t VERTICAL = A1;
#else
  //#define SAVE_IMAGES
  #if defined(PICO_SDK_VERSION_MAJOR)
    const uint8_t LEFT = 15;
    const uint8_t RIGHT = 16; 
    //const uint8_t ENTER = 14;
  #else
    #error not configured for board yet
  #endif
#endif



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



#define LEVEL_COUNT 7


#define IF_BACK2 "back2"
#define IF_SENS "sens"
#define IF_GAMEOVER "gameOver"
#define IF_BROD1 "brod1"
#define IF_BULET "bulet"
#define IF_ROCKET "rocket"
#define IF_EX2 "ex2"
#define IF_EXPLOSION "explosion"
#define IF_BUUM "buum"
#define IF_EBULLET "ebullet"
#define IF_EARTH(level) ("earth-" + String(level))
#define IF_SPACEWARS_IMGS "spacewarsimgs"

#if defined(ESP32)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true, 115200));
#elif defined(PICO_SDK_VERSION_MAJOR)
  // GP8 => RX of HC06; GP9 => TX of HC06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif

#include "Note.h"


const int NOTE_C3 = GetNoteFreq(-1, ToNoteIdx('C', 0));

const int NOTE_A4 = GetNoteFreq(0, ToNoteIdx('A', 0));
const int NOTE_B4 = GetNoteFreq(0, ToNoteIdx('B', 0));
const int NOTE_D4 = GetNoteFreq(0, ToNoteIdx('D', 0));
const int NOTE_E4 = GetNoteFreq(0, ToNoteIdx('E', 0));
const int NOTE_F4 = GetNoteFreq(0, ToNoteIdx('F', 0));
const int NOTE_G4 = GetNoteFreq(0, ToNoteIdx('G', 0));

const int NOTE_C5 = GetNoteFreq(1, ToNoteIdx('C', 0));
const int NOTE_G5 = GetNoteFreq(1, ToNoteIdx('G', 0));

const int NOTE_C6 = GetNoteFreq(2, ToNoteIdx('C', 0));



#include "PressTracker.h"

ButtonPressTracker leftTracker(LEFT);    // B
ButtonPressTracker rightTracker(RIGHT);  // A
//ButtonPressTracker selectTracker(ENTER);

#if defined(WITH_JOYSTICK)
JoyStickPressTracker horizontalTracker(HORIZONTAL);
JoyStickPressTracker verticalTracker(VERTICAL);
#endif



//#define BUZZER_PIN 27
//#define BUZZER_CHANNEL 0

#define TFT_BLACK "black"
#define TFT_GREEN "green"
#define TFT_GREY DD_HEX_COLOR(0x5AEB)
#define lightblue DD_HEX_COLOR(0x2D18)
#define orange DD_HEX_COLOR(0xFB60)
#define purple DD_HEX_COLOR(0xFB9B)




class Position {
  public:
    Position(float pos_x, float pos_y) {
      reset(pos_x, pos_y);
    }
    Position(float pos_both_xy) {
      reset(pos_both_xy, pos_both_xy);
    }
  public:
    void moveBy(float inc_x, float inc_y) {
      this->pos_x += inc_x;
      this->pos_y += inc_y;
      this->last_x = this->x;
      this->last_y = this->y;
      this->x = this->pos_x;
      this->y = this->pos_y;
      if (last_valid)
        this->moved = this->last_x != this->x || this->last_y != this->y;
      else {
        this->last_valid = true;
        this->moved = true;
      }
    }
    void moveTo(float pos_x, float pos_y) {
      this->pos_x = pos_x;
      this->pos_y = pos_y;
      this->x = this->pos_x;
      this->y = this->pos_y;
      if (last_valid)
        this->moved = this->last_x != this->x || this->last_y != this->y;
      else {
        this->last_valid = true;
        this->moved = true;
      }
    }
    void moveXTo(float pos_x) {
      moveTo(pos_x, this->pos_y);
    }
    void moveYTo(float pos_y) {
      moveTo(this->pos_x, pos_y);
    }
    bool checkMoved() {
      bool res = this->moved;
      this->moved = false;
      return res;
    }
    void reset(float pos_x, float pos_y) {
      this->pos_x = pos_x;
      this->pos_y = pos_y;
      this->x = this->pos_x;
      this->y = this->pos_y;
      this->last_valid = false;
      this->moved = true;
      // if (last_valid)
      //   this->moved = this->last_x != this->x || this->last_y != this->y;
      // else {
      //   this->last_valid = true;
      //   this->moved = true;
      // }
    }
    // void reset(float x, float y) {
    //   this->pos_x = x;
    //   this->pos_y = y;
    //   this->x = this->pos_x;
    //   this->y = this->pos_y;
    //   this->last_valid = false;
    // }
    inline int getX() { return this->x; }
    inline int getY() { return this->y; }
  private:
    float pos_x;
    float pos_y;
    int x;
    int y;
    bool last_valid;
    int last_x;
    int last_y;
    bool moved;
};


//const int buletSpeed = 1;

//TFT_eSPI tft = TFT_eSPI();  
int brojac=0;// Invoke custom library
//float buletX[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};
//float buletY[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};
Position buletXY[10] = { Position(-10), Position(-10), Position(-10), Position(-10), Position(-10),
                         Position(-10), Position(-10), Position(-10), Position(-10), Position(-10) };

float EbuletX[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};
float EbuletY[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};

float rocketX[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};
float rocketY[10]={-20,-20,-20,-20,-20,-20,-20,-20,-20,-20};
float rocketSpeed=0.22;
int rockets=3;

int counter=0;
int rcounter=0;
int Ecounter=0;
int level=1;

Position xy(10, 20);
//float x=10;
//float y=20;

Position exy(170, 18);
//float ey=18;
//float ex=170;

float es=0.1;

float bx=-50;
float by=0;

int pom=0; //pressdebounce for fire
int pom2=0; //pressdebounce for rockets
float sped=0.42;
//int blinkTime=0;
int eHealth=50;
int mHealth=eHealth;
int lives=4;
int ly[4]={0,0,0,0};
int ri[3]={0,0,0};
int fireTime=100;
int fireCount=0;
float EbulletSpeed=0.42;
int rDamage=8; //rocket damage
int tr=0;

int pom3=0;
bool sound=1; //sound on or off

int fase=0; //fase 0=start screen,//fase 1=playing fase //fase 3=game over

float spaceX[30];
float spaceY[30];


GraphicalDDLayer* graphical;

void restart()
{
  counter = 0;
  rcounter = 0;
  Ecounter = 0;
  level = 1;
  xy.reset(10, 20);
  //x = 10;
  //y = 20;
  exy.reset(170, 18);
  //ey = 18;
  //ex = 170;
  es = 0.1;
  bx = -50;
  by = 0;

  rockets = 3;
  rDamage = 8;
  lives = 4;
  brojac = 0;
  ri[0] = 0;
  ri[1] = 0;
  ri[2] = 0;
  ly[0] = 0;
  ly[1] = 0;
  ly[2] = 0;
  ly[3] = 0;
  exy.reset(exy.getX(), 44);
// ey = 44;
   sped = 0.42;
  eHealth = 50;
  mHealth = eHealth;
  EbulletSpeed = 0.42;
  rocketSpeed = 0.22;

  for (int i = 0; i < 10; i++)
  {
    //buletX[i] = -20;
    buletXY[i].reset(-20, -20);
    EbuletX[i] = -20;
    rocketX[i] = -20;
  }
}

void newLevel()
{
  level++;
  sped = sped + 0.05;
  EbulletSpeed = EbulletSpeed + 0.1;
  eHealth = 50 + (level * 5);
  mHealth = eHealth;
  es = 0.05 + (0.035 * level);

  rockets = 3;
  rDamage = 8 + (level * 2);
  rocketSpeed = 0.22 + (level * 0.02);
  ri[0] = 0;
  ri[1] = 0;
  ri[2] = 0;

  exy.reset(exy.getX(), 44);
  //ey = 44;

  for (int i = 0; i < 10; i++)
  {
    //buletX[i] = -20;
    buletXY[i].reset(-20, -20);
    EbuletX[i] = -20;
    rocketX[i] = -20;
  }

  graphical->fillScreen(TFT_BLACK);
  graphical->setCursor(0, 0 /*,4*/);
  graphical->print("Level " + String(level));
  graphical->setCursor(0, 22 /*,2*/);

  graphical->println("Enemy speed : " + String(es));
  graphical->println("Enemy health : " + String(eHealth));
  graphical->println("Enemy bullet speed : " + String(EbulletSpeed));
  graphical->println("Remaining lives: " + String(lives));
  graphical->println("My speed : " + String(sped));
  graphical->println("Rocket damage : " + String(rDamage));
  graphical->println("Rocket speed : " + String(rocketSpeed));

  graphical->drawImageFile("earth-" + String(level - 1) + ".png", 170, 5);
  graphical->drawImageFile("sens.png", 170, 61); // ,  72, 72, sens
  delay(2600);

  while (!rightTracker.checkPressed());
  // while(digitalRead(13)==1)// wait until button a is pressed.............
  //int nezz = 0;

  graphical->fillScreen(TFT_BLACK);

  graphical->drawLine(0, 16, 240, 16, lightblue);
  graphical->drawLine(0, 134, 240, 134, lightblue);

  graphical->setCursor(200, 0 /*,2*/);
  graphical->print(String(brojac));

  graphical->fillRect(120, 3, 70, 7, TFT_GREEN);
  graphical->drawRect(119, 2, 72, 9, TFT_GREY);
}

void setup(void)
{

  //       pinMode(21,INPUT_PULLUP);
  //       pinMode(22,INPUT_PULLUP);
  //       pinMode(17,INPUT_PULLUP);
  //       pinMode(2,INPUT_PULLUP);
  //       pinMode(12,INPUT_PULLUP); //fire2 B
  //       pinMode(13,INPUT_PULLUP); //fire1 A
  //       pinMode(25,OUTPUT); //led2
  //       pinMode(33,OUTPUT); //led1
  //       pinMode(26,OUTPUT); //led3
  //       pinMode(15,INPUT_PULLUP); //stisak
  //       pinMode(13,INPUT_PULLUP); //buttonB
  //       pinMode(0,INPUT); //LORA built in buttons
  //       pinMode(35,INPUT);
  // digitalWrite(26,1);

  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  //pinMode(ENTER, INPUT_PULLUP);
#ifdef WITH_JOYSTICK  
  pinMode(HORIZONTAL, INPUT);
  pinMode(VERTICAL, INPUT);
#endif  

  graphical = dumbdisplay.createGraphicalLayer(240, 135);

  //  tft.init();
  // tft.setRotation(1);
  graphical->fillScreen(TFT_BLACK);
  // tft.setSwapBytes(true);


#if defined (SAVE_IMAGES)
  dumbdisplay.writeComment("start caching ...");
  dumbdisplay.writeComment("... caching back2 ...");
  graphical->cachePixelImage16(IF_BACK2/*"back2.png"*/, back2, 240, 135, "", DD_COMPRESS_BA_0);
  if (true)
  {
    dumbdisplay.recordLayerCommands();
    graphical->drawImageFile(IF_BACK2/*"back2.png"*/);
    graphical->fillRect(0, 78, 120, 25, TFT_BLACK);
    dumbdisplay.playbackLayerCommands();
  }
  dumbdisplay.writeComment("... cachine sens ...");
    graphical->cachePixelImage16(IF_SENS/*"sens.png"*/, sens, 72, 72, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine gameOver ...");
    graphical->cachePixelImage16(IF_GAMEOVER/*"gameOver.png"*/, gameOver, 240, 135, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine brod1 ...");
    graphical->cachePixelImage16(IF_BROD1/*"brod1.png"*/, brod1, 49, 40, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine bulet ...");
    graphical->cachePixelImage16(IF_BULET/*"bulet.png"*/, bulet, 8, 8, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine rocket ...");
    graphical->cachePixelImage16(IF_ROCKET/*"rocket.png"*/, rocket, 24, 12, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine ex2 ...");
    graphical->cachePixelImage16(IF_EX2/*"ex2.png"*/, ex2, 12, 12, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine explosion ...");
    graphical->cachePixelImage16(IF_EXPLOSION/*"explosion.png"*/, explosion, 24, 24, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine buum ...");
    graphical->cachePixelImage16(IF_BUUM/*"buum.png"*/, buum, 55, 55, "", DD_COMPRESS_BA_0);
  dumbdisplay.writeComment("... cachine ebullet ...");
    graphical->cachePixelImage16(IF_EBULLET/*"ebullet.png"*/, ebullet, 7, 7, "", DD_COMPRESS_BA_0);
  for (int i = 0; i < LEVEL_COUNT; i++)
  {
    int level = i + 1;
    dumbdisplay.writeComment("... caching earth-" + String(level - 1) + " ...");
      graphical->cachePixelImage16(IF_EARTH(level)/*name + ".png"*/, earth[level - 1], 55, 54, "", DD_COMPRESS_BA_0);
  }
  dumbdisplay.writeComment("... done caching");
  graphical->saveCachedImageFiles(IF_SPACEWARS_IMGS/*"spacewarsimgs.png"*/);
#else
  int x = 0;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 240, 135, IF_BACK2); x += 240;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 72, 72, IF_SENS); x += 72;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 240, 135, IF_GAMEOVER); x += 240;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 49, 40, IF_BROD1); x += 49;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 8, 8, IF_BULET); x += 8;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 24, 12, IF_ROCKET); x += 24;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 12, 12, IF_EX2); x += 12;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 24, 24, IF_EXPLOSION); x += 24;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 55, 55, IF_BUUM); x += 55;
  graphical->loadImageFileCropped(IF_SPACEWARS_IMGS, x, 0, 7, 7, IF_EBULLET); x += 7;
  for (int i = 0; i < LEVEL_COUNT; i++)
  {
    graphical->loadImageFileCropped("spacewarsimgs.png", x + i * 55, 0, 55, 54, IF_EARTH(level));
  }
#endif

  for (int i = 0; i < 30; i++)
  {
    spaceX[i] = random(5, 235);
    spaceY[i] = random(18, 132);
  }

  // graphical->drawImageFile("back2.png"/*, 0, 0, 240, 135*/);
  // while (!selectTracker.checkPressed());

  // while(digitalRead(13)==1)// wait until button a is pressed.............
  // int nezz=0;
  //  digitalWrite(26,0);

  //newLevel();
  //while (true);
}

void handleRestart() {
    restart();
    graphical->fillScreen(TFT_BLACK);
    //tft.setSwapBytes(true);
    graphical->drawImageFile(IF_BACK2);
//dumbdisplay.writeComment("1...");
    while (!rightTracker.checkPressed());
//dumbdisplay.writeComment("...1");
    // while (digitalRead(13) == 1)
    // {
    //   int nezz = 0;
    // }
    //tft.fillScreen(TFT_BLACK);
    graphical->fillScreen(TFT_BLACK);
    graphical->setCursor(0, 0/*, 4*/);
    graphical->print("Level " + String(level));
    graphical->setCursor(0, 22/*, 2*/);

    graphical->println("Enemy speed : " + String(es));
    graphical->println("Enemy health : " + String(eHealth));
    graphical->println("Enemy bullet speed : " + String(EbulletSpeed));
    graphical->println("Remaining lives: " + String(lives));
    graphical->println("My speed : " + String(sped));
    graphical->println("Rocket damage : " + String(rDamage));
    graphical->println("Rocket speed : " + String(rocketSpeed));

    graphical->drawImageFile(IF_EARTH(level), 170, 5/*, 55, 54, earth[level - 1]*/);
    graphical->drawImageFile(IF_SENS, 170, 61/*, 72, 72, sens*/);
    delay(1000);

//dumbdisplay.writeComment("2...");
    while (!rightTracker.checkPressed());
//dumbdisplay.writeComment("...2");
    // while (digitalRead(13) == 1) // wait until button a is pressed.............
    //   int nezz = 0;

    graphical->fillScreen(TFT_BLACK);

    graphical->drawLine(0, 16, 240, 16, lightblue);
    graphical->drawLine(0, 134, 240, 134, lightblue);

    graphical->setCursor(200, 0/*, 2*/);
    graphical->print(String(brojac));

    graphical->fillRect(120, 3, 70, 7, TFT_GREEN);
    graphical->drawRect(119, 2, 72, 9, TFT_GREY);

    fase = 1;

}

void loop()
{
  if (fase == 0)
  {
    handleRestart();
  }
//dumbdisplay.writeComment("???");

  if (fase == 1)
  {                                      // playing fase
#if defined(WITH_JOYSTICK)  
    int8_t horizontalPress = horizontalTracker.checkPressed();
    int8_t verticalPress = verticalTracker.checkPressed();

    if (verticalPress == 1 && y < 94) // Move down
      y = y + sped;

    if (verticalPress == -1 && y > 18) // Move up
      y = y - sped;

    if (horizontalPress == 1 && x < 125) // Move right
      x = x + sped;

    if (horizontalPress == -1 && x > 0) // Move left
      x = x - sped;
#endif

    if (/*digitalRead(13) == 0*/rightTracker.checkPressed()) // fire button A button
    {
      if (pom == 0)
      {
        pom = 1;

        buletXY[counter].moveTo(xy.getX() + 34, xy.getY() + 15);
        //buletY[counter] = xy.getY() + 15;
        counter = counter + 1;
      }
    }
    else
      pom = 0;

    if (/*digitalRead(12) == 0*/leftTracker.checkPressed() && rockets > 0) // Rocket button B button
    {
      if (pom2 == 0)
      {
        pom2 = 1;
        rockets--;
        rocketX[rcounter] = xy.getX() + 34;
        rocketY[rcounter] = xy.getY() + 14;
        rcounter = rcounter + 1;
        ri[rockets] = -100;
        graphical->fillRect(70 + (rockets * 14), 0, 8, 14, TFT_BLACK);
      }
    }
    else
      pom2 = 0;

/*
    if (digitalRead(35) == 0) // buton 35 , on and off sound
    {
      if (pom3 == 0)
      {
        pom3 = 1;
        sound = !sound;
      }
    }
    else
      pom3 = 0;
*/

    // for (int i = 0; i < 30; i++)
    // { // drawStars..........................................
    //   graphical->drawPixel(spaceX[i], spaceY[i], TFT_BLACK);
    //   spaceX[i] = spaceX[i] - 0.5;
    //   graphical->drawPixel(spaceX[i], spaceY[i], TFT_GREY);
    //   if (spaceX[i] < 0)
    //   {
    //     graphical->drawPixel(spaceX[i], spaceY[i], TFT_BLACK);

    //     spaceX[i] = 244;
    //   }
    // }

    //tft.pushImage(x, y, 49, 40, brod1);
    //tft.pushImage(ex, ey, 55, 54, earth[level - 1]);
    if (xy.checkMoved()) {
      graphical->drawImageFile(IF_BROD1, xy.getX(), xy.getY());
    }
    if (exy.checkMoved()) {
      graphical->drawImageFile(IF_EARTH(level), exy.getX(), exy.getY());
    }

    for (int i = 0; i < 10; i++)
    { // firing buletts
      if (buletXY[i].getX() > 0)
      {
        //tft.pushImage(c, 8, 8, bulet);
        graphical->drawImageFile(IF_BULET, buletXY[i].getX(), buletXY[i].getY());
        buletXY[i].moveBy(0.6, 0);
      }
      if (buletXY[i].getX() > 240)
        buletXY[i].moveXTo(-30);
    }

    for (int i = 0; i < 10; i++)
    { // firing rockets
      if (rocketX[i] > 0)
      {
        //tft.pushImage(rocketX[i], rocketY[i], 24, 12, rocket);
        graphical->drawImageFile(IF_ROCKET, rocketX[i], rocketY[i]);
        rocketX[i] = rocketX[i] + rocketSpeed;
      }
      if (rocketX[i] > 240)
        rocketX[i] = -30;
    }

    // delay(1);

    for (int j = 0; j < 10; j++) // did my bulet hit enemy
    {
      if (buletXY[j].getX() > exy.getX() + 20 && buletXY[j].getY() > exy.getY() + 2 && buletXY[j].getY() < exy.getY() + 52)
      {
        //tft.pushImage(buletX[j], buletY[j], 12, 12, ex2);
        graphical->drawImageFile(IF_EX2, buletXY[j].getX(), buletXY[j].getY());
        if (sound == 1)
        {
          //tone(BUZZER_PIN, NOTE_C5, 12, BUZZER_CHANNEL);
          dumbdisplay.tone(NOTE_C5, 12);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
        }
        else
        {
          delay(12);
        }
        graphical->fillRect(buletXY[j].getX(), buletXY[j].getY(), 12, 12, TFT_BLACK);
        buletXY[j].moveXTo(-50);
        brojac = brojac + 1;
        graphical->setCursor(200, 0/*, 2*/);
        graphical->print(String(brojac));
        eHealth--;
        tr = map(eHealth, 0, mHealth, 0, 70);
        graphical->fillRect(120, 3, 70, 7, TFT_BLACK);
        graphical->fillRect(120, 3, tr, 7, TFT_GREEN);

        if (eHealth <= 0)
        {
          //tft.pushImage(ex, ey, 55, 55, buum);
          graphical->drawImageFile(IF_BUUM, exy.getX(), exy.getY());
          dumbdisplay.tone(NOTE_E4, 100);
          dumbdisplay.tone(NOTE_D4, 80);
          dumbdisplay.tone(NOTE_G5, 100);
          dumbdisplay.tone(NOTE_C3, 80);
          dumbdisplay.tone(NOTE_F4, 280);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
          delay(700);
          newLevel();
        }
        /*
        digitalWrite(25, 1);
        */
        //blinkTime = 1;
      }
    }

    for (int j = 0; j < 10; j++) // did my ROCKET hit enemy
    {
      if (rocketX[j] + 18 > exy.getX() && rocketY[j] > exy.getY() + 2 && rocketY[j] < exy.getY() + 52)
      {
        //tft.pushImage(rocketX[j], rocketY[j], 24, 24, explosion);
        graphical->drawImageFile(IF_EXPLOSION, rocketX[j], rocketY[j]);
        if (sound == 1)
        {
          dumbdisplay.tone(NOTE_C3, 40);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
        }
        else
        {
          delay(40);
        }
        graphical->fillRect(rocketX[j], rocketY[j], 24, 24, TFT_BLACK);
        // delay(30);

        rocketX[j] = -50;
        brojac = brojac + 12;
        graphical->setCursor(200, 0/*, 2*/);
        graphical->print(String(brojac));
        eHealth = eHealth - rDamage;
        tr = map(eHealth, 0, mHealth, 0, 70);
        graphical->fillRect(120, 3, 70, 7, TFT_BLACK);
        graphical->fillRect(120, 3, tr, 7, TFT_GREEN);

        if (eHealth <= 0)
        {
          //tft.pushImage(ex, ey, 55, 55, buum);
          graphical->drawImageFile(IF_BUUM, exy.getX(), exy.getY());
          dumbdisplay.tone(NOTE_E4, 100);
          dumbdisplay.tone(NOTE_D4, 80);
          dumbdisplay.tone(NOTE_G5, 100);
          dumbdisplay.tone(NOTE_C3, 80);
          dumbdisplay.tone(NOTE_F4, 280);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
          delay(700);
          newLevel();
        }
        //digitalWrite(25, 0);
        //blinkTime = 1;
      }
    }

    for (int j = 0; j < 10; j++) // Am I hit
    {
      if (EbuletX[j] < xy.getX() + 30 && EbuletX[j] > xy.getX() + 4 && EbuletY[j] > xy.getY() + 4 && EbuletY[j] < xy.getY() + 36)
      {
        EbuletX[j] = -50;
        ly[lives - 1] = -40;
        graphical->fillRect((lives - 1) * 14, 0, 14, 14, TFT_BLACK);
        lives--;
        if (lives == 0)
        {
          //tft.pushImage(x, y, 55, 55, buum);
          graphical->drawImageFile(IF_BUUM, xy.getX(), xy.getY());
          dumbdisplay.tone(NOTE_G4, 100);
          dumbdisplay.tone(NOTE_B4, 80);
          dumbdisplay.tone(NOTE_C5, 100);
          dumbdisplay.tone(NOTE_A4, 80);
          dumbdisplay.tone(NOTE_F4, 280);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
          delay(500);
          graphical->fillScreen(TFT_BLACK);
          fase = 2;
        }

        /*
        digitalWrite(33, 1);
        */
        //blinkTime = 1;
        if (sound == 1)
        {
          dumbdisplay.tone(NOTE_C6, 4);
          //noTone(BUZZER_PIN, BUZZER_CHANNEL);
        }
        else
        {
          delay(4);
        }
      }
    }

    exy.moveBy(0, es);
    //ey = ey + es;
    if (exy.getY() > 80)
      es = es * -1;

    if (exy.getY() < 18)
      es = es * -1;

    // if (blinkTime > 0)
    //   blinkTime++;

    // if (blinkTime > 2)
    // {
    //   digitalWrite(25, 0);
    //   digitalWrite(33, 0);
    //   blinkTime = 0;
    // }

    for (int i = 0; i < 10; i++)
    { // enemy shoots
      if (EbuletX[i] > -10)
      {
        //tft.pushImage(EbuletX[i], EbuletY[i], 7, 7, ebullet);
        graphical->drawImageFile(IF_EBULLET, EbuletX[i], EbuletY[i]);
        EbuletX[i] = EbuletX[i] - EbulletSpeed;
      }
    }

    // for (int i = 0; i < 4; i++) // draw lifes
    //   tft.pushImage(i * 14, ly[i], 12, 11, life);
    // for (int i = 0; i < 3; i++) // draw lifes
    //   tft.pushImage(70 + (i * 14), ri[i], 8, 14, ricon);

    fireCount++;
    if (fireTime == fireCount)
    {
      // EbuletX[Ecounter] = exy.getX() + 5;
      // EbuletY[Ecounter] = exy.getY() + 24;
      // fireCount = 0;
      // fireTime = random(110 - (level * 15), 360 - (level * 30));
      // Ecounter++;
    }
    if (counter == 9)
      counter = 0;

    if (rcounter == 3)
      rcounter = 0;

    if (Ecounter == 9)
      Ecounter = 0;
  }
  if (fase == 2) // game over fase
  {

    graphical->fillScreen(TFT_BLACK);
    //graphical->pushImage(0, 0, 240, 135, gameOver);
    graphical->drawImageFile(IF_GAMEOVER);
    graphical->setCursor(24, 54/*, 2*/);
    graphical->print("Score : " + String(brojac));
    graphical->setCursor(24, 69/*, 2*/);
    graphical->print("Level : " + String(level));
    while (!rightTracker.checkPressed());
    // while (digitalRead(13) == 1)
    // {
    //   int nezz = 0;
    // }

    fase = 0;
  }
}
