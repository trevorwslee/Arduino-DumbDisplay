
// comment out BLUETOOTH if don't have HC-05 / HC-06 connectivity
// . D3 => RX of HC-06; D2 => TX of HC-06; ; HC-06 should be configured to use baud rate of 115200
// if no HC-06 connectivity, will need to use DumbDisplayWifiBridge
#define BLUETOOTH

#ifdef BLUETOOTH
#include <ssdumbdisplay.h>
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200));
#else
#include <dumbdisplay.h>
DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif



#define SENSOR_OUT A1
#define S2         8
#define S3         9



PlotterDDLayer* plotterLayer;
LedGridDDLayer* rLayer;
LedGridDDLayer* gLayer;
LedGridDDLayer* bLayer;
SevenSegmentRowDDLayer* r7Layer;
SevenSegmentRowDDLayer* g7Layer;
SevenSegmentRowDDLayer* b7Layer;
LcdDDLayer* whiteLayer;
LcdDDLayer* blackLayer;
LcdDDLayer* colorLayer;


// get RGB intensity from the color sensor
int GetRGBIntensity(int s2, int s3) {
	// use S2 and S3 to direct the sensor which of R, G or B to read value from
  digitalWrite(S2, s2);
	digitalWrite(S3, s3);
  delay(100);
  // use pluseIn to read the pulse width of the color sensor output
	return pulseIn(SENSOR_OUT, LOW);
}

// get Red intensity
int GetRedIntensity() {
  return GetRGBIntensity(LOW, LOW);
}

// get Green intensity
int GetGreenIntensity() {
  return GetRGBIntensity(HIGH, HIGH);
}

// get Blue intensity
int GetBlueIntensity() {
  return GetRGBIntensity(LOW, HIGH);
}

// turn RGB intensity to correspond RBG color value (0-255)
int ToRGB(int rgbIntensity, int minRGBIntensity, int maxRGBIntensity) {
  int rgb = map(rgbIntensity, minRGBIntensity, maxRGBIntensity, 255, 0);
  if (rgb < 0) rgb = 0;
  if (rgb > 255) rgb = 255;
  return rgb;
}



bool calibrateMin = false;
bool calibrateMax = false;


void FeedbackHandler(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback) {
  if (pLayer == whiteLayer) {
    // which button clicked ==> calibrate MIN of the color intensities
    calibrateMin = true;
  } else if (pLayer == blackLayer) {
    // which button clicked ==> calibrate MAX of the color intensities
    calibrateMax = true;
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
  pinMode(SENSOR_OUT, INPUT);

  dumbdisplay.backgroundColor("beige"); 

  plotterLayer = dumbdisplay.createPlotterLayer(500, 180);
  plotterLayer->backgroundColor("azure");
  plotterLayer->padding(3);

  rLayer = dumbdisplay.createLedGridLayer();
  gLayer = dumbdisplay.createLedGridLayer();
  bLayer = dumbdisplay.createLedGridLayer();
  rLayer->backgroundColor("red");
  gLayer->backgroundColor("green");
  bLayer->backgroundColor("blue");

  r7Layer = dumbdisplay.create7SegmentRowLayer(2);
  g7Layer = dumbdisplay.create7SegmentRowLayer(2);
  b7Layer = dumbdisplay.create7SegmentRowLayer(2);
  r7Layer->border(10, "grey");
  r7Layer->backgroundColor("snow");
  g7Layer->border(10, "grey");
  g7Layer->backgroundColor("snow");
  b7Layer->border(10, "grey");
  b7Layer->backgroundColor("snow");

  whiteLayer = dumbdisplay.createLcdLayer(11, 3);
  blackLayer = dumbdisplay.createLcdLayer(11, 3);
  colorLayer = dumbdisplay.createLcdLayer(21, 7);
  whiteLayer->pixelColor("black");
  whiteLayer->backgroundColor("white");
  whiteLayer->border(2, "black", "round");
  whiteLayer->writeCenteredLine("cal white", 1);
  blackLayer->pixelColor("white");
  blackLayer->backgroundColor("black");
  blackLayer->writeCenteredLine("cal black", 1);
  blackLayer->border(2, "white", "round");
  colorLayer->border(2, "aqua");

  whiteLayer->setFeedbackHandler(FeedbackHandler, "fl");
  blackLayer->setFeedbackHandler(FeedbackHandler, "fl");

  dumbdisplay.configAutoPin(
    DD_AP_VERT_3(
      DD_AP_HORI_2(
        plotterLayer->getLayerId(),
        DD_AP_VERT_3(rLayer->getLayerId(), gLayer->getLayerId(), bLayer->getLayerId())
      ),
      DD_AP_STACK_2(
        colorLayer->getLayerId(),
        DD_AP_PADDING(50, 200, 50, 200,
          DD_AP_HORI_3(r7Layer->getLayerId(), g7Layer->getLayerId(), b7Layer->getLayerId()))
      ),
      DD_AP_HORI_2(whiteLayer->getLayerId(), blackLayer->getLayerId())
    )  
  );
}


int minRedIntensity = 0;
int maxRedIntensity = 5000;
int minGreenIntensity = 0;
int maxGreenIntensity = 5000;
int minBlueIntensity = 0;
int maxBlueIntensity = 5000;


void loop() {
  // read the red/green/blue intensities from color sensor
  int redIntensity = GetRedIntensity();
  int greenIntensity = GetGreenIntensity();
  int blueIntensity = GetBlueIntensity();

  if (calibrateMin) {
    minRedIntensity = redIntensity;
    minGreenIntensity = greenIntensity;
    minBlueIntensity = blueIntensity;
    calibrateMin = false;
  }
  if (calibrateMax) {
    maxRedIntensity = redIntensity;
    maxGreenIntensity = greenIntensity;
    maxBlueIntensity = blueIntensity;
    calibrateMax = false;
  }


  int red = ToRGB(redIntensity, minRedIntensity, maxRedIntensity);
  int green = ToRGB(greenIntensity, minGreenIntensity, maxGreenIntensity);
  int blue = ToRGB(blueIntensity, minBlueIntensity, maxBlueIntensity);

  Serial.print("R=");
	Serial.print(redIntensity);
	Serial.print("; G=");
	Serial.print(greenIntensity);
	Serial.print(" - B=");
	Serial.print(blueIntensity);
	Serial.print(" ==> ");

  Serial.print(red);
  Serial.print("-");
	Serial.print(green);
  Serial.print("-");
	Serial.print(blue);
  Serial.println();


  // plot the values to plotter layer
  plotterLayer->set("R", redIntensity, "G", greenIntensity, "B", blueIntensity);

  // change R/G/B leds with OFF color indicating the intensity of the R/G/B colors 
  rLayer->offColor(DD_RGB_COLOR(255, 255 - red, 255 - red));
  gLayer->offColor(DD_RGB_COLOR(255 - green, 255, 255 - green));
  bLayer->offColor(DD_RGB_COLOR(255 - blue, 255 - blue, 255));


  // set color layer background to the color detected
  colorLayer->backgroundColor(DD_RGB_COLOR(red, green, blue));


  // show the R/G/B values to the 3 7-seg layers
  r7Layer->showHexNumber(red);
  g7Layer->showHexNumber(green);
  b7Layer->showHexNumber(blue);
}
