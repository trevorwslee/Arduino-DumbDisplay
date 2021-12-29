// for a desciption of the melody experiment, please watch the YouTube video 
// -- Raspberry Pi Pico playing song melody tones, with DumbDisplay control and keyboard input
// -- https://www.youtube.com/watch?v=l-HrsJXIwBY 


#define SPEAKER_PIN 5


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
    noteName = noteIdx + 1; 
  } else if (halfNote == 'b') {
    noteName = noteIdx - 1;
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


void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
}

void loop() {
  for (int i = 0;; i += 2) {

    char noteName = song[i];
    char halfNote = song[i + 1];
    
    if (noteName == 0) {
      // reached end of song => break out of loop
      break;
    }

    // convert the song note into tone frequency
    int noteIdx = ToNoteIdx(noteName, halfNote);
    int freq = GetNoteFreq(octave[i] - '0', noteIdx);

    // get the how to to play the note/tone for 
    int duration = beatSpeed * (beat[i] - '0');

    // play the note/tone
    PlayTone(freq, duration);
  }
}
