


extern DumbDisplay dumbdisplay;  // there will be a dumbdisplay elsewhere

void PlayHitSoundEffect() {
  dumbdisplay.tone(1000, 50);
  dumbdisplay.tone(800, 50);
}
void PlayMissSoundEffect() {
  dumbdisplay.tone(500, 50);
  dumbdisplay.tone(700, 50);
}
void PlayMissLifeSoundEffect() {
  dumbdisplay.tone(600, 50);
  dumbdisplay.tone(500, 50);
  dumbdisplay.tone(400, 50);
  dumbdisplay.tone(300, 50);
}
void PlayEndGameSoundEffect() {
  dumbdisplay.tone(200, 100);
  dumbdisplay.tone(400, 200);
  dumbdisplay.tone(300, 200);
  dumbdisplay.tone(600, 100);
}

