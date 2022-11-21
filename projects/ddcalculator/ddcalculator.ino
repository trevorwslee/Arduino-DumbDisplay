
#if defined(ESP32)
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("LILIGO", true, 115200));
#else
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif

const char Keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

LcdDDLayer *CreateKeyLayer(int r, int c)
{
    String key = String(Keys[r][c]);
    LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 24, "DL:Roboto Mono");
    keyLayer->writeLine(key);
    keyLayer->customData = key;
    return keyLayer;
}

LcdDDLayer *keyLayers[4][4];

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
        for (int c = 0; c < 4; c++)
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
    // dumbdisplay.writeComment(autoPin);

    dumbdisplay.configAutoPin(autoPin);
}

void loop()
{
}
