// comment out SPEAKER_PIN if not actually connected to speaker
// . if no special connected, will use DumbDisplay to play tones
#define SPEAKER_PIN 5


// comment out BLUETOOTH if don't have HC-06 connectivity
// . GP8 => RX of HC-06; GP9 => TX of HC-06
// if no HC-06 connectivity, will need to use DumbDisplayWifiBridge
#define BLUETOOTH


#ifdef BLUETOOTH

// GP8 => RX of HC-06; GP9 => TX of HC-06
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include <picodumbdisplay.h>
DumbDisplay dumbdisplay(new DDPicoUart1IO(115200));

#else

#include <dumbdisplay.h>
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif



const int8_t TOP_HEIGHT = 30;
const int8_t WIDTH = 14;
const int8_t HEIGHT = 80;
const int8_t BORDER = 1;



// 440 is A
int GetNoteFreq(int8_t octaveIdx, int8_t nodeIdx) {
  if (nodeIdx == -99) return 0;  // -99 is a special value that signal no note
  int8_t n = nodeIdx + 12 * octaveIdx - 8;
  float freq = 440.0 * pow(2, n / 12.0);
  return round(freq);
}

int8_t ToNoteIdx(char nodeName, char semiTone) {
  if (nodeName == '0') return -99;
  int8_t nodeIdx;
  switch (nodeName) {
    case 'C': nodeIdx = 0; break;
    case 'D': nodeIdx = 2; break;
    case 'E': nodeIdx = 4; break;
    case 'F': nodeIdx = 5; break;
    case 'G': nodeIdx = 7; break;
    case 'A': nodeIdx = 9; break;
    case 'B': nodeIdx = 11; break;
  }
  if (semiTone == '#') {
    nodeIdx = nodeIdx + 1; 
  } else if (semiTone = 'b') {
    nodeIdx = nodeIdx - 1;
  }
  return nodeIdx;
}

void PlayTone(int freq, int duration, bool playToSpeaker) {
#ifdef SPEAKER_PIN
  if (playToSpeaker) {
    long tillMicros = 1000 * (millis() + duration);
    long waitMicros = (1000000.0 / freq) / 2;
    bool high = true;
    while (true) {
      long diffMicros = tillMicros - 1000 * millis();
      if (diffMicros < waitMicros) {
        break;
      } 
      digitalWrite(SPEAKER_PIN, high ? 1 : 0);
      high = !high; 
      delayMicroseconds(waitMicros);
    }
    return;
  }
#endif  
  dumbdisplay.tone(freq, duration);
  delay(duration);
}


const char* song = "GcecedcAGGcecedgegegecGAccAGGcecedc";
const char* beat = "24114242424114282111142411142411428";

const int speed = 300;



bool playToSpeaker = true;

bool play = false;
bool restart = false;

int adhocFreq = -1;



LcdDDLayer* playLayer;
LcdDDLayer* restartLayer;
LcdDDLayer* targetLayer;


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
  if (pLayer == playLayer) {
    play = !play;
    if (play) {
      playLayer->backgroundColor("lightgray");
    } else {
      playLayer->noBackgroundColor();
    }
  } if (pLayer == targetLayer) {
    playToSpeaker = !playToSpeaker;
    if (playToSpeaker) {
      targetLayer->noBackgroundColor();
    } else {
      targetLayer->backgroundColor("lightgray");
    }
  } else if (pLayer == restartLayer) {
    restart = true;
  } else {
    const char* nodeInfo = pLayer->customData.c_str();
    int8_t octaveIdx = nodeInfo[0] - '0';
    int8_t nodeIdx = nodeInfo[1] - '0';
    int freq = GetNoteFreq(octaveIdx, nodeIdx);
    adhocFreq = freq;
  }
}

void IdleCallback(long idleForMillis) {
  play = false;
}


