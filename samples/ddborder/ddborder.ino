#include "dumbdisplay.h"

/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));


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