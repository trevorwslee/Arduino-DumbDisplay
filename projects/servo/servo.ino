// for a desciption of the Servo experiment, please watch the YouTube video 
// -- ESP8266 Servo Arduino experiment, subsequently, with simple DumbDisplay UI
// -- https://www.youtube.com/watch?v=pgfL_qwj8cQ 


#include <Servo.h>


#define SERVO_PIN 2

// comment out the following lines if button/pin not connected
#define BUTTON_PIN 0
#define LED_PIN 16

// if want to try out ESP8266 DumbDisplay support, uncomment the following lines
// . that defines ESP8266_DD_SUPPORT
// . that defines your WIFI_SSID
// . that defines your WIFI_PASSWORD
//#define ESP8266_DD_SUPPORT
//#define WIFI_SSID           "<your-wifi-ssid>"
//#define WIFI_PASSWORD       "<your-wifi-passwor>"


// create the servo object, from <Servo.h>
Servo servo;


#ifdef ESP8266_DD_SUPPORT
// declears some routines for ESP8266 support; they will be defined later
void setup_DD();
void loop_DDPreProcess(int servoAngle);
void loop_DDPostProcess();
#endif


void setup() {
#ifdef BUTTON_PIN
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#endif  
#ifdef LED_PIN  
  pinMode(LED_PIN, OUTPUT);
#endif

  // attach the servo object to the server pin
  servo.attach(SERVO_PIN);

  // move the servo just for a test
  servo.write(0);
  delay(2000);

#ifdef ESP8266_DD_SUPPORT
  setup_DD();
#endif
}



int toAngle = 0;
int autoInc = 1;

long lastButtonMillis = 0;

void loop() {
  int servoAngle = servo.read();

#ifdef ESP8266_DD_SUPPORT
  loop_DDPreProcess(servoAngle);
#endif


  if (servoAngle != toAngle) {
    servo.write(toAngle);
#ifdef LED_PIN  
    analogWrite(LED_PIN, map(toAngle, 0, 180, 0, 255));
#endif
  } else {
    if (autoInc != 0) {
      toAngle += autoInc;
      if (toAngle >= 180) {
        toAngle = 180;
        autoInc = -1;
      } else if (toAngle <= 0) {
        toAngle = 0;
        autoInc = 1;
      }
    }
  }
#ifdef BUTTON_PIN
  if (digitalRead(BUTTON_PIN) == LOW) {
    if ((millis() - lastButtonMillis) > 200) {
      if (autoInc == 1) {
        autoInc = -1;
      } else {
        autoInc = 1;
      }
      lastButtonMillis = millis();
    }
  }
#endif

#ifdef ESP8266_DD_SUPPORT
  loop_DDPostProcess();
#endif
}


// For a brief explanation of the sketch, you may want to watch the video **ESP8266 Servo Arduino experiment, subsequently, with simple DumbDisplay UI** -- https://www.youtube.com/watch?v=pgfL_qwj8cQ

#ifdef ESP8266_DD_SUPPORT


#define DD_4_ESP8266
#include "wifidumbdisplay.h"

DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));


PlotterDDLayer* pPlotter;
GraphicalDDLayer* pGraphical;


const int DotRadius = 4;
const int GaugeRadius = 128;
const int LayerWidth = 2 * (GaugeRadius + DotRadius);
const int LayerHeight = GaugeRadius + 2 * DotRadius;
const String OffColor = DD_INT_COLOR(0xD3D3D3);    // lightgray
const String OnColor = DD_INT_COLOR(0x006400);     // darkgreen
const String BgColor = DD_INT_COLOR(0xF0FFFF);     // azure
const String HandColor = DD_INT_COLOR(0xFF0000);   // red



void AngleToCoors(int angle, int radius, int& xCoor, int& yCoor) {
  // just some Math
  float r = angle * 0.0174533;
  float s = sin(r);
  float c = cos(r);
  float x = radius * c;
  float y = radius * s;
  xCoor = round(x);
  yCoor = round(y);
}
void AngleToXY(int angle, int radius, int& x, int& y) {
  // just some Math
  float r = angle * 0.0174533;
  float s = sin(r);
  float c = cos(r);
  float base = radius * c;
  float height = radius * s;
  x = GaugeRadius + DotRadius + round(base);
  y = GaugeRadius + DotRadius - round(height);
}
int XYToAngle(int x, int y) {
  // just some Math
  float base = x - DotRadius - GaugeRadius;
  float height = DotRadius + GaugeRadius - y;
  if (abs(base) > 0) {
    float r = atan(abs(height) / base);
    int angle = round(r / 0.0174533);
    if (angle < 0) {
      angle = 180 + angle;
    }
    return angle;
  } else {
    return 90;
  }  
}
int ServoAngleToAngle(int servoAngle) {
  // servo angle will be in 5 degress increment
  return 5 * round(((float) servoAngle / 5.0));
}


