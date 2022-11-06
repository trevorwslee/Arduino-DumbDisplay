

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
  } else if (halfNote = 'b') {
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
