#define PIN_R1 8
#define PIN_R2 7
#define PIN_R3 6
#define PIN_R4 5
#define PIN_C1 4
#define PIN_C2 3 
#define PIN_C3 2

#define SS_PIN 10
#define RST_PIN 9

#define SIM_DOOR_LED_PIN A5

#define STORE_IN_EEPROM


#define TONE_KEY 1500
#define TONE_RFID 1400
#define TONE_UNLOCK 1000
#define TONE_LOCK 800
#define TONE_FAIL 500



#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));




LcdDDLayer *lcd;
GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;
DDConnectVersionTracker cvTracker;


const char Keys[4][4] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
void setupForReadingRow(int r) {
    digitalWrite(PIN_R1, r != 0);
    digitalWrite(PIN_R2, r != 1);
    digitalWrite(PIN_R3, r != 2);
    digitalWrite(PIN_R4, r != 3);
}
int readColumnOfKeyPress() {
  int c0 = digitalRead(PIN_C1);
  int c1 = digitalRead(PIN_C2);
  int c2 = digitalRead(PIN_C3);
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
      char key = Keys[r][c];
      lastKeyMillis = nowMillis;
      return key;
    }
  }
  return 0;   
}


const int MaxComboSize = 4;
struct Combo {
  int combinationSize = 0;
  char combinations[MaxComboSize];  // maximum 4 chars
};
char ComboBuffer[MaxComboSize + 1];
char* getCombo(const Combo& combo) {
    int i = 0;
    for (; i < combo.combinationSize; i++) {
      ComboBuffer[i] = combo.combinations[i];
    }
    for (; i < MaxComboSize; i++) {
      ComboBuffer[i] = '_';
    }
    ComboBuffer[MaxComboSize] = 0;
    return ComboBuffer;
}
void addComboKey(Combo &combo, char key) {
  if (combo.combinationSize == MaxComboSize) {
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


#if defined (SS_PIN)
#include <SPI.h>
#include <MFRC522.h>
class RFIDControl
{
public:
  RFIDControl(uint8_t ssPin, uint8_t rstPin): mfrc522(ssPin, rstPin) {
    noRFIDMillis = millis();
  }
public:
  void init() {
    SPI.begin();
    mfrc522.PCD_Init();
  }  
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
public:
  String rfidPass;
private: 
  MFRC522 mfrc522;
  String rfidRead;
  long noRFIDMillis;
};
RFIDControl rfidControl(SS_PIN, RST_PIN);
#endif

#if defined(STORE_IN_EEPROM)
#include <EEPROM.h>
class EEPROMStorage {
public:
  void readCombo(Combo& combo) {
    int offset = 0;
    // check header first
    for (uint8_t i = 0; i < 10; i++) {
      uint8_t r = EEPROM.read(offset + i);
      if (r != i) {
        return;
      }
    }
    combo.combinationSize = EEPROM.read(offset + 10);
    for (int i = 0; i < MaxComboSize; i++) {
      combo.combinations[i] = EEPROM.read(offset + 11 + i);
    }
  }
  void writeCombo(const Combo& combo) {
    int offset = 0;
    // write some header
    for (uint8_t i = 0; i < 10; i++) {
      EEPROM.write(offset + i, i);
    }
    EEPROM.write(offset + 10, combo.combinationSize);
    for (int i = 0; i < MaxComboSize; i++) {
      EEPROM.write(offset + 11 + i, combo.combinations[i]);
    }
  }
  void readRFID(String& rfid) {
    int offset = 30;
    // check header first
    for (uint8_t i = 0; i < 10; i++) {
      uint8_t r = EEPROM.read(offset + i);
      if (r != i) {
        return;
      }
    }
    uint8_t count = EEPROM.read(offset + 10);
    char buffer[count + 1];
    for (int i = 0; i < count; i++) {
      buffer[i] = EEPROM.read(offset + 11 + i);
    }
    buffer[count] = 0;
    rfid = String(buffer);
  }
  void writeRFID(const char* rfid) {
    int offset = 30;
    // write some header
    for (uint8_t i = 0; i < 10; i++) {
      EEPROM.write(offset + i, i);
    }
    int len = strlen(rfid);
    EEPROM.write(offset + 10, len);
    for (int i = 0; i < len; i++) {
      EEPROM.write(offset + 11 + i, rfid[i]);
    }
  }
};
EEPROMStorage eepromStorage;
#endif


#if defined(SIM_DOOR_LED_PIN)
class DoorHandler {
public:
  DoorHandler(uint8_t ledPin): ledPin(ledPin) {
    toggleCounter = 0;
  }
  // doorState -- 0: not changed; 1: opened; 2: closed
  void handleDoor(int doorState) {
    if (doorState == 1) {
      if (!opened) {
        //dumbdisplay.writeComment("OPEN");
        toggleCounter = 3;
        //toggleMillis = 0;
      }
      opened = true;
    } else if (doorState == 2) {
      if (opened) {
        //dumbdisplay.writeComment("CLOSE");
        toggleCounter = 3;
        //toggleMillis = 0;
      }
      opened = false;
    } else {
      if (toggleCounter > 0) {
        long nowMillis = millis();
        if ((toggleMillis - nowMillis) <= 0) {
          if (toggleCounter % 2 == 0) {
            digitalWrite(ledPin, opened ? 0 : 1);
          } else {
            digitalWrite(ledPin, opened ? 1 : 0);
          }
          if (--toggleCounter > 0) {
            toggleMillis = nowMillis + 150;
          } else {
            toggleMillis = 0;
          }
        }
      }
    }
  }  
private:
  uint8_t ledPin;
  bool opened;
  long toggleMillis;
  int toggleCounter;
};
DoorHandler doorHandler(SIM_DOOR_LED_PIN);
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
  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);
  pinMode(PIN_R3, OUTPUT);
  pinMode(PIN_R4, OUTPUT);
  pinMode(PIN_C1, INPUT_PULLUP);
  pinMode(PIN_C2, INPUT_PULLUP);
  pinMode(PIN_C3, INPUT_PULLUP);
#if defined(PIN_C3)  
  pinMode(PIN_C3, INPUT_PULLUP);
#endif

#if defined(SIM_DOOR_LED_PIN)
 pinMode(SIM_DOOR_LED_PIN, OUTPUT);
#endif

#if defined (SS_PIN)
  rfidControl.init();
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
  // tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", "lock-unlocked.png", false);
  // tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", "lock-locked.png", false);
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://${DDSS}/lock-unlocked.png", "lock-unlocked.png", false);
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://${DDSS}/lock-locked.png", "lock-locked.png", false);

#if defined(STORE_IN_EEPROM)
  eepromStorage.readCombo(lockCombo);
 #if defined (SS_PIN)
  eepromStorage.readRFID(rfidControl.rfidPass);
  dumbdisplay.writeComment("STORED RFID: " + rfidControl.rfidPass);
 #endif
#endif
}


