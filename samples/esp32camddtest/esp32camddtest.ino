#include <Arduino.h>
#include "esp_camera.h" 


#include "esp_camera.h"         // https://github.com/espressif/esp32-camera



#define BLUETOOTH


#ifdef BLUETOOTH
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32Cam", true, 115200));
#else
#include "wifidumbdisplay.h"
const char* ssid = <wifi SSID>;
const char* password = <wifi password>;
DumbDisplay dumbdisplay(new DDWiFiServerIO(ssid, password));
#endif





const int imageLayerWidth = 1024;
const int imageLayerHeight = 768;
const char* imageName = "esp32camdd_test.jpg";


LcdDDLayer* flashLayer;
LcdDDLayer* resolutionLayer;
LcdDDLayer* captureModeLayer;
GraphicalDDLayer* imageLayer;


LcdDDLayer* createAndSetupButton(DumbDisplay& dumbdisplay, const char* fontColor = "blue") {
  LcdDDLayer* buttonLayer = dumbdisplay.createLcdLayer(10, 1);
  buttonLayer->pixelColor(fontColor);
  buttonLayer->border(1, "darkgray", "round");
  buttonLayer->enableFeedback("f");
  return buttonLayer;
}

GraphicalDDLayer * createAndSetupImageLayer(DumbDisplay& dumbdisplay) {
  GraphicalDDLayer* imgLayer = dumbdisplay.createGraphicalLayer(imageLayerWidth, imageLayerHeight);
  imgLayer->backgroundColor("ivory");
  imgLayer->padding(10);
  imgLayer->border(20, "blue");
  imgLayer->enableFeedback("f");
  return imgLayer;
}


bool initialiseCamera(framesize_t frameSize);
bool captureAndSaveImage(bool useFlash, bool cacheOnly);


void setup() {

  flashLayer = createAndSetupButton(dumbdisplay);
  resolutionLayer = createAndSetupButton(dumbdisplay);
  captureModeLayer = createAndSetupButton(dumbdisplay, "red");

  imageLayer = createAndSetupImageLayer(dumbdisplay);

  dumbdisplay.configAutoPin(DD_AP_VERT_2(
    DD_AP_HORI_3(flashLayer->getLayerId(), resolutionLayer->getLayerId(), captureModeLayer->getLayerId()),
    imageLayer->getLayerId()
  ));

  imageLayer->drawImageFileFit("dumbdisplay.png");
}


bool cameraReady = false;
DDValueRecord<bool> flashOn(false, true);
DDValueRecord<byte> imageSize(1, 0);
DDValueRecord<bool> streaming(false, true);


void loop() {
  if (flashOn.record()) {
    if (flashOn) {
      flashLayer->writeCenteredLine("FLASH ON");
    } else {
      flashLayer->writeCenteredLine("FLASH OFF");
    }  
  }
  if (streaming.record()) {
    if (streaming) {
      captureModeLayer->writeCenteredLine("STREAM");
    } else {
      captureModeLayer->writeCenteredLine("STILL");
    }
  }
  if (imageSize.record()) {
    framesize_t frameSize;
    switch (imageSize) {
      case 1:
        frameSize = FRAMESIZE_VGA;
        resolutionLayer->writeCenteredLine("640x480");
        break;
      case 2:
        frameSize = FRAMESIZE_SVGA;
        resolutionLayer->writeCenteredLine("800x600");
        break;
      case 3: 
        frameSize = FRAMESIZE_XGA;
        resolutionLayer->writeCenteredLine("1024x768");
        break;
      case 4:  
        frameSize = FRAMESIZE_HD;
        resolutionLayer->writeCenteredLine("1280x720");
        break;
      default:
        frameSize = FRAMESIZE_QVGA;
        resolutionLayer->writeCenteredLine("320x240");
    }
    dumbdisplay.writeComment("Initializing camera ...");
    cameraReady = initialiseCamera(frameSize); 
    if (cameraReady) {
      dumbdisplay.writeComment("... initialized camera!");
    } else {
      dumbdisplay.writeComment("... failed to initialize camera!");
    }
  }

  if (flashLayer->getFeedback()) {
    flashOn = !flashOn;
  }

  if (resolutionLayer->getFeedback()) {
    if (imageSize < 4) {
      imageSize = imageSize + 1;
    } else {
      imageSize = 1;
    }
  }

  if (captureModeLayer->getFeedback()) {
    streaming = !streaming;
  }

  bool capture = imageLayer->getFeedback() != NULL || (cameraReady && streaming);
  if (capture) {
    if (cameraReady) {
      if (captureAndSaveImage(flashOn, streaming)) {
        if (!streaming) {
          imageLayer->unloadImageFile(imageName);
          imageLayer->clear();
        }      
        imageLayer->drawImageFileFit(imageName);
      } else {
        dumbdisplay.writeComment("Failed to capture image!");
      }
    } else {
      dumbdisplay.writeComment("Camera not ready!");
    }
  }
}







