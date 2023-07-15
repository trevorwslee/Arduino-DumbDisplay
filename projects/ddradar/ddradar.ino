#include <Arduino.h>

// * you do not need everything, but at least ultrasonic sensor or laser range finder (tof10120)
// * assign pins and/or comment out below pin assignment macros as appropriate
// * - if you do not have ultrasonic sensor, comment out US_TRIG_PIN and US_ECHO_PIN
// * - if you do not have laster range finder (tof10120), command out TOF_RX_PIN and TOF_TX_PIN
// * - if you do not have servo, comment out SERVO_PIN

#define US_TRIG_PIN 4
#define US_ECHO_PIN 5
#define TOF_RX_PIN  12
#define TOF_TX_PIN  13
#define SERVO_PIN   10

// * by default, max servo angle is 90
// * can set the max servo angle with the macro SERVO_MAX_ANGLE
// * if need some simple adjustment, can set the macro SERVO_ADJUST_FACTOR ... the actual angle servo will go to is angle * adjust_factor
// #define SERVO_MAX_ANGLE     120
// #define SERVO_ADJUST_FACTOR 1.2

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());

#ifdef SERVO_PIN
  #ifdef ESP32
    #include <ESP32_Servo.h>
    Servo servo;
    #define SERVO servo
  #else
    #include <Servo.h>
    Servo servo;
    #define SERVO servo
  #endif
#endif

#ifdef TOF_RX_PIN
  UART Serial3(TOF_RX_PIN, TOF_TX_PIN, 0, 0); // TX: 12; RX: 13
  #define TOF Serial3
#endif

const char *CompiledAt = __DATE__ " " __TIME__;

const int Width = 400;
const int Height = Width / 2;
const int W_half = Height;
const int H = Height;
const int BoundDist = 50;
const int VisibleDist = 49;
const float DistFactor = W_half / BoundDist;

#ifdef SERVO_MAX_ANGLE
  #define _MAX_ANGLE SERVO_MAX_ANGLE
#else
  #define _MAX_ANGLE 90
#endif
const int MaxAngle = min(180, max(60, _MAX_ANGLE));
const int AngleIncrement = 1;

const int A_start = (180 - MaxAngle) / 2;

const int ObjectDotRadius = 2;

void CalcCoor(int ang, int dist, int &x, int &y)
{
  float dist_norm = DistFactor * dist;
  int ang_norm_i = 180 - (A_start + ang);
  float rad_i = ang_norm_i * 3.1416 / 180.0;
  float y_i = dist_norm * sin(rad_i);
  float x_i = dist_norm * cos(rad_i);
  x = W_half - x_i;
  y = H - y_i;
}

GraphicalDDLayer *CreateGrahpicalLayer(const char *backgroundColor = NULL)
{
  GraphicalDDLayer *layer = dumbdisplay.createGraphicalLayer(Width, Height);
  layer->border(5, DD_COLOR_darkblue, "round");
  layer->padding(3);
  layer->penColor(DD_COLOR_green);
  if (backgroundColor != NULL)
  {
    layer->backgroundColor(backgroundColor);
  }
  else
  {
    layer->noBackgroundColor();
  }
  return layer;
}

void ServoGoto(int angle)
{
#ifdef SERVO_ADJUST_FACTOR
  angle = (float)angle * SERVO_ADJUST_FACTOR;
#endif
#ifdef SERVO
  SERVO.write(angle);
#endif
}

DDLayoutHelper layoutHelper(dumbdisplay);

const int ObjectLayerCount = 2;
DDFadingLayers<ObjectLayerCount> objectLayers;

const int BeamLayerCount = 4;
DDFadingLayers<BeamLayerCount> beamLayers;

GraphicalDDLayer *mainLayer;
PlotterDDLayer *plotterLayer;

#ifdef SERVO
  LcdDDLayer *uniDirectionalLayer;
  LcdDDLayer *biDirectionalLayer;
  LcdDDLayer *noDirectionalLayer;
#endif

#ifdef SERVO
  JoystickDDLayer *angleSliderLayer;
#endif

LcdDDLayer *ultraSonicLayer;
LcdDDLayer *laserLayer;
SevenSegmentRowDDLayer *distanceLayer;

bool measureWithUS = true;
short rotateType = 0; //  0: undirecional; 1: bi-directional; 2: fixed
int angle = 0;
bool isRunning = false;

