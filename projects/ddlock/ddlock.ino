#include <stdio.h>

#if defined(ESP32)
  // if using ESP32, assume you can use bluetooth with name ESP32
  //#define KEYPAD_4_COLS
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true, 115200));
#else
  #define KEYPAD_4_COLS
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


LcdDDLayer *lcd;
GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;



#if defined(ESP32)
  #define PIN_R0  GPIO_NUM_13
  #define PIN_R1  GPIO_NUM_12
  #define PIN_R2  GPIO_NUM_14
  #define PIN_R3  GPIO_NUM_27
  #define PIN_C0  GPIO_NUM_26
  #define PIN_C1  GPIO_NUM_25 
  #define PIN_C2  GPIO_NUM_33
  #define PIN_C3  GPIO_NUM_32
#else
  // #define PIN_R0 12
  // #define PIN_R1 11
  // #define PIN_R2 10
  // #define PIN_R3  9
  // #define PIN_C0  8
  // #define PIN_C1  7 
  // #define PIN_C2  6
  // #define PIN_C3  5
#endif
const char keys[4][4] = {
  { '1', '2', '3', 'A'},
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D'}
};
void setupForReadingRow(int r) {
#if defined(PIN_R0)  
    digitalWrite(PIN_R0, r != 0);
    digitalWrite(PIN_R1, r != 1);
    digitalWrite(PIN_R2, r != 2);
    digitalWrite(PIN_R3, r != 3);
#endif
}
int readColumnOfKeyPress() {
#if defined(PIN_R0)  
  int c0 = digitalRead(PIN_C0);
  int c1 = digitalRead(PIN_C1);
  int c2 = digitalRead(PIN_C2);
#ifdef KEYPAD_4_COLS
  int c3 = digitalRead(PIN_C3);
#else
  int c3 = 1;
#endif
  int c = -1;
  if (c0 == 0) {
    c = 0;
  } else if (c1 == 0) {
    c = 1;
  } else if (c2 == 0) {
    c = 2;
  } else if (c3 == 0) {
    c = 3;
  }
  return c;
#else
  return -1;
#endif
}
long lastKeyMillis = 0;
char getAKeyPress() {
  long nowMillis = millis();
  if ((nowMillis - lastKeyMillis) < 200) {
    return 0;
  }
  for (int r = 0; r < 4; r++) {
    setupForReadingRow(r);
    int c = readColumnOfKeyPress();
    if (c != -1) {
      char key = keys[r][c];
      lastKeyMillis = nowMillis;
      return key;
    }
  }
  return 0;   
}


const int maxComboSize = 4;
char combinationSize = 0;
char combinations[maxComboSize];  // maximum 4 chars
char comboBuffer[maxComboSize + 1];
char* getCombo() {
    int i = 0;
    for (; i < combinationSize; i++) {
      comboBuffer[i] = combinations[i];
    }
    for (; i < maxComboSize; i++) {
      comboBuffer[i] = '_';
    }
    comboBuffer[maxComboSize] = 0;
    return comboBuffer;
}
void setComboKey(char key) {
  if (combinationSize == maxComboSize) {
    memmove(combinations, combinations + 1, combinationSize - 1);
    combinationSize--;
  }
  combinations[combinationSize++] = key;
} 





void setup() {
#if defined(PIN_R0)  
  pinMode(PIN_R0, OUTPUT);
  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);
  pinMode(PIN_R3, OUTPUT);
  pinMode(PIN_C0, INPUT_PULLUP);
  pinMode(PIN_C1, INPUT_PULLUP);
  pinMode(PIN_C2, INPUT_PULLUP);
#ifdef KEYPAD_4_COLS
  pinMode(PIN_C3, INPUT_PULLUP);
#endif
#endif

  dumbdisplay.recordLayerSetupCommands();

  // create a LCD layer for display some info about the picture shown
  lcd = dumbdisplay.createLcdLayer(16, 2);
  lcd->bgPixelColor("darkgray");
  lcd->pixelColor("lightblue");
  lcd->border(2, "blue");
  lcd->writeCenteredLine("... ...");

  // create a graphical layer for drawing the web images to
  graphical = dumbdisplay.createGraphicalLayer(200, 300);
  graphical->padding(0);
  graphical->margin(2);
  graphical->border(2, "darkgreen");
  graphical->enableFeedback();

  // auto "pin" the two layers vertically, one above the other
  dumbdisplay.configAutoPin(DD_AP_VERT);

  dumbdisplay.playbackLayerSetupCommands("rc-lock");

  // create tunnels for downloading web images ... and save to your phone
  // tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", "lock-unlocked.png");
  // tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", "lock-locked.png");
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://shorturl.at/msxVZ", "lock-unlocked.png");
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://shorturl.at/dpHSW", "lock-locked.png");
  // tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://t.ly/_QyV", "lock-unlocked.png");
  // tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://t.ly/7CTt", "lock-locked.png");
}



bool locked = false;

bool allReady = false;
long lastShownMillis = 0;

DDConnectVersionTracker cvTracker;


void loop() {
  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int lockedResult = tunnel_locked->checkResult();
  int unlockedResult = tunnel_unlocked->checkResult();
  const DDFeedback* feedback = graphical->getFeedback();
 
  bool refresh = false;

  if (!allReady) {
    // not all ready ... if not refresh for a while (1000 ms), refresh
    long now = millis();
    long diff = now - lastShownMillis;
    if (diff > 1000) {
      locked = !locked;
      refresh = true;
    }
  } else {
    // if all ready ... see if double clicked ... if so, refresh
    if (feedback != NULL) {
      if (feedback->type == DOUBLECLICK) {
        locked = !locked;
        refresh = true;
      }
    }
  }

  if (!refresh) {
    // check if reconnected ... if so, refresh
    if (cvTracker.checkChanged(dumbdisplay)) {
      refresh = true;
    } 
  }
  

  char key = getAKeyPress();
  if (key != 0) {
    //dumbdisplay.writeComment(String(key));
    setComboKey(key);
    refresh = true;
  }

  if (!refresh) {
    return;
  }


  allReady = lockedResult == 1 && unlockedResult == 1;

  int result;
  const char* imageFileName;
  if (locked) {
    imageFileName = "lock-locked.png";
    result = lockedResult;
  } else {
    imageFileName = "lock-unlocked.png";
    result = unlockedResult;
  }
  if (result == 1) {
    graphical->drawImageFile(imageFileName);
    lcd->writeCenteredLine(locked ? "Lock locked" : "Lock unlocked");
    lcd->writeCenteredLine(allReady ? getCombo() : "...", 1);
  } else if (result == 0) {
    // downloading
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("... ...");
    graphical->println(imageFileName);
    graphical->println("... ...");
    lcd->writeCenteredLine("...");
  } else if (result == -1) {
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("XXX failed to download XXX");
    lcd->writeCenteredLine("failed");
  }

  lastShownMillis = millis();
}