GraphicalDDLayer* SetupKey(DumbDisplay& dumbdisplay, int8_t octiveOffset, int8_t nodeIdx) {
  int8_t width = WIDTH - 2 * BORDER;
  int8_t xOffset = nodeIdx * WIDTH / 2;
  int8_t height;
  const char* bgColor;
  bool isSemi = false;
  if (nodeIdx == 1 ||
      nodeIdx == 3 ||
      nodeIdx == 6 ||
      nodeIdx == 8 ||
      nodeIdx == 10) {
    height = HEIGHT / 2 + 10;
    bgColor = "black";
    isSemi = true;
  } else {
    height = HEIGHT;
    bgColor = "white";
  }
  if (nodeIdx > 4) {
    xOffset += WIDTH / 2;
  }
  char customData[3];
  customData[0] = '0' + octiveOffset;
  customData[1] = '0' + nodeIdx;
  customData[2] = 0;
  GraphicalDDLayer* keyLayer = dumbdisplay.createGraphicalLayer(width, height);
  keyLayer->customData = customData;
  keyLayer->backgroundColor(bgColor);
  keyLayer->border(BORDER, "gray");
  keyLayer->padding(0);
  keyLayer->setFeedbackHandler(FeedbackHandler, "f");
  if (isSemi) {
    dumbdisplay.reorderLayer(keyLayer, "T");
  } else {
    if (nodeIdx == 0) {
      keyLayer->drawStr(2, HEIGHT - 15, "C", "blue");
    }
  }
  int8_t l = WIDTH + octiveOffset * 7 * WIDTH + xOffset;
  int8_t t  = TOP_HEIGHT;
  int8_t w = width + 2 * BORDER;
  int8_t h = height + 2 * BORDER;
  dumbdisplay.pinLayer(keyLayer, l, t, w, h);
  return keyLayer;
}

LcdDDLayer* SetupButton(const String& label) {
  LcdDDLayer* buttonLayer = dumbdisplay.createLcdLayer(4, 1);
  buttonLayer->writeLine(label, 0, "C");
  buttonLayer->border(1, "darkgray", "round");
  buttonLayer->noBackgroundColor();
  buttonLayer->setFeedbackHandler(FeedbackHandler, "f");
  return buttonLayer;
}



void setup() {
#ifdef SPEAKER_PIN
  pinMode(SPEAKER_PIN, OUTPUT);
#endif

  dumbdisplay.recordLayerSetupCommands();

  dumbdisplay.configPinFrame(9 * WIDTH, TOP_HEIGHT + HEIGHT);

  SetupKey(dumbdisplay, -1, 11);
  for (int8_t i = 0; i < 12; i++) {
    SetupKey(dumbdisplay, 0, i);
  }
  SetupKey(dumbdisplay, 1, 0);

  playLayer = SetupButton("⏯");
  restartLayer = SetupButton("⏮");
  targetLayer = SetupButton("📱");

  dumbdisplay.pinAutoPinLayers(DD_AP_HORI_3(
                                   playLayer->getLayerId(),
                                   restartLayer->getLayerId(),
                                   targetLayer->getLayerId()),
                               0, 0, 9 * WIDTH, TOP_HEIGHT);

  dumbdisplay.playbackLayerSetupCommands("ddmelody");

  dumbdisplay.setIdleCalback(IdleCallback);
}

void loop() {
  for (int8_t i = 0;;) {

    int freq;
    int duration;

    if (adhocFreq == -1) {
      // no "adhoc note" to play

      char n;
      if (restart || !play) {
        // if restarting or not playing => set n to 0 (indicate no note to play)
        n = 0;
      } else {
        n = song[i];
        if (n == 0) {
          // reached end of song => set restart flag
          restart = true;
        }
      }

      if (n == 0) {
        // no note to play
        freq = -1;
      } else {        
        // get song note to play
        char nodeName;
        int8_t octaveIdx;
        if (n >= 'a') {
          nodeName = 'A' + (n - 'a');
          octaveIdx = 1;
        } else {
          nodeName = n;
          octaveIdx = 0;
        }

        // convert the song note into tone frequency
        int8_t nodeIdx = ToNoteIdx(nodeName, ' ');
        freq = GetNoteFreq(octaveIdx, nodeIdx);

        // get the how to to play the note/tone for 
        duration = speed * (beat[i] - '0');

        i = i + 1;
      }
    } else {
      // adhoc note (press of key on DumbDisplay) .. play it for 200ms
      freq = adhocFreq;
      duration = 200;
      adhocFreq = -1;
    }

    if (freq != -1) {
      // play the note/tone
      PlayTone(freq, duration, playToSpeaker);
    }
    
    DDYield();

    if (restart) {
      // restarting ... reset restart flag and break out of loop
      restart = false;
      break;
    }

  }
}
