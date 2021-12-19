#define SPEAKER_PIN 5


int8_t ToNoteIdx(char nodeName, char semiTone) {
  if (nodeName == '0') return -99;  // '0' means no node; -99 is a special node index that signal no note
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


// 440 is A
int GetNoteFreq(int8_t octaveIdx, int8_t nodeIdx) {
  if (nodeIdx == -99) return 0;  // -99 is a special node index that signal no note
  int8_t n = nodeIdx + 12 * octaveIdx - 8;
  float freq = 440.0 * pow(2, n / 12.0);
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



const char* song = "GcecedcAGGcecedgegegecGAccAGGcecedc";
const char* beat = "24114242424114282111142411142411428";

const int speed = 300;


void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
}

void loop() {
  for (int8_t i = 0;; i++) {

    char n = song[i];
    if (n == 0) {
      // reached end of song => break out of loop
      break;
    }

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
    int freq = GetNoteFreq(octaveIdx, nodeIdx);

    // get the how to to play the note/tone for 
    int duration = speed * (beat[i] - '0');

    // play the note/tone
    PlayTone(freq, duration);
  }
}
