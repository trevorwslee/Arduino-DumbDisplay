#include "dumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

MbDDLayer* pMbLayer = NULL;

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // enable "feedback" -- auto flashing the clicked area
    pMbLayer->enableFeedback("fa");
}

void loop() {
    // check for "feedback"
    const DDFeedback *pFeedback = pMbLayer->getFeedback();
    if (pFeedback != NULL) {
        // act upon "feedback"
        pMbLayer->toggle(pFeedback->x, pFeedback->y); 
    }
}
