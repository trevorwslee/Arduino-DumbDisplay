

#define SERVO_PIN    11

#define US_TRIG_PIN   9
#define US_ECHO_PIN  10





const int Width = 400;
const int Height = Width / 2;
const int W_half = Height;
const int H = Height;
const float DistFactor = W_half / 50.0;

const int MaxAngle = 120;
const int AngleIncrement = 1;

const int A_start = (180 - MaxAngle) / 2;


bool CalcCoor(int ang, int dist, int& x, int& y) {
  float dist_norm = DistFactor * dist;
  if (dist_norm > W_half) {
    return false;
  }
  int ang_norm_i = 180 - (A_start + ang);
  float rad_i = ang_norm_i * 3.1416 / 180.0;
  float y_i = dist_norm * sin(rad_i);
  float x_i = dist_norm * cos(rad_i);
  x = W_half - x_i;
  y = H - y_i;
  return true;
}



#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());



#include <Servo.h>
Servo servo;



GraphicalDDLayer* graphicalLayer;



int angle = 0;
bool angleIncreasing = true;

void setup() {
  servo.attach(SERVO_PIN);
  servo.write(angle);


  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input


  graphicalLayer = dumbdisplay.createGraphicalLayer(Width, Height);



}

void loop() {
  servo.write(angle);
  delay(15);  // give it some time

  // read ultrasoncic sensor for detected object distance
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // . ses the trigPin on HIGH state for 10 micro seconds
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  // . reas the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(US_ECHO_PIN, HIGH);
  // . calculating the distance ... 0.034: sound travels 0.034 cm per second
  int distance = duration * 0.034 / 2;
  //distance = 18;
  int x;
  int y;
  int r = 3;
  if (CalcCoor(angle, distance, x, y)) {
    if (true) {
      dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y);
    }
    graphicalLayer->fillCircle(x, y, r);
  }


  if (angleIncreasing) {
    angle += AngleIncrement;
  } else {
    angle -= AngleIncrement;
  }
  if (angle > MaxAngle) {
    angle = MaxAngle;
    angleIncreasing = false;
  } else {
    if (angle < 0) {
      angle = 0;
      angleIncreasing = true;
    }
  }
}