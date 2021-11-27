
#define R_SHIFT_PIN   6
#define R_CLOCK_PIN   27

#define C_SHIFT_PIN   22
#define C_LATCH_PIN   21
#define C_CLOCK_PIN   20

int data[8] = { 0b11100111, 
                0b11011011, 
                0b10111101,
                0b01111110,
                0b00000000,
                0b11100111,
                0b11100111,
                0b11100111 };



void setup() {
  pinMode(R_SHIFT_PIN, OUTPUT);
  pinMode(R_CLOCK_PIN, OUTPUT);
  
  pinMode(C_SHIFT_PIN, OUTPUT);
  pinMode(C_CLOCK_PIN, OUTPUT);
  pinMode(C_LATCH_PIN, OUTPUT);

  digitalWrite(R_CLOCK_PIN, 0);

  digitalWrite(C_CLOCK_PIN, 0);
  digitalWrite(C_LATCH_PIN, 0);
}




void loop() {
  for (int i = 0; i < 8; i++) {
    int bits = data[i];

    // shift the "column" bits
    shiftOut(C_SHIFT_PIN, C_CLOCK_PIN, LSBFIRST, bits);

    // set the "row" shift bit, which should only be 1 when i is 0
    digitalWrite(R_SHIFT_PIN, i == 0 ? 1 : 0);

    // pulse the "row" CLOCK to trigger shifting of the "rows"    
    digitalWrite(R_CLOCK_PIN, 1);
    digitalWrite(R_CLOCK_PIN, 0);

    // pulse the "column" LATCH to transfer to the output register
    digitalWrite(C_LATCH_PIN, 1);
    digitalWrite(C_LATCH_PIN, 0);

    // delay a bit so that the light will "shine through"
    delay(2);
  }
}
