/**
 * to run and see the result of this sketch, you will need two addition things:
 * . you will need to install Android DumbDisplay app from Play store
 *   https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay
 * . although there are several ways for microcontroller board to establish connection
 *   with DumbDisplay app, here, the simple OTG USB connection is assume;
 *   hence, you will need an OTG adaptor cable for connecting your microcontroller board
 *   to your Android phone
 * . after uploading the sketch to your microcontroller board, plug the USB cable
 *   to the OTG adaptor connected to your Android phone
 * . open the DumbDisplay app and make connection to your microcontroller board via the USB cable;
 *   hopefully, the UI is obvious enough :)
 * . for more details on DumbDisplay Arduino Library, please refer to
 *   https://github.com/trevorwslee/Arduino-DumbDisplay#readme
 * there is a related post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */


#include "dumbdisplay.h"

// create the DumbDisplay object; assuming USB connection with the default 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput());

// declare a Microbit-like layer object, to be created in setup()
MbDDLayer* mb;



// "feedback" handler ... to be referred to in setup()
void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    // got a click on (x, y) ... toggle it
    mb->toggle(feedback.x, feedback.y);
}


void setup() {
    // create the Microbit-like layer with size 10x10
    mb = dumbdisplay.createMicrobitLayer(10, 10);

    // Create a boarder around the layer (size 0.5 ~= 5% of a dot)
    mb->border(0.5, "black");
    
    // setup "callback" function to handle "feedback" event-driven -- auto flashing the clicked area
    mb->setFeedbackHandler(FeedbackHandler, "fa");
}

void loop() {
    // give DD a chance to capture "feedback" ... needed for event-driven "feedback" handling
    DDYield();
}

