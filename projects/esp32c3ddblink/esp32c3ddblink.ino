#define DD_4_ESP32
#include "esp32bledumbdisplay.h"


DumbDisplay dumbdisplay(new DDBLESerialIO("ESP32C3", true, 115200));


// AI Thinker ESP32-C8-32S-KIT
// IO5 接 RGB 的蓝色灯珠；IO3 接 RGB 的红色灯珠；IO4 接 RGB 的绿色灯珠；IO19 接冷色灯珠；IO18 接暖色灯珠；  

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
    button->writeCenteredLine(label, 1);
    button->setFeedbackHandler(FeedbackHandler, "f");
    button->pixelColor(color);
    button->backgroundColor("darkgray");
    button->border(1, "lightgray");
    return button;
}

void setup() {
    pinMode(PIN_COOL, OUTPUT);
    pinMode(PIN_WARM, OUTPUT);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    digitalWrite(PIN_COOL, 0);

    pBlue = CreateLayer("Blue", PIN_BLUE, "lightskyblue");
    pGreen = CreateLayer("Green", PIN_GREEN, "green");
    pRed = CreateLayer("Red", PIN_RED, "tomato");
    pWarm = CreateLayer("Warm", PIN_WARM, "gold");
    pCool = CreateLayer("Cool", PIN_COOL, "white");

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