void setup()
{
#ifdef TOF
  TOF.begin(9600);
  TOF.print("s5-1#"); // set not to auto send distance readins
  measureWithUS = false;
#endif
#ifdef US_TRIG_PIN
  measureWithUS = true;
#endif

#ifdef SERVO
#ifdef ESP32
  // SERVO.attach(SERVO_PIN, 500, 2400);  // 500 / 2400 for SG90
  SERVO.attach(SERVO_PIN);
#else
  SERVO.attach(SERVO_PIN);
#endif
  ServoGoto(angle);
#endif

#ifdef US_TRIG_PIN
  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT);  // Sets the echoPin as an Input
#endif

  dumbdisplay.writeComment(String("Sketch compiled @ ") + CompiledAt);
  layoutHelper.startInitializeLayout();

  for (int i = 0; i < ObjectLayerCount; i++)
  {
    GraphicalDDLayer *layer = CreateGrahpicalLayer();
    objectLayers.initAddLayer(layer);
  }

  for (int i = 0; i < BeamLayerCount; i++)
  {
    GraphicalDDLayer *layer = CreateGrahpicalLayer();
    beamLayers.initAddLayer(layer);
  }

  mainLayer = CreateGrahpicalLayer("gray");

  plotterLayer = dumbdisplay.createPlotterLayer(400, 160);
  plotterLayer->border(2, "blue");

#ifdef SERVO
  uniDirectionalLayer = dumbdisplay.createLcdLayer(10, 1);
  uniDirectionalLayer->enableFeedback("f");

  biDirectionalLayer = dumbdisplay.createLcdLayer(10, 1);
  biDirectionalLayer->enableFeedback("f");

  noDirectionalLayer = dumbdisplay.createLcdLayer(20, 1);
  noDirectionalLayer->enableFeedback("f");

  angleSliderLayer = dumbdisplay.createJoystickLayer(MaxAngle, "rl", 0.5);
  angleSliderLayer->border(3, "black");
  angleSliderLayer->colors("green", "darkgreen", "blue", "darkblue");
  angleSliderLayer->backgroundColor("cyan");
#endif

  ultraSonicLayer = dumbdisplay.createLcdLayer(15, 1);
  ultraSonicLayer->enableFeedback("f");

  laserLayer = dumbdisplay.createLcdLayer(15, 1);
  laserLayer->enableFeedback("f");

  distanceLayer = dumbdisplay.create7SegmentRowLayer(2);
  distanceLayer->margin(5);
  distanceLayer->border(10, "darkred", "round");

  layoutHelper.configAutoPin(DDAutoPinConfig('V')
    .addRemainingGroup('S')
    .addLayer(plotterLayer)
    .beginGroup('H')
      .beginGroup('V')    
        .addLayer(ultraSonicLayer)
        .addLayer(laserLayer)
      .endGroup()
      .addLayer(distanceLayer)
    .endGroup()    
#ifdef SERVO    
    .addLayer(angleSliderLayer)
    .addLayer(noDirectionalLayer)
    .beginGroup('H')
      .addLayer(uniDirectionalLayer)
      .addLayer(biDirectionalLayer)
    .endGroup()  
#endif
    .build());

  layoutHelper.finishInitializeLayout("ddradar");

  layoutHelper.setIdleCallback([](long idleForMillis, DDIdleConnectionState connectionState) {
    if (connectionState == DDIdleConnectionState::IDLE_RECONNECTING) {
      isRunning = false;
    }
  });

#ifndef SERVO
  angle = MaxAngle / 2;
  rotateType = 2;
#endif
#ifndef US_TRIG_PIN
  ultraSonicLayer->disabled(true);
#endif
#ifndef TOF
  laserLayer->disabled(true);
#endif
}

bool angleIncreasing = true;
GraphicalDDLayer *objectLayer = NULL;

int prevAngle = -1;
int prevDistance = -1;

int objectStartAngle = -1;
int objectStartDistance = -1;

