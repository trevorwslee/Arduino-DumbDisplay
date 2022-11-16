#include <stdio.h>

#if defined(ESP32)
  #define PIN_R0  GPIO_NUM_13
  #define PIN_R1  GPIO_NUM_12
  #define PIN_R2  GPIO_NUM_14
  #define PIN_R3  GPIO_NUM_27
  #define PIN_C0  GPIO_NUM_26
  #define PIN_C1  GPIO_NUM_25 
  #define PIN_C2  GPIO_NUM_33
  #define PIN_C3  GPIO_NUM_32
  // if using ESP32, assume you can use bluetooth with name ESP32
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true, 115200));
#else
  #define PIN_R0 8
  #define PIN_R1 7
  #define PIN_R2 6
  #define PIN_R3 5
  #define PIN_C0 4
  #define PIN_C1 3 
  #define PIN_C2 2
  #define SS_PIN 10
  #define RST_PIN 9
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


#define TONE_KEY 1500
#define TONE_RFID 1400
#define TONE_UNLOCK 1000
#define TONE_LOCK 800
#define TONE_FAIL 500


LcdDDLayer *lcd;
GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;


const char keys[4][4] = {
  { '1', '2', '3', 'A'},
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D'}
};
void setupForReadingRow(int r) {
    digitalWrite(PIN_R0, r != 0);
    digitalWrite(PIN_R1, r != 1);
    digitalWrite(PIN_R2, r != 2);
    digitalWrite(PIN_R3, r != 3);
}
int readColumnOfKeyPress() {
  int c0 = digitalRead(PIN_C0);
  int c1 = digitalRead(PIN_C1);
  int c2 = digitalRead(PIN_C2);
#if defined(PIN_C3)  
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
}
long lastKeyMillis = 0;
char getAKeyPress() {
  long nowMillis = millis();
  if ((nowMillis - lastKeyMillis) < 300) {
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

struct Combo {
  int combinationSize = 0;
  char combinations[maxComboSize];  // maximum 4 chars
};
char comboBuffer[maxComboSize + 1];
char* getCombo(const Combo& combo) {
    int i = 0;
    for (; i < combo.combinationSize; i++) {
      comboBuffer[i] = combo.combinations[i];
    }
    for (; i < maxComboSize; i++) {
      comboBuffer[i] = '_';
    }
    comboBuffer[maxComboSize] = 0;
    return comboBuffer;
}
void setComboKey(Combo &combo, char key) {
  if (combo.combinationSize == maxComboSize) {
    memmove(combo.combinations, combo.combinations + 1, combo.combinationSize - 1);
    combo.combinationSize--;
  }
  combo.combinations[combo.combinationSize++] = key;
} 
void resetCombo(Combo &combo) {
  combo.combinationSize = 0;
}
bool matchCombos(const Combo& combo1, const Combo& combo2) {
  if (combo1.combinationSize != combo2.combinationSize) {
    return false;
  }
  for (int i = 0; i < combo1.combinationSize; i++) {
    if (combo1.combinations[i] != combo2.combinations[i]) {
      return false;
    }
  }
  return true;
}
void copyCombos(const Combo& fromCombo, Combo& toCombo) {
  toCombo.combinationSize = fromCombo.combinationSize;
  memcpy(toCombo.combinations, fromCombo.combinations, toCombo.combinationSize);
}

Combo lockCombo;
Combo enteringCombo;
Combo newCombo;


bool allReady = false;
bool locked = false;
bool renewing = false;
long lastShownMillis = 0;

DDConnectVersionTracker cvTracker;


#if defined (SS_PIN)
#include <SPI.h>
#include <MFRC522.h>
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

String rfidRead;
long noRFIDMillis = millis();

const char* readRFID() {
  long nowMillis = millis();
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    char buffer[16];
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      sprintf(buffer + 2 * i, "%02x", mfrc522.uid.uidByte[i]);
    }
    bool accept = noRFIDMillis > 0 && (nowMillis - noRFIDMillis) >= 500;
//accept = true;
    noRFIDMillis = 0;
    if (accept) {
      rfidRead = buffer;
      return rfidRead.c_str();
    } else {
      return NULL;
    }
  } else {
    if (noRFIDMillis <= 0) {
      noRFIDMillis = nowMillis;
    }
    return NULL;
  }
}

