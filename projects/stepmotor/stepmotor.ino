#include <Arduino.h>


#define BLUETOOTH



#ifdef BLUETOOTH

#include "ssdumbdisplay.h"
// assume HC-06 connected; 2 => TX of HC06; 3 => RX of HC06
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));

#else

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif



#define PIN_IN1 4
#define PIN_IN2 5
#define PIN_IN3 6
#define PIN_IN4 7
 
#define STEPS_PER_REVOUTION 512

#define MAX_SPEED 10 
#define SPEED_DELAY_MILLIS 10


void SubStep(int v4, int v3, int v2, int v1) {
    digitalWrite(PIN_IN4, v4);
    digitalWrite(PIN_IN3, v3);
    digitalWrite(PIN_IN2, v2);
    digitalWrite(PIN_IN1, v1);
    delay(1);
}

void TurnStep(bool clockwise) {
    if (clockwise) {
        SubStep(1, 0, 0, 1);
        SubStep(0, 0, 0, 1);
        SubStep(0, 0, 1, 1);
        SubStep(0, 0, 1, 0);
        SubStep(0, 1, 1, 0);
        SubStep(0, 1, 0, 0);
        SubStep(1, 1, 0, 0);
        SubStep(1, 0, 0, 0);
    } else {
        SubStep(1, 0, 0, 0);
        SubStep(1, 1, 0, 0);
        SubStep(0, 1, 0, 0);
        SubStep(0, 1, 1, 0);
        SubStep(0, 0, 1, 0);
        SubStep(0, 0, 1, 1);
        SubStep(0, 0, 0, 1);
        SubStep(1, 0, 0, 1);
    }
}




const int GaugeDotRadius = 6;//4;
const int GaugeDotOffRadius = GaugeDotRadius - 2;
const int GaugeRadius = 128;
const int GaugeLayerWidth = 2 * (GaugeRadius + GaugeDotRadius);
const int GaugeLayerHeight = 2 * (GaugeRadius + 2 * GaugeDotRadius);
const String GaugeDotToColor = DD_INT_COLOR(0xFF0000);
const String GaugeDotOnColor = DD_INT_COLOR(0x006400);
const String GaugeDotOffColor = DD_INT_COLOR(0xD3D3D3);
const String GaugeBgColor = DD_INT_COLOR(0xF0FFFF);     // azure



void AngleToXY(int angle, int radius, int& x, int& y) {
  float r = angle * 0.0174533;
  float s = sin(r);
  float c = cos(r);
  float base = radius * c;
  float height = radius * s;
  x = GaugeRadius + GaugeDotRadius + round(base);
  y = GaugeRadius + GaugeDotRadius - round(height) + 5;  // +5 is just some adjustment for the UI
}
int XYToAngle(int x, int y) {
  float base = x - GaugeDotRadius - GaugeRadius;
  float height = GaugeDotRadius + GaugeRadius - y + 5;  // +5 is just some adjustment for the UI
  int angle = 90;
  if (base != 0) {
    float r = atan(abs(height) / base);
    angle = round(r / 0.0174533);
    if (angle < 0) {
      angle = 180 + angle;
    }
  }
  if (height < 0) {
      angle = 360 - angle;
  }
  return angle;  
}

int GaugeAngleToAngle(int gaugeAngle) {
  // angle will be in 5 degress increment
  int angle = 5 * round(((float) gaugeAngle / 5.0));
  if (angle >= 360) {
      angle = angle - 360;
  }
  return angle;
}




LcdDDLayer *calibrateLayer;
LcdDDLayer *controlLayer;
LedGridDDLayer *turnSpeedLayer;
GraphicalDDLayer *gaugeLayer;