void loop()
{
  bool needToUpdateUI = layoutHelper.checkNeedToUpdateLayers();

  bool oriMeasureWithUS = measureWithUS;
  short oriRotateType = rotateType;

#ifdef SERVO
  if (uniDirectionalLayer->getFeedback())
  {
    rotateType = 0;
  }
  if (biDirectionalLayer->getFeedback())
  {
    rotateType = 1;
  }
  if (noDirectionalLayer->getFeedback())
  {
    rotateType = 2;
  }
#endif

#if defined(US_TRIG_PIN) && defined(TOF)
  if (ultraSonicLayer->getFeedback())
  {
    measureWithUS = true;
  }
  if (laserLayer->getFeedback())
  {
    measureWithUS = false;
  }
#endif

  if (oriRotateType != rotateType || oriMeasureWithUS != measureWithUS)
  {
    needToUpdateUI = true;
  }

  if (needToUpdateUI)
  {
    mainLayer->fillArc(0, 0, Width, 2 * Height, 180 + A_start, MaxAngle, true, "black");
    for (int i = 0; i < 4; i++)
    {
      int x = i * Width / 8;
      int y = i * Height / 4;
      mainLayer->drawArc(x, y, Width - 2 * x, 2 * (Height - y), 180 + A_start, MaxAngle, true, "red");
    }
    for (int a = 0; a <= MaxAngle; a += 20)
    {
      int x;
      int y;
      CalcCoor(a, BoundDist, x, y);
      mainLayer->drawLine(W_half, H, x, y, "red");
    }

#ifdef SERVO
    if (rotateType != 0)
    {
      uniDirectionalLayer->border(2, "darkblue", "hair");
      uniDirectionalLayer->pixelColor("darkgreen");
      uniDirectionalLayer->writeLine("   Single");
    }
    if (rotateType != 1)
    {
      biDirectionalLayer->border(2, "darkblue", "hair");
      biDirectionalLayer->pixelColor("darkgreen");
      biDirectionalLayer->writeLine("   Both");
    }
    if (rotateType != 2)
    {
      noDirectionalLayer->border(2, "darkblue", "hair");
      noDirectionalLayer->pixelColor("darkgreen");
      noDirectionalLayer->writeLine("   No Sweeping");
      angleSliderLayer->disabled(true);
    }
    if (rotateType == 0)
    {
      uniDirectionalLayer->border(2, "darkblue", "flat");
      uniDirectionalLayer->pixelColor("darkblue");
      uniDirectionalLayer->writeLine("✅  Single");
    }
    else if (rotateType == 1)
    {
      biDirectionalLayer->border(2, "darkblue", "flat");
      biDirectionalLayer->pixelColor("darkblue");
      biDirectionalLayer->writeLine("✅  Both");
    }
    else if (rotateType == 2)
    {
      noDirectionalLayer->border(2, "darkblue", "flat");
      noDirectionalLayer->pixelColor("darkblue");
      noDirectionalLayer->writeLine("✅  No Sweeping");
      angleSliderLayer->disabled(false);
    }
#endif
    if (measureWithUS)
    {
      ultraSonicLayer->border(2, "darkblue", "flat");
      ultraSonicLayer->pixelColor("darkblue");
#ifdef US_TRIG_PIN
      ultraSonicLayer->writeLine("✅  Ultra Sonic");
#else
      ultraSonicLayer->writeLine("   Ultra Sonic");
#endif
      laserLayer->border(2, "darkblue", "hair");
      laserLayer->pixelColor("darkgreen");
      laserLayer->writeLine("   Laser");
    }
    else
    {
      ultraSonicLayer->border(2, "darkblue", "hair");
      ultraSonicLayer->pixelColor("darkgreen");
      ultraSonicLayer->writeLine("   Ultra Sonic");
      laserLayer->border(2, "darkblue", "flat");
      laserLayer->pixelColor("darkblue");
#ifdef TOF_RX_PIN
      laserLayer->writeLine("✅  Laser");
#else
      laserLayer->writeLine("   Laser");
#endif
    }
    isRunning = true;
  }

  if (!isRunning)
  {
    int newAngle = 0;
    if (rotateType == 2)
    {
      newAngle = MaxAngle / 2;
    }
    if (angle != newAngle)
    {
      angle = newAngle;
#ifdef SERVO
      ServoGoto(angle);
      angleSliderLayer->moveToPos(angle, 0);
#endif
    }
    return;
  }

  if (oriRotateType != rotateType)
  {
    if (oriRotateType == 2 || rotateType == 2)
    {
      objectLayers.clear();
      beamLayers.clear();
      plotterLayer->clear();
    }
    distanceLayer->clear();
  }
  if (oriMeasureWithUS != measureWithUS)
  {
    distanceLayer->clear();
  }

  if (rotateType == 0 || rotateType == 1)
  {
#ifdef SERVO
    ServoGoto(angle);
    angleSliderLayer->moveToPos(angle, 0);
#endif
    if (angle == 0 || angle == MaxAngle)
    {
      delay(200);
    }
  }
  else
  {
#ifdef SERVO
    const DDFeedback *feedback = angleSliderLayer->getFeedback();
    if (feedback != NULL)
    {
      angle = feedback->x;
      ServoGoto(angle);
    }
#endif
  }
  delay(20); // delay a bit for each reading

  int usDistance = -1;
#ifdef US_TRIG_PIN
  if (measureWithUS)
  {
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
  if (!measureWithUS)
  {
    TOF.print("r6#"); // send command to read distance once ... will get back something like L=156mm
    int count = 0;
    while (count++ < 5)
    {
      String dist = TOF.readStringUntil('\n');
      if (false)
      {
        dumbdisplay.writeComment(String(". TOF: [") + dist + "]");
      }
      if (dist.startsWith("L="))
      {
        tofDistance = dist.substring(2, dist.indexOf('m')).toInt() / 10; //  dist is like "L=156mm"; divide by 10 to get cm
        break;
      }
    }
  }
#endif
  if (false)
  {
    dumbdisplay.writeComment(String(". US: ") + usDistance + " / TOF: " + tofDistance);
  }

  int distance = usDistance != -1 ? usDistance : tofDistance;
  if (distance != -1 && distance <= 99)
  {
    distanceLayer->showNumber(distance);
  }
  else
  {
    distanceLayer->showFormatted("--");
  }

  GraphicalDDLayer *beamLayer = (GraphicalDDLayer *)beamLayers.useLayer();
  int x;
  int y;
  CalcCoor(angle, VisibleDist, x, y);
  beamLayer->drawLine(x, y, W_half, H);

  if (rotateType == 0 || rotateType == 1)
  {
    if (objectLayer == NULL)
    {
      objectLayer = (GraphicalDDLayer *)objectLayers.useLayer();
    }
  }
  else
  {
    objectLayer = (GraphicalDDLayer *)objectLayers.useLayer();
  }

  bool atBoundary = false;
  if (angleIncreasing && angle == MaxAngle)
  {
    atBoundary = true;
  }
  else if (!angleIncreasing && angle == 0)
  {
    atBoundary = true;
  }
  else
  {
    if (rotateType == 0)
    {
      if (angle == 0 || angle == MaxAngle)
      {
        objectStartAngle = -1;
      }
    }
  }

  int objectEndAngle = -1;
  int objectEndDistance = -1;
  bool isNewObject = false;
  if (rotateType == 0 || rotateType == 1)
  {
    plotterLayer->set("Ang", angle, "Dist", distance > VisibleDist ? 0 : distance);
  }
  else
  {
    plotterLayer->set("Dist", distance > VisibleDist ? 0 : distance);
  }
  if (distance != -1 && distance <= VisibleDist)
  {
    CalcCoor(angle, distance, x, y);
    if (false)
    {
      dumbdisplay.writeComment(String("Ang:") + angle + " Dist:" + distance + " X:" + x + " Y:" + y);
    }
    if (rotateType == 0 || rotateType == 1)
    {
      objectLayer->fillCircle(x, y, ObjectDotRadius);
      if (objectStartAngle != -1)
      {
        if (atBoundary)
        {
          objectEndAngle = angle;
          objectEndDistance = distance;
        }
        else
        {
          if (abs(distance - objectStartDistance) >= 10)
          {
            objectEndAngle = prevAngle;
            objectEndDistance = prevDistance;
            isNewObject = true;
          }
        }
      }
      else
      {
        objectStartAngle = angle;
        objectStartDistance = distance;
      }
    }
    else
    {
      objectLayer->fillCircle(x, y, 3 * ObjectDotRadius, "green");
      objectStartAngle = -1;
      objectEndAngle = -1;
      objectEndDistance = -1;
    }
  }
  else
  {
    objectEndAngle = prevAngle;
    objectEndDistance = prevDistance;
  }

  if (objectEndAngle != -1)
  {
    if (objectStartAngle != -1)
    {
      int objectAngle = (objectEndAngle + objectStartAngle) / 2;
      int objectDistance = (objectEndDistance + objectStartDistance) / 2;
      CalcCoor(objectAngle, objectDistance, x, y);
      objectLayer->fillCircle(x, y, 3 * ObjectDotRadius, "red");
    }
    objectStartAngle = -1;
    if (isNewObject)
    {
      objectStartAngle = angle;
      objectStartDistance = distance;
    }
  }

  prevAngle = angle;
  prevDistance = distance;

  if (rotateType == 0 || rotateType == 1)
  {
    if (angleIncreasing)
    {
      angle += AngleIncrement;
    }
    else
    {
      angle -= AngleIncrement;
    }
    if (angle > MaxAngle)
    {
      if (rotateType == 0)
      {
        angle = 0;
      }
      else
      {
        angle = MaxAngle;
        angleIncreasing = false;
      }
      objectLayer = NULL;
    }
    if (angle < 0)
    {
      if (rotateType == 0)
      {
        angle = MaxAngle;
      }
      else
      {
        angle = 0;
        angleIncreasing = true;
      }
      objectLayer = NULL;
    }
  }
}
