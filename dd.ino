#include "ssdumbdisplay.h"
#include "ddtester.h"


#define MB
#define TURTLE

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));


DDTester ddTester;
#ifdef MB
MicroBitLayer* pMbLayer = NULL;
#endif
#ifdef TURTLE
TurtleLayer* pTurtleLayer = NULL;
#endif

void setup() {
#ifdef TURTLE
  pTurtleLayer = dumbdisplay.createTurtleLayer(251, 201);
  CreateTurtleTester(dumbdisplay);
#endif
#ifdef MB
  pMbLayer = dumbdisplay.createMicroBitLayer(5, 5);
  CreateMbTester(dumbdisplay);
#endif
}

int stepCount = 0;
void loop() {
#ifdef TURTLE
  ddTester.turtleTestStep(pTurtleLayer, stepCount);
#endif
#ifdef MB
  ddTester.mbTestStep(pMbLayer, stepCount);
#endif
  stepCount++;
}
