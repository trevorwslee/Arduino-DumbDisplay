
/*

A|   13| ---
R|     |     LED
U|  GND| --- 
N|     |
I| 3.3V| --- 
N|     |     PHOTO
O|   A2| ---
 |     |    | 10K 
U|  GND| --- 
N|
O|   5V| --- VCC | HC06 
 |  GND| --- GND
 |   11| --- TXD
 |   10| --- RXD

*/



#include "ssdumbdisplay.h"


#define USE_BLUETOOTH



// pins
const int a_btn = 2;
const int c_btn = 4;
const int d_btn = 5;
const int b_btn = 3;
const int e_btn = 6;
const int f_btn = 7;
const int k_btn = 8;
const int x_joystick = A0;
const int y_joystick = A1;
const int photo_resist = A2;


const int width = 128;
const int height = 88;


const int l_len = 20;
const int cir_x = 25;
const int cir_y = 28/*32*/;
const int cir_r = 13;

const int txt_x = 4;
const int txt_y = 2;




#ifdef USE_BLUETOOTH
unsigned long baud = DUMBDISPLAY_BAUD;
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(11, 10), baud, true, 57600));
#else
DumbDisplay dumbdisplay(new DDInputOutput(serialBaud));
#endif

GraphicalDDLayer *pLayer;





void setup() {

  // initialise the digital pins
  pinMode(a_btn, INPUT);
  pinMode(c_btn, INPUT);
  pinMode(d_btn, INPUT);
  pinMode(b_btn, INPUT);
  pinMode(e_btn, INPUT);
  pinMode(f_btn, INPUT);
  pinMode(k_btn, INPUT);


  // initialize dumbdisplay

  dumbdisplay.debugSetup(13);  // setup to use pin 13
  dumbdisplay.connect();
  DDLogToSerial("=== connected ===");
  dumbdisplay.writeComment("Good Day!");

  pLayer = dumbdisplay.createGraphicalLayer(width, height);
  pLayer->border(2, "navy", "round");
  pLayer->backgroundColor("navy");

}


void drawBtn(int x, int y, int r, int off_x, int off_y, int btn, char* txt) {
  if (!digitalRead(btn)) {
    dumbdisplay.writeComment("BTN-" + String(txt));
  }  
  r = r + 1;
  pLayer->drawStr(x - r / 2 + off_x - 1, y + cir_r / 2 - off_y + 2, txt, "gold");
  pLayer->drawCircle(x, y + r, !digitalRead(btn) ? 3 * r / 4 : r / 2, "aqua");
}

void loop()
{
  dumbdisplay.recordLayerCommands();

  pLayer->clear();

  // to parse analog sensors to digital ones, we first read them and
  // map them to a value in [-1, 0, 1]
  int x_reading = analogRead(x_joystick);
  int y_reading = analogRead(y_joystick);
  int new_x_joystick = map(x_reading, 0, 1023, -1 * cir_r, cir_r) + 2;
  int new_y_joystick = map(y_reading, 0, 1023, -1 * cir_r, cir_r);
  int photo_resist_reading = analogRead(photo_resist);
  dumbdisplay.writeComment("x:" + String(x_reading) + String("; ") +
                           "y:" + String(y_reading) + String("; ") +
                           "p:" + String(photo_resist_reading));


  // change background color based on photo resistor value
  int bg_color = map(photo_resist_reading, 200, 700, 0, 255);
  //dumbdisplay.writeComment("c:" + DD_RGB_COLOR(bg_color, bg_color, 0xff));                       
  dumbdisplay.backgroundColor(DD_RGB_COLOR(bg_color, bg_color, 0xff));     


  //draw grid
  pLayer->drawLine(cir_x, cir_y - l_len, cir_x, cir_y + l_len, "slateblue");
  pLayer->drawLine(cir_x - l_len, cir_y , cir_x + l_len, cir_y, "slateblue");

  //draw circle
  //pLayer->drawText(cir_x + new_x_joystick, cir_y - new_y_joystick, "K", "green");
  pLayer->drawCircle(cir_x + new_x_joystick, cir_y - new_y_joystick, !digitalRead(k_btn) ? 3 * cir_r / 2 : cir_r, "azure");

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

  dumbdisplay.playbackLayerCommands();

  DDDelay(100);
}