void loop() {
  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int lockedResult = tunnel_locked->checkResult();
  int unlockedResult = tunnel_unlocked->checkResult();
  const DDFeedback* feedback = graphical->getFeedback();
 
  bool refresh = false;
  bool wasLocked = locked;

  if (!allReady) {
    // not all ready ... if not refreshed for a while (1000 ms), refresh
    long now = millis();
    long diff = now - lastShownMillis;
    if (diff > 1000) {
      locked = !locked;
      refresh = true;
    }
  } else {
    // if all ready ... see if double clicked ... if so, handle and refresh
    if (feedback != NULL) {
      if (feedback->type == DDFeedbackType::DOUBLECLICK) {
        if (locked) {
          // if locked ... check if combo matches ... if so, unlock
          onTried((matchCombos(lockCombo, enteringCombo)), true);
        } else {
          // if not locked ... lock
          dumbdisplay.tone(TONE_LOCK, 200);
          locked = true;
          resetCombo(enteringCombo);
        }
        refresh = true;
      }
    }
    char key = getAKeyPress();
    if (key != 0) {
      // a key on keypad pressed
      dumbdisplay.tone(TONE_KEY, 100);
      if (locked) {
        // already locked ... handled entering of combo
        if (key == '*') {
          // if key is '*', clear combo being entered
          resetCombo(enteringCombo);
        } else  if (key != '#') {
          // if key is not '#', add the key to what combo being entered
          addComboKey(enteringCombo, key);
        }
        // try the combo
        onTried((matchCombos(lockCombo, enteringCombo)), key == '#');
      } else {
        // not locked ... handle if changing combo
        if (key == '*') {
          // if key is '*', start changing combo
          renewing = true;
          resetCombo(newCombo);
          dumbdisplay.tone(TONE_UNLOCK, 200);
          dumbdisplay.tone(TONE_KEY, 100);
        } else {
          if (renewing) {
            if (key == '#') {
              // if changing combo and key pressed is '#', use the new combo
              copyCombos(newCombo, lockCombo);
              resetCombo(enteringCombo);
              resetCombo(newCombo);
              onNew();
#if defined(STORE_IN_EEPROM)
              eepromStorage.writeCombo(lockCombo);
#endif
            } else {
              // add the key to the new combo being entered
              addComboKey(newCombo, key);
            }
          }
        }
      }
      refresh = true;
    }
  }
  

#if defined (SS_PIN)
    const char* rfid = rfidControl.readRFID();
    if (rfid != NULL) {
      // got RFID
      dumbdisplay.writeComment("RFID: " + String(rfid));
      dumbdisplay.tone(TONE_RFID, 100);
      if (allReady) {
        if (locked) {
          // if already locked ... try to match the RFID, to see if can unlock
          onTried(rfidControl.matchRFIDPass(rfid), true);
        } else {
          if (renewing) {
            // if renewing combo and got RFID ... use the RFID as new RFID pass  
            rfidControl.rfidPass = String(rfid);
            onNew();
#if defined(STORE_IN_EEPROM)
            eepromStorage.writeRFID(rfid);
#endif
          }
        }
        refresh = true;
      }
    }
#endif

#if defined(SIM_DOOR_LED_PIN)
  int doorState = 0;  // door state not changed
  if (wasLocked && !locked) {
    doorState = 1;  // door should open now
  } else if (!wasLocked && locked) {
    doorState = 2;  // door should close now
  }
  doorHandler.handleDoor(doorState);
#endif

  if (!refresh) {
    // check if reconnected ... if so, refresh anyway
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
