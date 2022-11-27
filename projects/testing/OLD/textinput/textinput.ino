

#include "ssdumbdisplay.h"

// assume HC-06 connected; 2 => TX of HC06; 3 => RX of HC06
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));



LcdDDLayer* lcd;

void setup() {
    lcd = dumbdisplay.createLcdLayer(32, 3);
    lcd->enableFeedback("f:keys");
}

void loop() {
    DDFeedback* feedback = lcd->getFeedback();
    if (feedback != NULL) {
        if (feedback->text.length() > 0) {
            lcd->writeLine(feedback->text, 1);
        }
    }
}