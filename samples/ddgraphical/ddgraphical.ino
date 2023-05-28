

#if defined(ESP32)

  #if defined(BLUETOOTH)
    #include "esp32dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
  #elif defined(WIFI_SSID)
    #include "wifidumbdisplay.h"
    DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
  #else
    #include "dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDInputOutput());
  #endif

#else

  #ifdef BLUETOOTH
    // assume HC-06 connected; 2 => TX of HC06; 3 => RX of HC06
    #include "ssdumbdisplay.h"
    DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true));
  #else
  // otherwise, can use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
    #include "dumbdisplay.h"
    DumbDisplay dumbdisplay(new DDInputOutput(115200));
  #endif

#endif



void setup() {
  // create 4 graphical [LCD] layers
  GraphicalDDLayer *pLayer1 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer2 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer3 = dumbdisplay.createGraphicalLayer(151, 101);
  GraphicalDDLayer *pLayer4 = dumbdisplay.createGraphicalLayer(151, 101);

  // set fill screen with color
  pLayer1->fillScreen(DD_COLOR_azure);
  pLayer2->fillScreen(DD_COLOR_azure);
  pLayer3->fillScreen(DD_COLOR_azure);
  pLayer4->fillScreen(DD_COLOR_azure);

  //  configure to "auto pin" the 4 layers
  // -- end result of DD_AP_XXX(...) is the layout spec "H(V(0+1)+V(2+3))"
  // -- . H/V: layout direction
  // -- . 0/1/2/3: layer id
  dumbdisplay.configAutoPin(DD_AP_HORI_2(
                              DD_AP_VERT_2(pLayer1->getLayerId(), pLayer2->getLayerId()),
                              DD_AP_VERT_2(pLayer3->getLayerId(), pLayer4->getLayerId())));

  // draw triangles
  int left = 0;
  int right = 150;
  int top = 0;
  int bottom = 100;
  int mid = 50;
  for (int i = 0; i < 15; i++) {
    left += 3;
    top += 3;
    right -= 3;
    bottom -= 3;
    int x1 = left;
    int y1 = mid;
    int x2 = right;
    int y2 = top;
    int x3 = right;
    int y3 = bottom;
    int r = 25 * i;
    int g = 255 - (10 * i);
    int b = 2 * i;
    pLayer1->drawTriangle(x1, y1, x2, y2, x3, y3, DD_RGB_COLOR(r, g, b));
  }

  // draw lines
  for (int i = 0;; i++) {
    int delta = 5 * i;
    int x1 = 150;
    int y1 = 0;
    int x2 = -150 + delta;
    int y2 = delta;
    pLayer2->drawLine(x1, y1, x2, y2, DD_COLOR_blue);
    if (x2 > 150)
      break;
  }

  // draw rectangles
  for (int i = 0; i < 15; i++) {
    int delta = 3 * i;
    int x = delta;
    int y = delta;
    int w = 150 - 2 * x;
    int h = 100 - 2 * y;
    pLayer3->drawRect(x, y, w, h, DD_COLOR_plum);
  }

  // draw circles
  int radius = 10;
  for (int i = 0; i < 8; i++) {
    int x = 2 * radius * i;
    for (int j = 0; j < 6; j++) {
      int y = 2 * radius * j;
      int r = radius;
      pLayer4->drawCircle(x, y, r, DD_COLOR_teal);
      pLayer4->fillCircle(x + r, y + r, r, DD_COLOR_gold);
    }
  }
}

void loop() {
}
