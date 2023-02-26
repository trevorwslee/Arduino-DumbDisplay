#include <Arduino.h>


#define SERVO_PIN    11

#define US_TRIG_PIN   9
#define US_ECHO_PIN  10



const int Width = 400;
const int Height = Width / 2;
const int W_half = Height;
const int H = Height;
const int BoundDist = 50;
const int VisibleDist = 49;
const float DistFactor = W_half / BoundDist;

const int MaxAngle = 120;
const int AngleIncrement = 1;

const int A_start = (180 - MaxAngle) / 2;



void CalcCoor(int ang, int dist, int& x, int& y) {
  float dist_norm = DistFactor * dist;
  int ang_norm_i = 180 - (A_start + ang);
  float rad_i = ang_norm_i * 3.1416 / 180.0;
  float y_i = dist_norm * sin(rad_i);
  float x_i = dist_norm * cos(rad_i);
  x = W_half - x_i;
  y = H - y_i;
  //return dist_norm;
}


#include <Servo.h>
Servo servo;



#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());

GraphicalDDLayer* CreateGrahpicalLayer() {
  GraphicalDDLayer* layer = dumbdisplay.createGraphicalLayer(Width, Height);
  layer->border(5, "gray", "round");
  layer->padding(5);
  layer->penColor("green");
  return layer;
}




template<int MAX_LAYER_COUNT>
class FadingLayers {
  public:
    FadingLayers() {
      this->layerCount = 0;
      this->nextUseLayerIdx = 0;
    }
    void initAddLayer(DDLayer* layer) {
      if (layerCount < MAX_LAYER_COUNT) {
        layers[layerCount++] = layer;
      }
    }
  public:  
    DDLayer* useLayer() {
      int layerIdx = nextUseLayerIdx;
      nextUseLayerIdx = (nextUseLayerIdx + 1) % layerCount;
      for (int i = 0; i < this->layerCount; i++) {
        int lidx = (layerIdx + i) % layerCount;
        DDLayer* layer = layers[lidx];
        if (i == 0) {
          layer->opacity(100);
          layer->clear();
        } else {
          int opacity = i * (100.0 / layerCount);
          layer->opacity(opacity);
        }
      }
      return layers[layerIdx];
    }  
  private:
    DDLayer* layers[MAX_LAYER_COUNT];
    int layerCount;
    int nextUseLayerIdx;
};



const int ObjectLayerCount = 3;
FadingLayers<ObjectLayerCount> objectLayers; 

const int BeamLayerCount = 5;
FadingLayers<BeamLayerCount> beamLayers; 


//GraphicalDDLayer* CreateGrahpicalLayer();


int angle = 0;
void setup() {
  servo.attach(SERVO_PIN);
  servo.write(angle);


  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input


  for (int i = 0; i < ObjectLayerCount; i++) {
    GraphicalDDLayer* layer = CreateGrahpicalLayer();
    layer->noBackgroundColor();
    objectLayers.initAddLayer(layer);
  }

  for (int i = 0; i < BeamLayerCount; i++) {
    GraphicalDDLayer* layer = CreateGrahpicalLayer();
    layer->noBackgroundColor();
    beamLayers.initAddLayer(layer);
  }

  GraphicalDDLayer* layer = CreateGrahpicalLayer();
  for (int i = 0; i <= MaxAngle; i++) {
    int x;
    int y;
    CalcCoor(i, BoundDist, x, y);
    layer->fillCircle(x, y, 2);
    if (i == 0 || i == MaxAngle) {
      layer->drawLine(x, y, W_half, H);
    }
  }

}


bool angleIncreasing = true;
GraphicalDDLayer* objectLayer = NULL;

void loop() {

  servo.write(angle);
  //delay(15);  // give it some time
  delay(20);  // give it some time

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

  if (true) {
    if (objectLayer == NULL) {
      objectLayer = (GraphicalDDLayer*) objectLayers.useLayer();
    }
    if (distance <= VisibleDist) {
      int x;
      int y;
      CalcCoor(angle, distance, x, y);
      if (false) {
        dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y);
      }
      objectLayer->fillCircle(x, y, 3);
    }
  }

  if (true) {
    GraphicalDDLayer* beamLayer = (GraphicalDDLayer*) beamLayers.useLayer();
    int x;
    int y;
    CalcCoor(angle, BoundDist, x, y);
    beamLayer->drawLine(x, y, W_half, H);
  }


  if (angleIncreasing) {
    angle += AngleIncrement;
  } else {
    angle -= AngleIncrement;
  }
  if (angle > MaxAngle) {
    angle = MaxAngle;
    angleIncreasing = false;
    objectLayer = NULL;
  } if (angle < 0) {
    angle = 0;
    angleIncreasing = true;
    objectLayer = NULL;
  }

}