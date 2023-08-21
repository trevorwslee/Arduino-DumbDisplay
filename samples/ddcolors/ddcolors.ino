
//#define DD_DISABLE_PARAM_ENCODING


#if defined(BLUETOOTH)

  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true));

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
  DD_COLOR_darkorange,
  DD_COLOR_darkviolet,
  DD_COLOR_violet,
  DD_COLOR_lightcyan,

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
  DD_COLOR_antiquewhite,
  DD_COLOR_bisque,
  DD_COLOR_cornsilk,
  DD_COLOR_crimson,
  DD_COLOR_maroon,
  DD_COLOR_olive,
  DD_COLOR_salmon,
  DD_COLOR_silver,
  DD_COLOR_skyblue,
  DD_COLOR_snow,
  DD_COLOR_tomato,
  DD_COLOR_turquoise,
  DD_COLOR_wheat,
  DD_COLOR_whitesmoke,
  DD_COLOR_tan,
  DD_COLOR_springgreen,
  DD_COLOR_steelblue,
  DD_COLOR_slategray,
  DD_COLOR_slateblue,
  DD_COLOR_seashell,
  DD_COLOR_royalblue,
  DD_COLOR_rosybrown,
  DD_COLOR_peru,
  DD_COLOR_chartreuse,
  DD_COLOR_chocolate,
  DD_COLOR_greenyellow,
  DD_COLOR_honeydew,
  DD_COLOR_hotpink,
  DD_COLOR_indianred,
  DD_COLOR_khaki,
  DD_COLOR_lavender
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
#if defined(DD_DISABLE_PARAM_ENCODING)
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

 