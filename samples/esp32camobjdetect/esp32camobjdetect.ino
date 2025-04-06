// ***** it is strongly suggested to use Bluetooth for connectivity *****
// to use Bluetooth, #define Bluetooth name like the following line
// #define BLUETOOTH "ESP32CamBT"
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
#else
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#endif


#include "esp_camera.h" 



const framesize_t frameSize = FRAMESIZE_VGA;
const int imageLayerWidth = 640;
const int imageLayerHeight = 480;
const char* imageName = "esp32camobjdetect.jpg";


GraphicalDDLayer* imageLayer;
GraphicalDDLayer* objectLayer;
ObjectDetectDemoServiceDDTunnel* objectTunnel;
bool cameraReady = false;


bool initialiseCamera(framesize_t frameSize);
bool captureAndSaveImage(bool useFlash, bool cacheOnly);


void setup() {
  Serial.begin(115200);

  // create the top layer for showing detected object rectangles
  objectLayer = dumbdisplay.createGraphicalLayer(imageLayerWidth, imageLayerHeight);
  objectLayer->border(10, DD_COLOR_blue);
  objectLayer->padding(5);
  objectLayer->noBackgroundColor();
  objectLayer->penSize(2);

  // create the bottom layer for showing the ESP32 CAM capatured image
  imageLayer = dumbdisplay.createGraphicalLayer(imageLayerWidth, imageLayerHeight);
  imageLayer->border(10, DD_COLOR_blue);
  imageLayer->padding(5);
  imageLayer->backgroundColor(DD_COLOR_azure);

  // create a tunnel for object detection demo via TensorFlow Lite running on phone side
  objectTunnel = dumbdisplay.createObjectDetectDemoServiceTunnel();
 
  imageLayer->drawImageFileFit("dumbdisplay.png");

  cameraReady = initialiseCamera(frameSize); 
  if (cameraReady) {
    dumbdisplay.writeComment("... initialized camera!");
  } else {
    dumbdisplay.writeComment("... failed to initialize camera!");
  }
}


bool detecting = false;

void loop() {
  if (cameraReady) {
    if (captureAndSaveImage(false, true)) {
      imageLayer->drawImageFileFit(imageName);
      if (objectTunnel->eof()) {
        objectTunnel->reconnectForObjectDetectFrom(imageLayer, imageName);
        detecting = true;
#if defined (DEBUG_WIFI)
        Serial.println("*** reconnected for OD");
#endif        
      } else {
        DDObjectDetectDemoResult objectDetectResult;
        if (objectTunnel->readObjectDetectResult(objectDetectResult)) {
#if defined (DEBUG_WIFI)
          Serial.println("*** OD read result");
#endif        
          if (detecting) {
            objectLayer->clear();
          }
          int x = objectDetectResult.left;
          int y = objectDetectResult.top;
          int w = objectDetectResult.right - objectDetectResult.left;
          int h = objectDetectResult.bottom - objectDetectResult.top;
          objectLayer->drawRect(x, y, w, h, DD_COLOR_green);
          objectLayer->drawStr(x, y, objectDetectResult.label, DD_COLOR_yellow, "a70%darkgreen", 32);
          detecting = false;
        }
      }
    }
  }
}





const bool serialDebug = 1;                          // show debug info. on serial port (1=enabled, disable if using pins 1 and 3 as gpio)


#define PIXFORMAT PIXFORMAT_JPEG                     // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
int cameraImageBrightness = 0;                       // Image brightness (-2 to +2)

const int brightLED = 4;                             // onboard Illumination/flash LED pin (4)
const int ledFreq = 5000;                            // PWM settings
const int ledChannel = 15;                           // camera uses timer1
const int ledRresolution = 8;                        // resolution (8 = from 0 to 255)



#if defined(FOR_LILYGO_TCAMERA)
  // for TCAMERA v17
  #define PWDN_GPIO_NUM     26
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM     32
  #define SIOD_GPIO_NUM     13      // i2c sda
  #define SIOC_GPIO_NUM     12      // i2c scl
  #define Y9_GPIO_NUM       39
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       23
  #define Y6_GPIO_NUM       18
  #define Y5_GPIO_NUM       15
  #define Y4_GPIO_NUM        4
  #define Y3_GPIO_NUM       14
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    27      // vsync_pin
  #define HREF_GPIO_NUM     25      // href_pin
  #define PCLK_GPIO_NUM     19      // pixel_clock_pin
  #define VFLIP
#elif defined(FOR_LILYGO_TCAMERAPLUS)
  // for T-CAMERA PLUS
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM     4
  #define SIOD_GPIO_NUM     18      // i2c sda
  #define SIOC_GPIO_NUM     23      // i2c scl
  #define Y9_GPIO_NUM       36
  #define Y8_GPIO_NUM       37
  #define Y7_GPIO_NUM       38
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM       35
  #define Y4_GPIO_NUM       26
  #define Y3_GPIO_NUM       13
  #define Y2_GPIO_NUM       34
  #define VSYNC_GPIO_NUM    5       // vsync_pin
  #define HREF_GPIO_NUM     27      // href_pin
  #define PCLK_GPIO_NUM     25      // pixel_clock_pin
