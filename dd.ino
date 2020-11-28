#include "ssdumbdisplay.h"
#include "ddtester.h"


#define MB
#define TURTLE

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), true));


DDTester ddTester;
#ifdef MB
MbDDTester *pMbTester = NULL;
#endif
#ifdef TURTLE
TurtleDDTester *pTurtleTester = NULL;
#endif

void setup() {
#ifdef TURTLE
  pTurtleTester = CreateTurtleTester(dumbdisplay);
#endif
#ifdef MB
  pMbTester = CreateMbTester(dumbdisplay);
#endif
}

int stepCount = 0;
void loop() {
#ifdef TURTLE
  pTurtleTester->testStep(stepCount);
#endif
#ifdef MB
  pMbTester->testStep(stepCount);
#endif
  delay(1000);
  stepCount++;
}
