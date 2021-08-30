#include <dumbdisplay.h>


#define WITH_DD


// *** COMMON_CATHODE

const int p_g = 21;
const int p_f = 20;
const int p_a = 19;
const int p_b = 18;

const int p_e = 10;
const int p_d = 11;
const int p_c = 12;
const int p_dp = 13;

const int seg_pins[] = { p_a, p_b, p_c, p_d, p_e, p_f, p_g, p_dp };
const bool seg_none[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
const bool seg_err[] = { 1, 0, 0, 1, 1, 1, 1, 0 };
const bool seg_dot[] = { 0, 0, 0, 0, 0, 0, 0, 1 };
const bool seg_0[] = { 1, 1, 1, 1, 1, 1, 0, 0 };
const bool seg_1[] = { 0, 1, 1, 0, 0, 0, 0, 0 };
const bool seg_2[] = { 1, 1, 0, 1, 1, 0, 1, 0 };
const bool seg_3[] = { 1, 1, 1, 1, 0, 0, 1, 0 };
const bool seg_4[] = { 0, 1, 1, 0, 0, 1, 1, 0};
const bool seg_5[] = { 1, 0, 1, 1, 0, 1, 1, 0 };
const bool seg_6[] = { 1, 0, 1, 1, 1, 1, 1, 0 };
const bool seg_7[] = { 1, 1, 1, 0, 0, 0, 0, 0 };
const bool seg_8[] = { 1, 1, 1, 1, 1, 1, 1, 0 };
const bool seg_9[] = { 1, 1, 1, 1, 0, 1, 1, 0 };


const bool* getStates(char c) {
   switch (c) {
      case '0': return seg_0;
      case '1': return seg_1;
      case '2': return seg_2;
      case '3': return seg_3;
      case '4': return seg_4;
      case '5': return seg_5;
      case '6': return seg_6;
      case '7': return seg_7;
      case '8': return seg_8;
      case '9': return seg_9;
      case '.': return seg_dot;
      default: return seg_err;
   }
}

void turnOnSeg(const bool* states) {
   for (int i = 0; i <= 7; i++) {
      digitalWrite(seg_pins[i], states[i] ? HIGH : LOW);
   }
}

void turnOnSeg(char c) {
   turnOnSeg(getStates(c));
}


#ifdef WITH_DD
DumbDisplay dumbdisplay(new DDInputOutput(57600));
SevenSegmentRowDDLayer *sevenSeg;
#endif



void setup() {
   pinMode(p_g, OUTPUT);
   pinMode(p_f, OUTPUT);
   pinMode(p_a, OUTPUT);
   pinMode(p_b, OUTPUT);

   pinMode(p_e, OUTPUT);
   pinMode(p_d, OUTPUT);
   pinMode(p_c, OUTPUT);
   pinMode(p_dp, OUTPUT);

#ifdef WITH_DD
   sevenSeg = dumbdisplay.create7SegmentRowLayer(1);
#endif     
}

char c = '0';
void loop() {
   turnOnSeg(c);

#ifdef WITH_DD
   if (c == '.') {
      sevenSeg->setOn(".");
   } else if (c == '?')
      sevenSeg->showFormatted("E");
   else   
      sevenSeg->showNumber(c - '0');
#endif     

   if (c == '9')
      c = '.';
   else if (c == '.')
      c = '?';
   else if (c == '?')
      c = '0';
   else 
      c = c + 1;         

   delay(1000);
}
