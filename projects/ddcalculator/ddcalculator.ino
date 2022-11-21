
#if defined(ESP32)
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("LILIGO", true, 115200));
#else
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif

const char Keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

LcdDDLayer *CreateKeyLayer(int r, int c);

LcdDDLayer *keyLayers[4][3];

void setup()
{
    String autoPin("V(");
    for (int r = 0; r < 4; r++)
    {
        if (r > 0)
        {
            autoPin += "+";
        }
        autoPin += "H(";
        for (int c = 0; c < 3; c++)
        {
            LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
            keyLayers[r][c] = keyLayer;
            if (c > 0)
            {
                autoPin += "+";
            }
            autoPin += keyLayer->getLayerId();
        }
        autoPin += ")";
    }
    autoPin += ")";

    dumbdisplay.configAutoPin(autoPin);
}

void loop()
{
    DDYield(); // need to call this so that DumbDisplay lib can check "feedback"
}

void FeedbackHandler(DDLayer *pLayer, DDFeedbackType type, const DDFeedback &feedback)
{
    if (type == CLICK)
    {
        char key = pLayer->customData.charAt(0);
        dumbdisplay.writeComment("key [" + String(key) + "]");
    }
}

LcdDDLayer *CreateKeyLayer(int r, int c)
{
    String key = String(Keys[r][c]);
    LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32, "DL:Share Tech Mono");
    keyLayer->pixelColor("navy");
    keyLayer->border(5, "darkgray", "raised");
    keyLayer->padding(1);
    keyLayer->writeLine(key);
    keyLayer->setFeedbackHandler(FeedbackHandler, "fl");
    keyLayer->customData = key;
    return keyLayer;
}
