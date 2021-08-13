#include "ssdumbdisplay.h"

const int a_btn = 2;
const int c_btn = 4;
const int d_btn = 5;
const int b_btn = 3;
const int e_btn = 6;
const int f_btn = 7;
const int k_btn = 8;
const int x_joystick = A0;
const int y_joystick = A1;

// keep state of all btns
bool prev_a_btn = false;
bool prev_c_btn = false;
bool prev_d_btn = false;
bool prev_b_btn = false;
bool prev_e_btn = false;
bool prev_f_btn = false;
bool prev_k_btn = false;

#define USE_BLUETOOTH

boolean enableSerial = true;
unsigned long serialBaud = 57600;
#ifdef USE_BLUETOOTH
unsigned long baud = DUMBDISPLAY_BAUD;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(11, 10), baud, enableSerial, serialBaud));
#else
DumbDisplay dumbdisplay(new DDInputOutput(serialBaud));
#endif

GraphicalDDLayer *pLayer;


void setup() {
  dumbdisplay.debugSetup(9/*13*/);  // setup to use pin 13
  dumbdisplay.connect();
  DDLogToSerial("=== connected ===");
  dumbdisplay.writeComment("Good Day!");

  pLayer = dumbdisplay.createGraphicalLayer(151, 101);


  // initialise the digital pins
  pinMode(a_btn, INPUT);
  pinMode(c_btn, INPUT);
  pinMode(d_btn, INPUT);
  pinMode(b_btn, INPUT);
  pinMode(e_btn, INPUT);
  pinMode(f_btn, INPUT);
  pinMode(k_btn, INPUT);

  //Setup display
  //Serial.begin(9600);

}

void btn_cb(bool new_val, bool &old_val)
{
  // if there is no change in state, don't generate events
  if (new_val == old_val)
    return;
  // update the previous value so changes in state can be registered
  old_val = new_val;
}

const int l_len = 20;
const int cir_x = 25;
const int cir_y = 32;
const int cir_r = 13;

const int txt_x = 4;
const int txt_y = 2;

void drawBtn(int x, int y, int r, int off_x, int off_y, int btn, char* txt) {
  if (!digitalRead(btn)) {
    dumbdisplay.writeComment("BTN-" + String(txt));
    //Serial.print("BTN-");
    //Serial.print(txt);
    //Serial.print("\n");
  }  
  pLayer->drawText(x - r / 2 + off_x, y + cir_r / 2 - off_y, txt, "pink");
  pLayer->drawCircle(x, y + r, !digitalRead(btn) ? 3 * r / 4 : r / 2, "pink");
  // u8g2.drawStr(x - r / 2 + off_x, y + cir_r / 2 - off_y, txt);
  // u8g2.drawCircle(x, y, !digitalRead(btn) ? 3 * r / 4 : r / 2);
}

void loop()
{
  pLayer->clear();

  //u8g2.clearBuffer();
  // handle normal buttons, the state will be true on LOW, when the
  // button is pressed (low resistance on button press)
  btn_cb(digitalRead(a_btn) == LOW, prev_a_btn);
  btn_cb(digitalRead(b_btn) == LOW, prev_b_btn);
  btn_cb(digitalRead(c_btn) == LOW, prev_c_btn);
  btn_cb(digitalRead(d_btn) == LOW, prev_d_btn);
  btn_cb(digitalRead(e_btn) == LOW, prev_e_btn);
  btn_cb(digitalRead(f_btn) == LOW, prev_f_btn);

  // to parse analog sensors to digital ones, we first read them and
  // map them to a value in [-1, 0, 1]
  int new_x_joystick = map(analogRead(x_joystick), 0, 1000, -1 * cir_r, cir_r);
  int new_y_joystick = map(analogRead(y_joystick), 0, 1000, -1 * cir_r, cir_r);
  dumbdisplay.writeComment("x:" + String(cir_x + new_x_joystick) + 
                           String(" -- ") +
                           "y:" + String(cir_y + new_y_joystick));
  // Serial.print("x:");
  // Serial.print(cir_x + new_x_joystick);
  // Serial.print("\t");
  // Serial.print("y:");
  // Serial.print(cir_y + new_y_joystick);
  // Serial.print("\n");

  //draw grid
  pLayer->drawLine(cir_x, cir_y - l_len, cir_x, cir_y + l_len, "blue");
  pLayer->drawLine(cir_x - l_len, cir_y , cir_x + l_len, cir_y, "green");
  // u8g2.drawLine(cir_x, cir_y - l_len, cir_x, cir_y + l_len);
  // u8g2.drawLine(cir_x - l_len, cir_y , cir_x + l_len, cir_y);

  //draw circle
  // u8g2.drawStr(cir_x + new_x_joystick, cir_y - new_y_joystick, "K");
  // u8g2.drawCircle(cir_x + new_x_joystick, cir_y - new_y_joystick, !digitalRead(k_btn) ? 3 * cir_r / 2 : cir_r);

  //draw Btn
  //A
  drawBtn(100, 20, cir_r, txt_x, txt_y, a_btn, "A");
  //D
  drawBtn(85, 35, cir_r, txt_x, txt_y, d_btn, "D");
  //C
  drawBtn(100, 50, cir_r, txt_x, txt_y, c_btn, "C");
  //B
  drawBtn(115, 35, cir_r, txt_x, txt_y, b_btn, "B");
  //F
  drawBtn(55, 55, cir_r, txt_x, txt_y, f_btn, "F");
  //E
  drawBtn(75, 55, cir_r, txt_x, txt_y, e_btn, "E");

//  u8g2.sendBuffer();

  DDDelay(100);
}
