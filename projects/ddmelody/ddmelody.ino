// for a desciption of the melody experiment, please watch the YouTube video 
// -- Raspberry Pi Pico playing song melody tones, with DumbDisplay control and keyboard input
// -- https://www.youtube.com/watch?v=l-HrsJXIwBY 



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
DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));

#else

#include <dumbdisplay.h>
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif


// noteName: C, D, E, F, G, A, B
// halfNote: #, b
int ToNoteIdx(char noteName, char halfNote) {
  int noteIdx;
  switch (noteName) {
    case 'C': noteIdx = 0; break;
    case 'D': noteIdx = 2; break;
    case 'E': noteIdx = 4; break;
    case 'F': noteIdx = 5; break;
    case 'G': noteIdx = 7; break;
    case 'A': noteIdx = 9; break;
    case 'B': noteIdx = 11; break;
  }
  if (halfNote == '#') {
    noteIdx = noteIdx + 1; 
  } else if (halfNote == 'b') {
    noteIdx = noteIdx - 1;
  }
  return noteIdx;
}


// octave: can be negative
// noteIdx: 0 to 11; i.e. 12 note indexes in an octave
int GetNoteFreq(int octave, int noteIdx) {
  int n = noteIdx + 12 * octave - 8;
  float freq = 440.0 * pow(2, n / 12.0);  // 440 is A
  return round(freq);
}


void PlayTone(int freq, int duration) {
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
}



const char* song   = "G C E C E D C A G G C E C E D G E G E G E C G A C C A G G C E C E D C ";
const char* octave = "0 1 1 1 1 1 1 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 1 1 0 0 0 1 1 1 1 1 1 ";
const char* beat   = "2 4 1 1 4 2 4 2 4 2 4 1 1 4 2 8 2 1 1 1 1 4 2 4 1 1 1 4 2 4 1 1 4 2 8 ";

const int beatSpeed = 300;




const int TOP_HEIGHT = 30;
const int WIDTH = 14;
const int HEIGHT = 80;
const int BORDER = 1;



void PlayTone(int freq, int duration, bool playToSpeaker) {
#ifdef SPEAKER_PIN
  if (playToSpeaker) {
    PlayTone(freq, duration);
    return;
  }
#endif  
  dumbdisplay.tone(freq, duration);
  delay(duration);
}


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
    const char* noteInfo = pLayer->customData.c_str();
    int octaveIdx = noteInfo[0] - '0';
    int noteIdx = noteInfo[1] - '0';
    int freq = GetNoteFreq(octaveIdx, noteIdx);
    adhocFreq = freq;
  }
}

void IdleCallback(long idleForMillis, DDIdleConnectionState connectionState) {
  play = false;
}


GraphicalDDLayer* SetupKey(int octaveOffset, int noteIdx) {
  int width = WIDTH - 2 * BORDER;
  int xOffset = noteIdx * WIDTH / 2;
  int height;
  const char* bgColor;
  bool isSemi = false;
  if (noteIdx == 1 ||
      noteIdx == 3 ||
      noteIdx == 6 ||
      noteIdx == 8 ||
      noteIdx == 10) {
    height = HEIGHT / 2 + 10;
    bgColor = "black";
    isSemi = true;
  } else {
    height = HEIGHT;
    bgColor = "white";
  }
  if (noteIdx > 4) {
    xOffset += WIDTH / 2;
  }
  char customData[3];
  customData[0] = '0' + octaveOffset;
  customData[1] = '0' + noteIdx;
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
    if (noteIdx == 0) {
      keyLayer->drawStr(2, HEIGHT - 15, "C", "blue");
    }
  }
  int l = WIDTH + octaveOffset * 7 * WIDTH + xOffset;
  int t  = TOP_HEIGHT;
  int w = width + 2 * BORDER;
  int h = height + 2 * BORDER;
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

  SetupKey(-1, 11);
  for (int i = 0; i < 12; i++) {
    SetupKey(0, i);
  }
  SetupKey(1, 0);

  playLayer = SetupButton("⏯");
  restartLayer = SetupButton("⏮");
  targetLayer = SetupButton("📱");

  dumbdisplay.pinAutoPinLayers(DD_AP_HORI_3(
                                   playLayer->getLayerId(),
                                   restartLayer->getLayerId(),
                                   targetLayer->getLayerId()),
                               0, 0, 9 * WIDTH, TOP_HEIGHT);

  dumbdisplay.playbackLayerSetupCommands("ddmelody");

  dumbdisplay.setIdleCallback(IdleCallback);
}

void loop() {
  for (int i = 0;;) {

    DDYield();

    if (adhocFreq != -1) {
      // key on DumbDisplay pressed ...  play the note/tone of the key press
      PlayTone(adhocFreq, 200, playToSpeaker);
      adhocFreq = -1;
    }

    
    if (restart) {
      // restarting ... reset restart flag and break out of loop
      restart = false;
      break;
    }


    if (!play) {
      // not playing ... simply loop
      continue;
    }

    char noteName = song[i];

    if (noteName == 0) {
      // reached end of song => break out of loop
      break;
    }

    char halfNote = song[i + 1];

    // convert the song note into tone frequency
    int noteIdx = ToNoteIdx(noteName, halfNote);
    int freq = GetNoteFreq(octave[i] - '0', noteIdx);

    // get the how to to play the note/tone for 
    int duration = beatSpeed * (beat[i] - '0');

    // play the note/tone
    PlayTone(freq, duration, playToSpeaker);

    // increment i by 2
    i += 2;
  }
}
