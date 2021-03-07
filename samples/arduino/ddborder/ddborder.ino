#include <ssdumbdisplay.h>


DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));


void setup() {
  for (int i = 0; i < 4; i++) {
    const char* borderType;
    if (i == 0) borderType = "raised";
    else if (i == 1) borderType = "sunken";
    else if (i == 2) borderType = "round";
    else borderType = "flat";
    LcdDDLayer* pLcdLayer = dumbdisplay.createLcdLayer(8, 1);
    pLcdLayer->backgroundColor(DD_RGB_COLOR(222, 222, 222));
    pLcdLayer->writeCenteredLine(borderType, 0);
    pLcdLayer->border(1.5, "gray", borderType);
  }
  dumbdisplay.configAutoPin(DD_AP_VERT);
}

void loop() {
}