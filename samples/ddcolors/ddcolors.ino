
//#define DD_DISABLE_PARAM_ENCODEING


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

GraphicalDDLayer *graphicalLayer;


const char* colors[] = {
  
  DD_COLOR_red,
  DD_COLOR_green,
  DD_COLOR_blue,
  DD_COLOR_yellow,
  DD_COLOR_magenta,
  DD_COLOR_cyan,
  DD_COLOR_white,
  DD_COLOR_black,
  DD_COLOR_orange,
  DD_COLOR_pink,
  DD_COLOR_gray,
  DD_COLOR_darkgray,
  DD_COLOR_lightgray,
  DD_COLOR_brown,
  DD_COLOR_purple,
  DD_COLOR_darkgreen,
  DD_COLOR_darkblue,
  DD_COLOR_indigo,
  DD_COLOR_lightgreen,
  DD_COLOR_darkred,
  DD_COLOR_lightblue,

  DD_COLOR_azure,
  DD_COLOR_plum,
  DD_COLOR_gold,
  DD_COLOR_teal,
  DD_COLOR_ivory,
  DD_COLOR_coral,
  DD_COLOR_beige,
  DD_COLOR_snow,
  DD_COLOR_aqua,
  DD_COLOR_lemonchiffon,
  DD_COLOR_navy,
  DD_COLOR_midnightblue,
  DD_COLOR_aliceblue,

};


const int circleRadius = 10;
const int colorCount = sizeof(colors) / sizeof(colors[0]);
const int circlePadding = 2;
const int circleWidth = 2 * (circlePadding + circleRadius);
const int circleHeight = circleWidth;

int rowColorCount;
int width;
int height;

void setup() {
  rowColorCount = 1 + sqrt(colorCount);

  width = circleWidth * rowColorCount;
  height = width;

  graphicalLayer = dumbdisplay.createGraphicalLayer(width, height);
#if defined(DD_DISABLE_PARAM_ENCODEING)
  graphicalLayer->border(5, DD_COLOR_black, "round");
#else
  graphicalLayer->border(5, DD_COLOR_blue, "round");
#endif

  int x = 0;
  int y = 0;
  for (int i = 0; i < colorCount; i++) {
    const char* color = colors[i];
    int cx = x + circlePadding + circleRadius;
    int cy = y + circlePadding + circleRadius;
    graphicalLayer->fillCircle(cx, cy, circleRadius, color);
    x += circleWidth;
    if (x >= width) {
      x = 0;
      y += circleHeight;
    }
  }
}


int bgColorIdx = 0;
void loop() { 
  graphicalLayer->backgroundColor(colors[bgColorIdx]);
  delay(500);
  bgColorIdx += 1;
  if (bgColorIdx >= colorCount) {
    bgColorIdx = 0;
  }
}

 