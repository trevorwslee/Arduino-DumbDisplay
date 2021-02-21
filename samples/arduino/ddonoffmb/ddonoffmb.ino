#include <ssdumbdisplay.h>

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

MbDDLayer* pMbLayer = NULL;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, int x, int y) {
    // got a click on (x, y) ... toogle it
    pMbLayer->toggle(x, y);
}

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // set handler to "feedback"
    pMbLayer->setFeedbackHandler(FeedbackHandler, "f");
}

void loop() {
    // give DD a chance to capture "feedback"
    DDYield();
}