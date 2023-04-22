
#ifdef PICO_HC_BLUETOOTH

  // GP8 => RX of HC06; GP9 => TX of HC06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  /* HC-06 connectivity */
  DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

#endif



#define SERVO_PIN    10

#define US_TRIG_PIN  14
#define US_ECHO_PIN  15

#ifdef PICO_TOF
  UART Serial3(12, 13, 0, 0);   // TX: 12; RX: 13
  #define TOF Serial3
#endif



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


#ifdef SERVO_PIN
  #include <Servo.h> 
  Servo servo;
 #define SERVO servo
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
PlotterDDLayer* plotterLayer;
LcdDDLayer* uniDirectionalLayer;
LcdDDLayer* biDirectionalLayer;
LcdDDLayer* noDirectionalLayer;
LcdDDLayer* ultraSonicLayer;


bool measureWithUS = true;
short rotateType = 0;    //  0: undirecional; 1: bi-directional; 2: fixed 
int angle = 0;
bool isRunning = false;

void setup() {
#ifdef TOF
  TOF.begin(9600);
  TOF.print("s5-1#");  // set not to auto send distance readins
  measureWithUS = false;
#endif  
#ifdef US_TRIG_PIN
  measureWithUS = true;
#endif

#ifdef SERVO
  SERVO.attach(SERVO_PIN);
  SERVO.write(angle);
#endif


#ifdef US_TRIG_PIN
  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input
#endif

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

  plotterLayer = dumbdisplay.createPlotterLayer(400, 160);
  plotterLayer->border(2, "blue");

  uniDirectionalLayer = dumbdisplay.createLcdLayer(20, 1);
  uniDirectionalLayer->enableFeedback("f");

  biDirectionalLayer = dumbdisplay.createLcdLayer(20, 1);
  biDirectionalLayer->enableFeedback("f");

  noDirectionalLayer = dumbdisplay.createLcdLayer(20, 1);
  noDirectionalLayer->enableFeedback("f");

#if defined(US_TRIG_PIN) && defined (TOF)    
  ultraSonicLayer = dumbdisplay.createLcdLayer(20, 1);
  ultraSonicLayer->enableFeedback("f");
#else
  ultraSonicLayer = NULL;    
#endif

  layoutHelper.configAutoPin(DDAutoPinConfig('V')
    .addRemainingGroup('S')
    .addLayer(plotterLayer)
    .addLayer(uniDirectionalLayer)
    .addLayer(biDirectionalLayer)
    .addLayer(noDirectionalLayer)
    .addLayer(ultraSonicLayer)
    .build());

  layoutHelper.finishInitializeLayout("ddradar");

  layoutHelper.setIdleCalback([](long idleForMillis) {
    isRunning = false;  // if idle, e.g. disconnected, stop whatever
  });

#ifndef SERVO
  uniDirectionalLayer->disabled(true);
  biDirectionalLayer->disabled(true);
  angle = MaxAngle / 2;
  rotateType = 2;
#endif  

}



bool angleIncreasing = true;
GraphicalDDLayer* objectLayer = NULL;


int prevAngle = -1;
int prevDistance = -1;

int objectStartAngle = -1;
int objectStartDistance = -1;

