

#define SERVO_PIN    11

#define US_TRIG_PIN   9
#define US_ECHO_PIN  10





const int Width = 400;
const int Height = Width / 2;
const int W_half = Height;
const int H = Height;
const int MaxDist = W_half - 20;
const float DistFactor = W_half / 50.0;

const int MaxAngle = 120;
const int AngleIncrement = 1;

const int A_start = (180 - MaxAngle) / 2;



bool CalcCoor(int ang, int dist, int& x, int& y) {
  float dist_norm = DistFactor * dist;
  if (dist_norm > MaxDist) {
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


#include <Servo.h>
Servo servo;



#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());


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
        if (i == 0) {
          layers[lidx]->clear();
        } else {
          int opacity = i * (100.0 / layerCount);
          layers[lidx]->opacity(opacity);
        }
      }
      return layers[layerIdx];
    }  
  private:
    DDLayer* layers[MAX_LAYER_COUNT];
    int layerCount;
    int nextUseLayerIdx;
};



const int GrahpicalLayerCount = 3;
FadingLayers<GrahpicalLayerCount> graphicalLayers; 



int angle = 0;
void setup() {
  servo.attach(SERVO_PIN);
  servo.write(angle);


  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input


  for (int i = 0; i < GrahpicalLayerCount; i++) {
    GraphicalDDLayer* graphicalLayer = dumbdisplay.createGraphicalLayer(Width, Height);
    graphicalLayer->noBackgroundColor();
    graphicalLayers.initAddLayer(graphicalLayer);
  }


}


bool angleIncreasing = true;
GraphicalDDLayer* graphicalLayer = NULL;

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

  //distance = 40;
  int x;
  int y;
  int r = 3;

  if (graphicalLayer == NULL) {
    graphicalLayer = graphicalLayers.useLayer();
  }
  //GraphicalDDLayer* graphicalLayer = graphicalLayers.useLayer();
  if (CalcCoor(angle, distance, x, y)) {
    if (true) {
      dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y + " LID:" + graphicalLayer->getLayerId());
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
    graphicalLayer = NULL;
  } if (angle < 0) {
    angle = 0;
    angleIncreasing = true;
    graphicalLayer = NULL;
  }
}