void DrawStepMotorPointer(int atAngle, int toAngle, bool turnOn) {
// Serial.print(atAngle);
// Serial.print(".");
// Serial.print(toAngle);
// Serial.print(" = ");
// Serial.println(turnOn);    
  int atX;
  int atY;
  AngleToXY(atAngle, GaugeRadius, atX, atY);
  int toX;
  int toY;
  AngleToXY(toAngle, GaugeRadius, toX, toY);
  if (turnOn) {
    gaugeLayer->fillCircle(atX, atY, GaugeDotRadius, GaugeDotOnColor);
    gaugeLayer->fillCircle(toX, toY, GaugeDotRadius, GaugeDotToColor);
    if (atAngle == toAngle) {
        gaugeLayer->fillCircle(toX, toY, GaugeDotOffRadius, GaugeDotOnColor);
    } else {
        gaugeLayer->fillCircle(toX, toY, GaugeDotOffRadius, GaugeDotOffColor);
    }
  } else {
    gaugeLayer->fillCircle(atX, atY, GaugeDotRadius, GaugeBgColor);
    gaugeLayer->fillCircle(atX, atY, GaugeDotOffRadius, GaugeDotOffColor);
    gaugeLayer->fillCircle(toX, toY, GaugeDotRadius, GaugeBgColor);
    gaugeLayer->fillCircle(toX, toY, GaugeDotOffRadius, GaugeDotOffColor);
  }
}




int stepMotorAtStep = 0;


bool calibrating = true;
DDValueStore<bool> knownCalibrate(calibrating);
int turnSpeed = 0;
DDValueStore<int> knownTurnSpeed(turnSpeed);

int atAngle = 0;
DDValueStore<int> knownAtAngle(atAngle);
int toAngle = 0;
DDValueStore<int> knownToAngle(toAngle);

DDConnectVersionTracker ddConnectionChecker(-1);



void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    if (pLayer == calibrateLayer) {
        calibrating = true;
    } else if (pLayer == controlLayer) {
        calibrating = false;
    } else if (pLayer == turnSpeedLayer) {
        turnSpeed = feedback.x - MAX_SPEED;
    } else if (pLayer == gaugeLayer) {
        int gaugeAngle = XYToAngle(feedback.x, feedback.y);
        int angle = GaugeAngleToAngle(gaugeAngle);
        if (calibrating) {
            if (type == DOUBLECLICK) {
                atAngle = angle;
                toAngle = angle;
                stepMotorAtStep = (gaugeAngle / 360.0) * STEPS_PER_REVOUTION;     
            }           
        } else {
            toAngle = angle;
        }
    }
    if (!calibrating) {
        turnSpeed = MAX_SPEED;
        int step = STEPS_PER_REVOUTION * (toAngle / 360.0);
        int stepDiff =  (STEPS_PER_REVOUTION + step - stepMotorAtStep) % STEPS_PER_REVOUTION;
        // Serial.print(toAngle);
        // Serial.print(">");
        // Serial.print(step);
        // Serial.print(".");
        // Serial.println(stepDiff);
        if (stepDiff < (STEPS_PER_REVOUTION) / 2) {
            turnSpeed = -turnSpeed;
        }
    }
}

void setup() {
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);

    dumbdisplay.recordLayerSetupCommands();

    calibrateLayer = dumbdisplay.createLcdLayer(12, 1);
    calibrateLayer->writeCenteredLine("Calibrate");
    calibrateLayer->setFeedbackHandler(FeedbackHandler, "fl");
    controlLayer = dumbdisplay.createLcdLayer(10, 1);
    controlLayer->writeCenteredLine("Control");
    controlLayer->setFeedbackHandler(FeedbackHandler, "fl");


    turnSpeedLayer = dumbdisplay.createLedGridLayer(2 * MAX_SPEED + 1, 1, 1, 3);
    turnSpeedLayer->border(0.2, "darkgray");
    turnSpeedLayer->offColor("lightgray");
    turnSpeedLayer->setFeedbackHandler(FeedbackHandler, "fa:rpt50");


    gaugeLayer = dumbdisplay.createGraphicalLayer(GaugeLayerWidth, GaugeLayerHeight);
  
    gaugeLayer->drawStr(2 * GaugeRadius - 3 * GaugeDotRadius + 8, GaugeRadius + 3, "0");
    gaugeLayer->drawStr(GaugeRadius - 1, 14, "90");
    gaugeLayer->drawStr(2 * GaugeDotRadius, GaugeRadius + 3, "180");
    gaugeLayer->drawStr(GaugeRadius - 5, 2 * GaugeRadius - 11, "270");
    
    for (int angle = 0; angle <= 355; angle += 5) {
        int x;
        int y;
        AngleToXY(angle, GaugeRadius, x, y);
        gaugeLayer->fillCircle(x, y, GaugeDotOffRadius, GaugeDotOffColor);
    } 

     gaugeLayer->setFeedbackHandler(FeedbackHandler, "fs");



    dumbdisplay.configAutoPin(
        DD_AP_VERT_2(
            DD_AP_HORI_2(calibrateLayer->getLayerId(), controlLayer->getLayerId()),
            DD_AP_STACK_2(gaugeLayer->getLayerId(), DD_AP_PADDING(20, 20, 20, 20, turnSpeedLayer->getLayerId()))
    ));

    dumbdisplay.playbackLayerSetupCommands("stepmotor");
}



