#include "dumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

MbDDLayer* pMbLayer = NULL;

void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    // got a click on (x, y) ... toogle it
    pMbLayer->toggle(feedback.x, feedback.y);
}

void setup() {
    // create the MB layer with size 10x10
    pMbLayer = dumbdisplay.createMicrobitLayer(10, 10);
    // setup "callback" function to handle "feedback" passively -- auto flashing the clicked area
    pMbLayer->setFeedbackHandler(FeedbackHandler, "fa");
}

void loop() {
    // give DD a chance to capture "feedback"
    DDYield();
}