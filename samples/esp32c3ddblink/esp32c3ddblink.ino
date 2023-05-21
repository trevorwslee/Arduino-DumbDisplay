//**********
//***
//*** for AI Thinker ESP32-C8-32S-KIT
//***
//**********


// for a desciption of the experiment, please watch the YouTube video 
// -- ESP32-C3 Blink Test with Arduino IDE and DumbDisplay
// -- https://www.youtube.com/watch?v=BAnvHOs5Fks


//#define DD_4_ESP32
#include "esp32bledumbdisplay.h"


// - use ESP32 BLE with name "ESP32C3"
// - at the same time, enable Serial connection with 115200 baud 
DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32C3", true, 115200));


#define PIN_COOL  19
#define PIN_WARM  18
#define PIN_RED    3
#define PIN_GREEN  4
#define PIN_BLUE   5


LcdDDLayer* pCool;
LcdDDLayer* pWarm;
LcdDDLayer* pRed;
LcdDDLayer* pGreen;
LcdDDLayer* pBlue;


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);

LcdDDLayer* CreateLayer(const char* label, int pin, const char* color) {
    // create a layer to simulate a button
    LcdDDLayer* button = dumbdisplay.createLcdLayer(12, 3);

    // set pin to the layer's "custom data"
    button->customData = String(pin);  

    // setup appearance of the layer
    button->writeCenteredLine(label, 1);
    button->pixelColor(color);
    button->backgroundColor(DD_COLOR_darkgray);
    button->border(1, DD_COLOR_gray);

    // set "feedback" handler
    button->setFeedbackHandler(FeedbackHandler, "f");
    
    return button;
}

void setup() {
    pinMode(PIN_COOL, OUTPUT);
    pinMode(PIN_WARM, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    digitalWrite(PIN_COOL, 0);

    pCool = CreateLayer("Cool", PIN_COOL, "white");
    pWarm = CreateLayer("Warm", PIN_WARM, "gold");
    pRed = CreateLayer("Red", PIN_RED, "tomato");
    pGreen = CreateLayer("Green", PIN_GREEN, "green");
    pBlue = CreateLayer("Blue", PIN_BLUE, "lightskyblue");

    // auto "pin" the layers (buttons) vertically
    dumbdisplay.configAutoPin(DD_AP_VERT);
}

void loop() {
    // just yield to DumbDisplay to do its work
    DDYield();
}


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    // layer (button) clicked ... get pin from the layer's "custom data"
    int pin = pLayer->customData.toInt();

    // flash the onboard LED of the pin 
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
}

