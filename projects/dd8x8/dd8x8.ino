
// for a desciption of the 8x8 LED Matrix with DumbDisplay UI experiment, please watch the YouTube video 
// -- Raspberry Pi Pico 8x8 LED Matrix Arduino Experiment with HC-06 and DumpDisplay
// -- https://www.youtube.com/watch?v=YRfc9Z-L73A 




// comment out BLUETOOTH if don't have HC-06 connectivity
// . GP8 => RX of HC-06; GP9 => TX of HC-06
// if no HC-06 connectivity, will need to use DumbDisplayWifiBridge
#define BLUETOOTH

#ifdef BLUETOOTH

// GP8 => RX of HC-06; GP9 => TX of HC-06; HC-06 should be configured to use baud rate of 115200
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include <picodumbdisplay.h>
DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));

#else

#include <dumbdisplay.h>
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif


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

LedGridDDLayer *ledMatrix;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);

void setup() {
  pinMode(R_SHIFT_PIN, OUTPUT);
  pinMode(R_CLOCK_PIN, OUTPUT);
  
  pinMode(C_SHIFT_PIN, OUTPUT);
  pinMode(C_CLOCK_PIN, OUTPUT);
  pinMode(C_LATCH_PIN, OUTPUT);

  digitalWrite(R_CLOCK_PIN, 0);

  digitalWrite(C_CLOCK_PIN, 0);
  digitalWrite(C_LATCH_PIN, 0);

  // turn off all the LEDs
  shiftOut(C_SHIFT_PIN, C_CLOCK_PIN, LSBFIRST, 0xff);
  digitalWrite(C_LATCH_PIN, 1);
  digitalWrite(C_LATCH_PIN, 0);

  // create and setup LED layer on DumbDisplay
  ledMatrix = dumbdisplay.createLedGridLayer(8, 8);
  ledMatrix->border(0.2, "white");
  ledMatrix->padding(0.2);
  ledMatrix->onColor("red");  
  ledMatrix->offColor("lightgray");
  ledMatrix->backgroundColor("black");
  ledMatrix->setFeedbackHandler(FeedbackHandler, "fa");
  for (int i = 0; i < 8; i++) {
    int bits = data[i];
    ledMatrix->bitwise(~bits, i);  // notice that the bit are inverted
  }
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

  DDYield();
}



void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {

  if (type != LONGPRESS) {
    int c = feedback.x;
    int r = feedback.y;
    ledMatrix->toggle(c, r);
    int bits = data[r];
    int mask = 1 << (7 - c);
    bool on = (bits & mask) == 0; 
    if (on) {
      data[r] = bits | mask;
    } else {
      data[r] = bits & ~mask;
    }
  } else {
    ledMatrix->clear();
    for (int r = 0; r < 8; r++) {
      data[r] = 0xff;
    }
  }
}
