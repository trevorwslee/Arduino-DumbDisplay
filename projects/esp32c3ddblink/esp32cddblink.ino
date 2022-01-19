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


void SetLayerNormal(DDLayer* button) {
    button->backgroundColor("white");
    button->border(1, "darkgray", "hair");
}
void SetLayerTurnedOn(DDLayer* button) {
    button->backgroundColor("lightgray");
    button->border(1, "darkgray", "round");
}

LcdDDLayer* CreateLayer(const char* label, int pin) {
    LcdDDLayer* button = dumbdisplay.createLcdLayer(12, 3);
    button->customData = String(pin);
    button->writeCenteredLine(label, 1);
    button->setFeedbackHandler(FeedbackHandler, "fl");
    SetLayerNormal(button);
    return button;
}

void setup() {
    pinMode(PIN_COOL, OUTPUT);
    pinMode(PIN_WARM, OUTPUT);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    digitalWrite(PIN_COOL, 0);

    pBlue = CreateLayer("Blue", PIN_BLUE);
    pGreen = CreateLayer("Green", PIN_GREEN);
    pRed = CreateLayer("Red", PIN_RED);
    pWarm = CreateLayer("Warm", PIN_WARM);
    pCool = CreateLayer("Cool", PIN_COOL);

    dumbdisplay.configAutoPin(DD_AP_VERT);
}

void loop() {
    DDYield();
    // digitalWrite(PIN_WARM, 1);
    // delay(1000);
    // digitalWrite(PIN_WARM, 0);
    // delay(1000);

    // digitalWrite(PIN_COOL, 1);
    // delay(1000);
    // digitalWrite(PIN_COOL, 0);
    // delay(1000);

    // digitalWrite(PIN_RED, 1);
    // delay(1000);
    // digitalWrite(PIN_RED, 0);
    // delay(1000);

    // digitalWrite(PIN_GREEN, 1);
    // delay(1000);
    // digitalWrite(PIN_GREEN, 0);
    // delay(1000);

    // digitalWrite(PIN_BLUE, 1);
    // delay(1000);
    // digitalWrite(PIN_BLUE, 0);
    // delay(1000);
}


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
    int pin = pLayer->customData.toInt();
    SetLayerTurnedOn(pLayer);
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
    SetLayerNormal(pLayer);

}

