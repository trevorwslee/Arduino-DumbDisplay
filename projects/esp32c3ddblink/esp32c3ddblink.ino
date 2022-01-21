//**********
//***
//*** for AI Thinker ESP32-C8-32S-KIT
//***
//**********



#define DD_4_ESP32
#include "esp32bledumbdisplay.h"


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
    LcdDDLayer* button = dumbdisplay.createLcdLayer(12, 3);
    button->customData = String(pin);
//dumbdisplay.backgroundColor("lightblue");
    button->writeCenteredLine(label, 1);
//delay(1000);    
    button->pixelColor(color);
//delay(1000);    
    button->backgroundColor("darkgray");
//delay(1000);    
    button->border(1, "gray");
//delay(1000);    
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
//delay(1000);
    pWarm = CreateLayer("Warm", PIN_WARM, "gold");
//delay(1000);
    pRed = CreateLayer("Red", PIN_RED, "tomato");
//delay(1000);
    pGreen = CreateLayer("Green", PIN_GREEN, "green");
//delay(1000);
    pBlue = CreateLayer("Blue", PIN_BLUE, "lightskyblue");

    dumbdisplay.configAutoPin(DD_AP_VERT);
}

void loop() {
    DDYield();
}


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    int pin = pLayer->customData.toInt();
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
}
