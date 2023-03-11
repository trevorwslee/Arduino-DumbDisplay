#include <Arduino.h>


//#define UNO_BLUETOOTH
//#define PICO_WIFI



#define SERVO_PIN    11

#define US_TRIG_PIN   9
#define US_ECHO_PIN  10



const char* CompiledAt = __DATE__ " " __TIME__;

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


const int ObjectDotRadius = 2;




void CalcCoor(int ang, int dist, int& x, int& y) {
  float dist_norm = DistFactor * dist;
  int ang_norm_i = 180 - (A_start + ang);
  float rad_i = ang_norm_i * 3.1416 / 180.0;
  float y_i = dist_norm * sin(rad_i);
  float x_i = dist_norm * cos(rad_i);
  x = W_half - x_i;
  y = H - y_i;
}


#include <Servo.h>
Servo servo;


#if defined(UNO_BLUETOOTH)

  #include "ssdumbdisplay.h"
  // assume HC-06 connected; 2 => TX of HC06; 3 => RX of HC06
  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));

#elif defined(PICO_WIFI)

  //#include "wifidumbdisplay.h"
  //DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

#endif


GraphicalDDLayer* CreateGrahpicalLayer(const char* backgroundColor = NULL) {
  GraphicalDDLayer* layer = dumbdisplay.createGraphicalLayer(Width, Height);
  layer->border(5, "darkblue", "round");
  layer->padding(3);
  layer->penColor("green");
  if (backgroundColor != NULL) {
    layer->backgroundColor(backgroundColor);
  } else {
    layer->noBackgroundColor();
  }
  return layer;
}




DDLayoutHelper layoutHelper(dumbdisplay);



const int ObjectLayerCount = 2;
DDFadingLayers<ObjectLayerCount> objectLayers; 

const int BeamLayerCount = 4;
DDFadingLayers<BeamLayerCount> beamLayers; 



GraphicalDDLayer* mainLayer;


int angle = 0;
bool isRunning = false;

void setup() {
  servo.attach(SERVO_PIN);
  servo.write(angle);


  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input

  dumbdisplay.writeComment(String("Sketch compiled @ ") + CompiledAt);
  layoutHelper.startInitializeLayout();
    
  for (int i = 0; i < ObjectLayerCount; i++) {
    GraphicalDDLayer* layer = CreateGrahpicalLayer();
    objectLayers.initAddLayer(layer);
  }

  for (int i = 0; i < BeamLayerCount; i++) {
    GraphicalDDLayer* layer = CreateGrahpicalLayer();
    beamLayers.initAddLayer(layer);
  }

  mainLayer = CreateGrahpicalLayer("gray");

  layoutHelper.finishInitializeLayout("ddradar");

  layoutHelper.setIdleCalback([](long idleForMillis) {
    isRunning = false;  // if idle, e.g. disconnected, stop whatever
  });
}



bool angleIncreasing = true;
GraphicalDDLayer* objectLayer = NULL;


int prevAngle = -1;
int prevDistance = -1;

int objectStartAngle = -1;
int objectStartDistance = -1;

void loop() {

  // if (layoutHelper.checkNeedToInitializeLayout()) {
  //   layoutHelper.startInitializeLayout(dumbdisplay);
     
  //   for (int i = 0; i < ObjectLayerCount; i++) {
  //     GraphicalDDLayer* layer = CreateGrahpicalLayer();
  //     objectLayers.initAddLayer(layer);
  //   }

  //   for (int i = 0; i < BeamLayerCount; i++) {
  //     GraphicalDDLayer* layer = CreateGrahpicalLayer();
  //     beamLayers.initAddLayer(layer);
  //   }

  //   mainLayer = CreateGrahpicalLayer("gray");

  //   layoutHelper.finishInitializeLayout(dumbdisplay, "ddradar");
  // }

  if (layoutHelper.checkNeedToUpdateLayers()) {
      mainLayer->fillArc(0, 0, Width, 2 * Height, 180 + A_start, MaxAngle, true, "black");
      mainLayer->drawArc(0, 0, Width, 2 * Height, 180 + A_start, MaxAngle, true, "red");

      isRunning = true;
  }

  if (!isRunning) {
    return;
  }

  servo.write(angle);
  delay(15);  // give it some time

  // read ultrasoncic sensor for detected object distance
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // . set the trigPin on HIGH state for 10 micro seconds
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  // . read the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(US_ECHO_PIN, HIGH);
  // . calculating the distance ... 0.034: sound travels 0.034 cm per second
  int distance = duration * 0.034 / 2;

  GraphicalDDLayer* beamLayer = (GraphicalDDLayer*) beamLayers.useLayer();
  int x;
  int y;
  CalcCoor(angle, VisibleDist, x, y);
  beamLayer->drawLine(x, y, W_half, H);

  if (objectLayer == NULL) {
    objectLayer = (GraphicalDDLayer*) objectLayers.useLayer();
  }


  bool atBoundary = false;
  if (angleIncreasing && angle == MaxAngle) {
    atBoundary = true;
  } else if (!angleIncreasing && angle == 0) {
    atBoundary = true;
  }

  int objectEndAngle = -1;
  int objectEndDistance = -1;
  bool isNewObject = false;
  if (distance <= VisibleDist) {
    // int x;
    // int y;
    CalcCoor(angle, distance, x, y);
    if (false) {
      dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y);
    }
    objectLayer->fillCircle(x, y, ObjectDotRadius);
    if (objectStartAngle != -1) {
      if  (atBoundary) {
        objectEndAngle = angle;
        objectEndDistance = distance;
      } else {
        if (abs(distance - objectStartDistance) >= 10) {
          objectEndAngle = prevAngle;
          objectEndDistance = prevDistance;
          isNewObject = true;
        }
      }
    } else {
      objectStartAngle = angle;
      objectStartDistance = distance;
    }
  } else {
    objectEndAngle = prevAngle;
    objectEndDistance = prevDistance;
  }

  if (objectEndAngle != -1) {
    if (objectStartAngle != -1) {
      int objectAngle = (objectEndAngle + objectStartAngle) / 2;
      int objectDistance = (objectEndDistance + objectStartDistance) / 2;
      // int x;
      // int y;
      CalcCoor(objectAngle, objectDistance, x, y);
      objectLayer->fillCircle(x, y, 3 * ObjectDotRadius, "red");
    }
    objectStartAngle = -1;
    if (isNewObject) {
      objectStartAngle = angle;
      objectStartDistance = distance;
    }
  }


  prevAngle = angle;
  prevDistance = distance;
  

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