// declare the "feedback handler" routine; it will be defined later
void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);


void setup_DD() {
  // record the setup layer commands so that DumbDisplay app can re-connected
  dumbdisplay.recordLayerSetupCommands();

  // create a plotter layer that shows the angle, and for more fun, sin and cos of the angle
  pPlotter = dumbdisplay.createPlotterLayer(300, 150);
  pPlotter->padding(10);
  //pPlotter->opacity(64);
  pPlotter->opacity(25);
  pPlotter->noBackgroundColor();
  pPlotter->label("Servo Angle");

  // create a graphical layer for the gauge to show and control servo
  pGraphical = dumbdisplay.createGraphicalLayer(LayerWidth, LayerHeight);
  pGraphical->border(3, "darkblue", "round", 2);
  pGraphical->padding(4);
  pGraphical->setTextSize(18);
  pGraphical->backgroundColor(BgColor);
  pGraphical->setTextColor("red");

  // draw markers on the gauge 
  pGraphical->drawStr(2 * GaugeRadius - 3 * DotRadius - 4, GaugeRadius - 10, "0");
  pGraphical->drawStr(GaugeRadius - 7, 6, "90");
  pGraphical->drawStr(2 * DotRadius + 2, GaugeRadius - 10, "180");
  
  // draw the gauge's "semi-circle" with dots, in 5 degress increment
  for (int angle = 0; angle <= 180; angle += 5) {
    int x;
    int y;
    AngleToXY(angle, GaugeRadius, x, y);
    pGraphical->fillCircle(x, y, DotRadius, OffColor);
  } 

  // setup interaction when clicking / dragging on the gauge;
  // 1) feedback will be handled by FeedbackHandler
  // 2) feedback will auto-repeat as long as the gauge is kept being touched, in 50 ms;
  //    something like dragging
  // 3) note that the repeat is trigger by long pressing
  pGraphical->setFeedbackHandler(FeedbackHandler, "fs:lprpt50");

  // play back the layer setup commands recorded;
  // and instruct that "servo" is the id of the setup layer commands, for re-connection
  dumbdisplay.playbackLayerSetupCommands("servo");
}


void DrawServoPointer(int servoAngle, bool turnOn) {
  int angle = ServoAngleToAngle(servoAngle);
  int x;
  int y;
  AngleToXY(angle, GaugeRadius, x, y);
  if (turnOn) {
    pGraphical->fillCircle(x, y, DotRadius + 1, OnColor);
  } else {
    pGraphical->fillCircle(x, y, DotRadius + 1, BgColor);
    pGraphical->fillCircle(x, y, DotRadius, OffColor);
  }  
  int tx;
  int ty;
  AngleToXY(angle, GaugeRadius - 50, tx, ty);
  int cx = LayerWidth / 2;
  int cy = LayerHeight;
  pGraphical->drawLine(tx, ty, cx, cy, turnOn ? HandColor : BgColor);
  pGraphical->fillCircle(tx, ty, DotRadius - 1, turnOn ? OnColor : BgColor);
}


int lastAngle = -1;


// will be called i
void loop_DDPreProcess(int servoAngle) {
  if (autoInc != 0) {
    // set angle, sin and cos of it, to the plotter layer
    float r = servoAngle * 0.01745329252; 
    float s = sin(r);
    float c = cos(r);
    pPlotter->set("", servoAngle, "s", s, "c", c);
  }
  if (servoAngle != lastAngle) {
    if (lastAngle != -1) {
      DrawServoPointer(lastAngle, false);
    }
    DrawServoPointer(servoAngle, true);
    lastAngle = servoAngle;
  }
}

// will be called last in the loop block
void loop_DDPostProcess() {
  // give DumbDisplay library a change to do it's work
  DDYield();
}


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
  if (type == DDFeedbackType::DOUBLECLICK) {
    // if click type is "double click" ... toggle "auto move servo" on/off
    if (autoInc == 1) {
      autoInc = -1;
    } else {
      autoInc = 1;
    }
  } else {
    // otherwise, set "to angle" to "clicked angle", and turn off "auto move servo"
    int angle = XYToAngle(feedback.x, feedback.y);
    toAngle = angle;
    autoInc = 0;
    pPlotter->clear();
  }
}


#endif