const bool serialDebug = 1;                            // show debug info. on serial port (1=enabled, disable if using pins 1 and 3 as gpio)


   #define PIXFORMAT PIXFORMAT_JPEG                     // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
   //int cameraImageExposure = 0;                         // Camera exposure (0 - 1200)   If gain and exposure both set to zero then auto adjust is enabled
   //int cameraImageGain = 0;                             // Image gain (0 - 30)
   int cameraImageBrightness = 0;                       // Image brightness (-2 to +2)

 //const int TimeBetweenStatus = 600;                     // speed of flashing system running ok status light (milliseconds)

 //const int indicatorLED = 33;                           // onboard small LED pin (33)

 // Bright LED (Flash)
   const int brightLED = 4;                             // onboard Illumination/flash LED pin (4)
   //int brightLEDbrightness = 0;                         // initial brightness (0 - 255)
   const int ledFreq = 5000;                            // PWM settings
   const int ledChannel = 15;                           // camera uses timer1
   const int ledRresolution = 8;                        // resolution (8 = from 0 to 255)

 //const int iopinA = 13;                                 // general io pin 13
 //const int iopinB = 12;                                 // general io pin 12 (must not be high at boot)

 //const int serialSpeed = 115200;                        // Serial data speed to use

 // NTP - Internet time
  //  const char* ntpServer = "pool.ntp.org";
  //  const char* TZ_INFO    = "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
  //  long unsigned lastNTPtime;
  //  tm timeinfo;
  //  time_t now;

// camera settings (for the standard - OV2640 - CAMERA_MODEL_AI_THINKER)
// see: https://randomnerdtutorials.com/esp32-cam-camera-pin-gpios/
// set camera resolution etc. in 'initialiseCamera()' and 'cameraImageSettings()'
 #define CAMERA_MODEL_AI_THINKER
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





bool cameraImageSettings() {

  if (serialDebug) Serial.println("Applying camera settings");

  sensor_t *s = esp_camera_sensor_get();
  // something to try?:     if (s->id.PID == OV3660_PID)
  if (s == NULL) {
    if (serialDebug) Serial.println("Error: problem reading camera sensor settings");
    return 0;
  }

  // enable auto adjust
  s->set_gain_ctrl(s, 1);                       // auto gain on
  s->set_exposure_ctrl(s, 1);                   // auto exposure on
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness

   return 1;
}  // cameraImageSettings



void setupFlashPWM();

bool initialiseCamera(framesize_t frameSize) {
  if (true) {
    esp_camera_deinit();     // disable camera
    delay(50);
    setupFlashPWM();    // configure PWM for the illumination LED
  }

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

  if (useFlash) brightLed(255);   // change LED brightness (0 - 255)
  camera_fb_t *fb = esp_camera_fb_get();             // capture image frame from camera
  if (useFlash) brightLed(0);   // change LED brightness back to previous state
  if (fb == NULL) {
    if (serialDebug) Serial.println("Error: Camera capture failed");
     return false;
  }

  // if (serialDebug) {
  //    Serial.print("Captured image with ");
  //    Serial.print(fb->len);
  //    Serial.println(" bytes");
  // }

  if (cacheOnly) {
    imageLayer->cacheImage(imageName, fb->buf, fb->len);
  } else {
    dumbdisplay.writeComment("Saving image (" + String(fb->len) + ") ...");
    dumbdisplay.saveImage(imageName, fb->buf, fb->len);
    dumbdisplay.writeComment("... save image");
  }


  esp_camera_fb_return(fb);        // return frame so memory can be released

  return true;
}