String rfidPass;
bool matchRFIDPass(const char* rfid) {
  const char* csRFIDPass = rfidPass.c_str();
  for (int i = 0;; i++) {
    char p = csRFIDPass[i];
    char c = rfid[i];
    if (p == 0 && c == 0) {
      break;
    }
    if (p == 0 || c == 0 || p != c) {
      return false;
    }
  }
  return true;
}
#endif


void onTried(bool matched, bool warnIfNoMatch) {
  if (matched) {
    dumbdisplay.tone(TONE_UNLOCK, 200);
    locked = false;
    renewing = false;
  } else if (warnIfNoMatch) {
    dumbdisplay.tone(TONE_FAIL, 100);
    dumbdisplay.tone(TONE_LOCK, 100);
    dumbdisplay.tone(TONE_FAIL, 200);
  }
}
void onNew() {
  dumbdisplay.tone(TONE_LOCK, 100);
  dumbdisplay.tone(TONE_UNLOCK, 100);
  locked = true;
  renewing = false; 
}




void setup() {
  pinMode(PIN_R0, OUTPUT);
  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);
  pinMode(PIN_R3, OUTPUT);
  pinMode(PIN_C0, INPUT_PULLUP);
  pinMode(PIN_C1, INPUT_PULLUP);
  pinMode(PIN_C2, INPUT_PULLUP);
#if defined(PIN_C3)  
  pinMode(PIN_C3, INPUT_PULLUP);
#endif

#if defined (SS_PIN)
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
	//delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
#endif

  dumbdisplay.recordLayerSetupCommands();

  // create a LCD layer for display some info about the picture shown
  lcd = dumbdisplay.createLcdLayer(16, 3);
  //lcd->backgroundColor(DD_RGB_COLOR(0xee, 0xee, 0xee));
  //lcd->pixelColor("darkblue");
  lcd->border(2, "black");
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
        if (locked) {
          onTried((matchCombos(lockCombo, enteringCombo)), true);
        } else {
          dumbdisplay.tone(TONE_LOCK, 200);
          locked = true;
          resetCombo(enteringCombo);
        }
        refresh = true;
      }
    }
    char key = getAKeyPress();
    if (key != 0) {
      dumbdisplay.writeComment(String(key));
      dumbdisplay.tone(TONE_KEY, 100);
      if (locked) {
        if (key == '*') {
          resetCombo(enteringCombo);
        } else  if (key != '#') {
          setComboKey(enteringCombo, key);
        }
        onTried((matchCombos(lockCombo, enteringCombo)), key == '#');
      } else {
        if (key == '*') {
          renewing = true;
          resetCombo(newCombo);
        } else {
          if (renewing) {
            if (key == '#') {
              copyCombos(newCombo, lockCombo);
              onNew();
            } else {
              setComboKey(newCombo, key);
            }
          }
        }
      }
      refresh = true;
    }
  }

#if defined (SS_PIN)
    //String rfid;
    const char* rfid = readRFID();
    if (rfid != NULL) {
      dumbdisplay.writeComment("RFID: " + String(rfid));
      dumbdisplay.tone(TONE_RFID, 100);
      if (allReady) {
        if (locked) {
          onTried(matchRFIDPass(rfid), true);
        } else {
            if (renewing) {
              rfidPass = String(rfid);
              onNew();
            }
        }
        refresh = true;
      }
    }
#endif


  if (!refresh) {
    // check if reconnected ... if so, refresh
    if (cvTracker.checkChanged(dumbdisplay)) {
      refresh = true;
    } 
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
    String status1;
    String status2;
    if (allReady) {
      status1 = "🔒  " + String(getCombo(lockCombo));
      if (locked) {
        status2 = "🔑  " + String(getCombo(enteringCombo));
        lcd->backgroundColor(DD_RGB_COLOR(0xff, 0xdd, 0xdd));
        lcd->pixelColor("darkred");
      } else {
        if (renewing) {
          status2 = "🆕  " + String(getCombo(newCombo));
          lcd->backgroundColor(DD_RGB_COLOR(0xdd, 0xdd, 0xff));
          lcd->pixelColor("darkblue");
        } else {
           status2 = "";
          lcd->backgroundColor(DD_RGB_COLOR(0xdd, 0xff, 0xdd));
          lcd->pixelColor("darkgreen");
        }  
      }
    } else {
      status1 = "not";
      status2 = "ready";
    }
    graphical->drawImageFile(imageFileName);
    lcd->clear();
    lcd->writeCenteredLine(locked ? "Lock locked" : "Lock unlocked");
    lcd->writeCenteredLine(status1, 1);
    lcd->writeCenteredLine(status2, 2);
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
