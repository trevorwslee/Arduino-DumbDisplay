// for a desciption of the Step Motor experiment, please watch the YouTube video 
// -- Arduino experiment using Raspberry Pi Pico on 28BYJ-48 Step Motor with ULN2003 and DumbDisplay
// -- https://www.youtube.com/watch?v=lMFkCNaTh84


#include <Arduino.h>



// comment out BLUETOOTH if don't have HC-06 connectivity
// . GP8 => RX of HC-06; GP9 => TX of HC-06; ; HC-06 should be configured to use baud rate of 115200
// if no HC-06 connectivity, will need to use DumbDisplayWifiBridge
#define BLUETOOTH


#ifdef BLUETOOTH

// GP8 => RX of HC-06; GP9 => TX of HC-06
#define DD_4_PICO_TX 8
#define DD_4_PICO_RX 9
#include "picodumbdisplay.h"
DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));

#else

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif



// comment out PIN_IN1 if don't have step motor attached
#define PIN_IN1 14
#define PIN_IN2 15
#define PIN_IN3 16
#define PIN_IN4 17
 

#define MAX_SPEED 10 

#define STEPS_PER_REVOUTION 512




void SubStep(int v4, int v3, int v2, int v1) {
#ifdef PIN_IN1    
    digitalWrite(PIN_IN4, v4);
    digitalWrite(PIN_IN3, v3);
    digitalWrite(PIN_IN2, v2);
    digitalWrite(PIN_IN1, v1);
#endif    
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




const int GaugeDotRadius = 6;
const int GaugeDotOffRadius = GaugeDotRadius - 2;
const int GaugeRadius = 128;
const int GaugeLayerWidth = 2 * (GaugeRadius + GaugeDotRadius);
const int GaugeLayerHeight = 2 * (GaugeRadius + 2 * GaugeDotRadius);
const String GaugeDotToColor = DD_INT_COLOR(0xFF0000);
const String GaugeDotOnColor = DD_INT_COLOR(0x006400);
const String GaugeDotOffColor = DD_INT_COLOR(0xD3D3D3);
const String GaugeBgColor = DD_INT_COLOR(0xF0FFFF);



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



void DrawMotorPointer(int atAngle, int toAngle, bool turnOn) {
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



int motorAtStep = 0;
int calibratedAngle = 0;

DDValueRecord<int> atAngle(0);
DDValueRecord<int> toAngle(0);

DDValueRecord<bool> calibrating(true);
DDValueRecord<int> turnSpeed(0);

DDConnectVersionTracker ddConnectionChecker;


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    if (pLayer == controlLayer) {
        calibrating = false;
    } else if (pLayer == calibrateLayer) {
        calibrating = true;
        turnSpeed = 0;
        toAngle = calibratedAngle;
    } else if (pLayer == turnSpeedLayer) {
        turnSpeed = feedback.x - MAX_SPEED;
    } else if (pLayer == gaugeLayer) {
        int gaugeAngle = XYToAngle(feedback.x, feedback.y);
        int angle = GaugeAngleToAngle(gaugeAngle);
        if (calibrating) {
            if (type == DDFeedbackType::DOUBLECLICK) {
                calibratedAngle = angle;
                atAngle = angle;
                toAngle = angle;
                motorAtStep = (gaugeAngle / 360.0) * STEPS_PER_REVOUTION;     
            }           
        } else {
            toAngle = angle;
        }
    }
    if (!calibrating) {
        turnSpeed = MAX_SPEED;
        int step = STEPS_PER_REVOUTION * (toAngle / 360.0);
        int stepDiff =  (STEPS_PER_REVOUTION + step - motorAtStep) % STEPS_PER_REVOUTION;
        if (stepDiff < (STEPS_PER_REVOUTION) / 2) {
            turnSpeed = -turnSpeed;
        }
    }
}


void setup() {
#ifdef PIN_IN1    
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
#endif

    dumbdisplay.recordLayerSetupCommands();

    calibrateLayer = dumbdisplay.createLcdLayer(12, 1);
    calibrateLayer->backgroundColor(GaugeBgColor);
    calibrateLayer->writeCenteredLine("Calibrate");
    calibrateLayer->setFeedbackHandler(FeedbackHandler, "fl");
    
    controlLayer = dumbdisplay.createLcdLayer(10, 1);
    controlLayer->backgroundColor(GaugeBgColor);
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
            int gaugeAngle = (360.0 * motorAtStep) / STEPS_PER_REVOUTION;
            int angle = GaugeAngleToAngle(gaugeAngle);
            turn = angle != toAngle; 
        }
        if (turn) {
            bool clockwise = turnSpeed > 0;
            TurnStep(clockwise);
            delayMillis = 10 * (MAX_SPEED - abs(turnSpeed.get()));
            // calculate and record where it is at
            motorAtStep += clockwise ? -1 : 1;
            if (motorAtStep == -1)
                motorAtStep = STEPS_PER_REVOUTION - 1;
            else
                motorAtStep %= STEPS_PER_REVOUTION;
            int gaugeAngle = (360.0 * motorAtStep) / STEPS_PER_REVOUTION;
            atAngle = GaugeAngleToAngle(gaugeAngle);
        }
    }

    // update UI
    bool forceRefreshUI = ddConnectionChecker.checkChanged(dumbdisplay);
    if (forceRefreshUI) {
        gaugeLayer->backgroundColor(GaugeBgColor);
    }
    bool turnSpeedChanged = turnSpeed.record();
    if (turnSpeedChanged || forceRefreshUI) {
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
    bool calibratingChanged = calibrating.record();
    if (calibratingChanged || forceRefreshUI) {
        if (calibrating) {
            calibrateLayer->border(2, GaugeDotOffColor);
            controlLayer->border(2, GaugeDotOffColor, "hair");
            turnSpeedLayer->transparent(false);
        } else {
            calibrateLayer->border(2, GaugeDotOffColor, "hair");
            controlLayer->border(2, GaugeDotOffColor);
            turnSpeedLayer->transparent(true);
        }
    }
    int lastAtAngle = atAngle.getRecorded();
    int lastToAngle = toAngle.getRecorded();
    bool atAngleChanged = atAngle.record();
    bool toAngleChanged = toAngle.record(); 
    if (atAngleChanged || toAngleChanged || forceRefreshUI) {
        dumbdisplay.recordLayerCommands();
        DrawMotorPointer(lastAtAngle, lastToAngle, false);
        DrawMotorPointer(atAngle, toAngle, true);
        dumbdisplay.playbackLayerCommands();
    }

    if (delayMillis > 0)
        DDDelay(delayMillis);
    else
        DDYield();    
}

