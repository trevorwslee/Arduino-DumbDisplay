#include <ssdumbdisplay.h>

DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));

MbDDLayer* pMbLayer = NULL;

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // enable "feedback"
    pMbLayer->enableFeedback("f");
}

void loop() {
    // check for "feedback"
    const DDFeedback *pFeedback = pMbLayer->getFeedback();
    if (pFeedback != NULL) {
        // act upon "feedback"
        pMbLayer->toggle(pFeedback->x, pFeedback->y); 
    }
}