void loop() {
    int delayMillis = -1;

    // turn step motor
    if (turnSpeed != 0) {
        // turn step motor
        bool turn;
        if (calibrating) {
            turn = true;
        } else {
            int gaugeAngle = (360.0 * stepMotorAtStep) / STEPS_PER_REVOUTION;
            int angle = GaugeAngleToAngle(gaugeAngle);
            turn = angle != toAngle; 
        }
        if (turn) {
            bool clockwise = turnSpeed > 0;
            TurnStep(clockwise);
            delayMillis = SPEED_DELAY_MILLIS * (MAX_SPEED - abs(turnSpeed));
            // calculate and record where it is at
            stepMotorAtStep += clockwise ? -1 : 1;
            if (stepMotorAtStep == -1)
                stepMotorAtStep = STEPS_PER_REVOUTION - 1;
            else
                stepMotorAtStep %= STEPS_PER_REVOUTION;
            int gaugeAngle = (360.0 * stepMotorAtStep) / STEPS_PER_REVOUTION;
            atAngle = GaugeAngleToAngle(gaugeAngle);
        }
    }

    // update UI
    bool forceRefreshUI = ddConnectionChecker.checkChanged(dumbdisplay);
    if (forceRefreshUI) {
        gaugeLayer->backgroundColor(GaugeBgColor);
    }
    if (knownTurnSpeed.set(turnSpeed) || forceRefreshUI) {
        int count = turnSpeed;
        int xStart;
        const char* color;
        if (turnSpeed < 0) {
            xStart = MAX_SPEED;
            color = "red"; 
        } else if (turnSpeed > 0) {
            xStart = MAX_SPEED + 1;
            color = "green";
        } else {
            count = 1;
            xStart = MAX_SPEED;
            color = "blue"; 
        }
        turnSpeedLayer->horizontalBarEx(count, xStart, color);
    }
    if (knownCalibrate.set(calibrating) || forceRefreshUI) {
        if (calibrating) {
            calibrateLayer->backgroundColor("gray");
            controlLayer->noBackgroundColor();
            turnSpeedLayer->setTransparent(false);

        } else {
            calibrateLayer->noBackgroundColor();
            controlLayer->backgroundColor("gray");
            turnSpeedLayer->setTransparent(true);
        }
    }
    int lastAtAngle = knownAtAngle.get();
    int lastToAngle = knownToAngle.get();
    bool atAngleChanged = knownAtAngle.set(atAngle);
    bool toAngleChanged = knownToAngle.set(toAngle); 
    if (atAngleChanged || toAngleChanged || forceRefreshUI) {
        dumbdisplay.recordLayerCommands();
        DrawStepMotorPointer(lastAtAngle, lastToAngle, false);
        DrawStepMotorPointer(atAngle, toAngle, true);
        dumbdisplay.playbackLayerCommands();
    }

    if (delayMillis > 0)
        DDDelay(delayMillis);
    else
        DDYield();    
}