#elif defined(FOR_LILYGO_TSIMCAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM     14
  #define SIOD_GPIO_NUM      4      // i2c sda
  #define SIOC_GPIO_NUM      5      // i2c scl
  #define Y9_GPIO_NUM       15
  #define Y8_GPIO_NUM       16
  #define Y7_GPIO_NUM       17
  #define Y6_GPIO_NUM       12
  #define Y5_GPIO_NUM       10
  #define Y4_GPIO_NUM        8
  #define Y3_GPIO_NUM        9
  #define Y2_GPIO_NUM       11
  #define VSYNC_GPIO_NUM     6      // vsync_pin
  #define HREF_GPIO_NUM      7      // href_pin
  #define PCLK_GPIO_NUM     13      // pixel_clock_pin
#elif defined(FOR_VCC_S3EYE) ||  defined(FOR_VCC_S3EYE_2) ||  defined(FOR_ESP_SPARKBOT)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM     15
  #define SIOD_GPIO_NUM      4      // i2c sda
  #define SIOC_GPIO_NUM      5      // i2c scl
  #define Y9_GPIO_NUM       16
  #define Y8_GPIO_NUM       17
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       12
  #define Y5_GPIO_NUM       10
  #define Y4_GPIO_NUM        8
  #define Y3_GPIO_NUM        9
  #define Y2_GPIO_NUM       11
  #define VSYNC_GPIO_NUM     6      // vsync_pin
  #define HREF_GPIO_NUM      7      // href_pin
  #define PCLK_GPIO_NUM     13      // pixel_clock_pin
  #define VFLIP
#elif defined(FOR_XIAO_S3SENSE)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM     10
  #define SIOD_GPIO_NUM     40      // i2c sda
  #define SIOC_GPIO_NUM     39      // i2c scl
  #define Y9_GPIO_NUM       48
  #define Y8_GPIO_NUM       11
  #define Y7_GPIO_NUM       12
  #define Y6_GPIO_NUM       14
  #define Y5_GPIO_NUM       16
  #define Y4_GPIO_NUM       18
  #define Y3_GPIO_NUM       17
  #define Y2_GPIO_NUM       15
  #define VSYNC_GPIO_NUM    38      // vsync_pin
  #define HREF_GPIO_NUM     47      // href_pin
  #define PCLK_GPIO_NUM     13      // pixel_clock_pin
  #define VFLIP
#elif defined(FOR_ESP32CAM)
  // for CAMERA_MODEL_AI_THINKER
  #define PWDN_GPIO_NUM     32      // power to camera (on/off)
  #define RESET_GPIO_NUM    -1      // -1 = not used
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26      // i2c sda
  #define SIOC_GPIO_NUM     27      // i2c scl
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25      // vsync_pin
  #define HREF_GPIO_NUM     23      // href_pin
  #define PCLK_GPIO_NUM     22      // pixel_clock_pin
#else
  #error board not supported
#endif



bool cameraImageSettings() {

  if (serialDebug) Serial.println("Applying camera settings");

  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    if (serialDebug) Serial.println("Error: problem reading camera sensor settings");
    return 0;
  }

  // enable auto adjust
  s->set_gain_ctrl(s, 1);                       // auto gain on
  s->set_exposure_ctrl(s, 1);                   // auto exposure on
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness
#ifdef VFLIP  
  s->set_vflip(s, true);
#endif

  return 1;
}  // cameraImageSettings



void setupFlashPWM();

bool initialiseCamera(framesize_t frameSize) {
  esp_camera_deinit();     // disable camera
  delay(50);
  setupFlashPWM();         // configure PWM for the illumination LED

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;               // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
  config.pixel_format = PIXFORMAT;              // Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
  config.frame_size = frameSize;                // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA),
                                                //              400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA),
                                                //              1600x1200 (UXGA)
  config.jpeg_quality = 15;                     // 0-63 lower number means higher quality
  config.fb_count = 1;                          // if more than one, i2s runs in continuous mode. Use only with JPEG

  // check the esp32cam board has a psram chip installed (extra memory used for storing captured images)
  //    Note: if not using "AI thinker esp32 cam" in the Arduino IDE, SPIFFS must be enabled
  if (!psramFound()) {
    if (serialDebug) Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
    config.frame_size = FRAMESIZE_CIF;
  }

  esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
  if (camerr != ESP_OK) {
    if (serialDebug) Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
  }

  cameraImageSettings();                        // apply custom camera settings

  return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}


// change illumination LED brightness
void brightLed(byte ledBrightness){
   ledcWrite(ledChannel, ledBrightness);   // change LED brightness (0 - 255)
}

void setupFlashPWM() {
    ledcSetup(ledChannel, ledFreq, ledRresolution);
    ledcAttachPin(brightLED, ledChannel);
    brightLed(32);
    brightLed(0);
}


bool captureAndSaveImage(bool useFlash, bool cacheOnly) {

  if (useFlash) brightLed(255);            // change LED brightness (0 - 255)
  camera_fb_t *fb = esp_camera_fb_get();   // capture image frame from camera
  if (useFlash) brightLed(0);              // change LED brightness back to previous state
  if (fb == NULL) {
    if (serialDebug) Serial.println("Error: Camera capture failed");
     return false;
  }

  if (cacheOnly) {
    imageLayer->cacheImage(imageName, fb->buf, fb->len);
  } else {
    dumbdisplay.writeComment("Saving image (" + String(fb->len) + ") ...");
    dumbdisplay.saveImage(imageName, fb->buf, fb->len);
    dumbdisplay.writeComment("... saved image");
  }

  esp_camera_fb_return(fb);        // return frame so memory can be released

  return true;
}