void loop() {
  bool needToUpdateUI = layoutHelper.checkNeedToUpdateLayers(); 

  bool oriMeasureWithUS = measureWithUS;
  short oriRotateType = rotateType;
  if (uniDirectionalLayer->getFeedback()) {
    rotateType = 0;
    needToUpdateUI = true;
  } 
  if (biDirectionalLayer->getFeedback()) {
    rotateType = 1;
    needToUpdateUI = true;
  }
  if (noDirectionalLayer->getFeedback()) {
    rotateType = 2;
    needToUpdateUI = true;
  }
  if (ultraSonicLayer != NULL && ultraSonicLayer->getFeedback()) {
    measureWithUS = !measureWithUS;
    needToUpdateUI = true;
  }

  if (needToUpdateUI) {
      mainLayer->fillArc(0, 0, Width, 2 * Height, 180 + A_start, MaxAngle, true, "black");
      for (int i = 0; i < 4; i++) {
        int x = i * Width / 8;
        int y = i * Height / 4;
        mainLayer->drawArc(x, y, Width - 2 * x, 2 * (Height - y), 180 + A_start, MaxAngle, true, "red");
      }
      for (int a = 0; a <= MaxAngle; a += 20) {
        int x;
        int y;
        CalcCoor(a, BoundDist, x, y);
        mainLayer->drawLine(W_half, H, x, y, "red");
      }

      if (rotateType != 0) {
        uniDirectionalLayer->border(2, "darkblue", "hair");
        uniDirectionalLayer->pixelColor("darkgreen");
        uniDirectionalLayer->writeLine("   Single Direction");
      }
      if (rotateType != 1) {
        biDirectionalLayer->border(2, "darkblue", "hair");
        biDirectionalLayer->pixelColor("darkgreen");
        biDirectionalLayer->writeLine("   Bi Directional");
      }
      if (rotateType != 2) {
        noDirectionalLayer->border(2, "darkblue", "hair");
        noDirectionalLayer->pixelColor("darkgreen");
        noDirectionalLayer->writeLine("   No Rotation");
      }
      if (rotateType == 0) {
        uniDirectionalLayer->border(2, "darkblue", "flat");
        uniDirectionalLayer->pixelColor("darkblue");
        uniDirectionalLayer->writeLine("✅  Single Direction");
      } else if (rotateType == 1) {
        biDirectionalLayer->border(2, "darkblue", "flat");
        biDirectionalLayer->pixelColor("darkblue");
        biDirectionalLayer->writeLine("✅  Bi Directional");
      } else if (rotateType == 2) {
        noDirectionalLayer->border(2, "darkblue", "flat");
        noDirectionalLayer->pixelColor("darkblue");
        noDirectionalLayer->writeLine("✅  No Rotation");
      }
      if (ultraSonicLayer != NULL) {
        if (measureWithUS) {
          ultraSonicLayer->border(2, "darkgreen", "flat");
          ultraSonicLayer->pixelColor("darkblue");
          ultraSonicLayer->writeLine("✅  Ultra Sonic");
        } else {
          ultraSonicLayer->border(2, "darkgreen", "hair");
          ultraSonicLayer->pixelColor("darkgray");
          ultraSonicLayer->writeLine("🟩  Ultra Sonic");
        }
      }
      isRunning = true;
  }

  if (!isRunning) {
    int newAngle = 0;
    if (rotateType == 2) {
      newAngle = MaxAngle / 2;
    }
    if (angle != newAngle) {
      angle = newAngle;
#ifdef SERVO
      SERVO.write(angle);
#endif
    }
    return;
  }

  bool clearAll = false;//oriMeasureWithUS != measureWithUS;
  if (oriRotateType != rotateType) {
    if (oriRotateType == 2 || rotateType == 2) { 
      clearAll = true;
    }
  }
  if (clearAll) {
      objectLayers.clear();
      beamLayers.clear();
      plotterLayer->clear();
  }

  if (rotateType == 0 || rotateType == 1) {
#ifdef SERVO
    SERVO.write(angle);
#endif
    if (angle == 0 || angle == MaxAngle) {
      delay(200);
    }
  }
  delay(20);  // delay a bit for each reading

  int usDistance = -1;
#ifdef US_TRIG_PIN
  if (measureWithUS) {
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
    usDistance = duration * 0.034 / 2;
  }
#endif

  int tofDistance = -1;
 #ifdef TOF
  if (!measureWithUS) {
    TOF.print("r6#");  // send command to read distance once ... will get back something like L=18mm
    int count = 0;
    while (count++ < 5) {
      String dist = TOF.readStringUntil('\n');
      if (false) {
        dumbdisplay.writeComment(String(". TOF: [") + dist + "]");
      }
      if (dist.startsWith("L=")) {
        tofDistance = dist.substring(2, dist.indexOf('m')).toInt() / 10;
        if (false) {
          dumbdisplay.writeComment(String("  - tofDist: ") + tofDistance);
        }
        break;
      }
    }
  }
#endif  

  if (true) {
    if (usDistance != -1 && tofDistance != -1) {
      dumbdisplay.writeComment(String(". US: ") + usDistance + " / TOF: " + tofDistance);
    }
  }

  int distance = usDistance != -1 ? usDistance : tofDistance;

  GraphicalDDLayer* beamLayer = (GraphicalDDLayer*) beamLayers.useLayer();
  int x;
  int y;
  CalcCoor(angle, VisibleDist, x, y);
  beamLayer->drawLine(x, y, W_half, H);

  if (rotateType == 0 || rotateType == 1) {
    if (objectLayer == NULL) {
      objectLayer = (GraphicalDDLayer*) objectLayers.useLayer();
    }
  } else {
      objectLayer = (GraphicalDDLayer*) objectLayers.useLayer();
  }

  bool atBoundary = false;
  if (angleIncreasing && angle == MaxAngle) {
    atBoundary = true;
  } else if (!angleIncreasing && angle == 0) {
    atBoundary = true;
  } else {
    if (rotateType == 0) {
      if (angle == 0 || angle == MaxAngle) {
        objectStartAngle = -1;
      }
    }
  }

  int objectEndAngle = -1;
  int objectEndDistance = -1;
  bool isNewObject = false;
  plotterLayer->set("Ang", angle, "Dist", distance > VisibleDist ? 0 : distance);
  if (distance != -1 && distance <= VisibleDist) {
    CalcCoor(angle, distance, x, y);
    if (false) {
      dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y);
    }
    if (rotateType == 0 || rotateType == 1) {
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
        objectLayer->fillCircle(x, y, 3 * ObjectDotRadius, "green");
        objectStartAngle = -1;
        objectEndAngle = -1;
        objectEndDistance = -1;
    }
  } else {
    objectEndAngle = prevAngle;
    objectEndDistance = prevDistance;
  }

  if (objectEndAngle != -1) {
    if (objectStartAngle != -1) {
      int objectAngle = (objectEndAngle + objectStartAngle) / 2;
      int objectDistance = (objectEndDistance + objectStartDistance) / 2;
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
 
  if (rotateType == 0 || rotateType == 1) {
    if (angleIncreasing) {
      angle += AngleIncrement;
    } else {
      angle -= AngleIncrement;
    }
    if (angle > MaxAngle) {
      if (rotateType == 0) {
        angle = 0;
      } else {
        angle = MaxAngle;
        angleIncreasing = false;
      }
      objectLayer = NULL;
    }
    if (angle < 0) {
      if (rotateType == 0) {
        angle = MaxAngle;
      } else {
        angle = 0;
        angleIncreasing = true;
      }
      objectLayer = NULL;
    }
  }
}
