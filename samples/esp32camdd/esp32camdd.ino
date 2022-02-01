#include <Arduino.h>


#define HAS_ESP32_CAM

#define BLUETOOTH



#ifdef BLUETOOTH


#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32CamDD", true, 115200));


#else

#include "wifidumbdisplay.h"
const char* ssid = <wifi SSID>;
const char* password = <wifi password>;
DumbDisplay dumbdisplay(new DDWiFiServerIO(ssid, password));

#endif


const char* imageName = "esp32camdd_test.jpg";

LcdDDLayer* flashLayer;
GraphicalDDLayer* imageLayer;



#ifdef HAS_ESP32_CAM
  // Used to disable brownout detection
 #include "soc/soc.h"
 #include "soc/rtc_cntl_reg.h"
#endif


bool initialiseCamera();
void captureAndSaveImage(bool useFlash);
void setupFlashPWM();


DDValueRecord<bool> flashOn(false, true);

//bool ready = false;
void setup() {


#ifdef HAS_ESP32_CAM
   //pinMode(indicatorLED, OUTPUT);
   //digitalWrite(indicatorLED, HIGH);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);     // Turn-off the 'brownout detector'
  setupFlashPWM();    // configure PWM for the illumination LED
#endif

  flashLayer = dumbdisplay.createLcdLayer(12, 1);
  flashLayer->enableFeedback("f");

  imageLayer = dumbdisplay.createGraphicalLayer(500, 400);
  imageLayer->backgroundColor("ivory");
  imageLayer->padding(10);
  imageLayer->border(20, "blue", "round");
  imageLayer->enableFeedback("f");


  dumbdisplay.configAutoPin(DD_AP_VERT);


#ifdef HAS_ESP32_CAM
  dumbdisplay.writeComment("Initializing camera ...");
  if (!initialiseCamera()) {
    dumbdisplay.writeComment("... failed to initialize camera!");
  } else {
    dumbdisplay.writeComment("... initialized camera!");
  }
#else
  dumbdisplay.writeComment("No camera!");
#endif

  imageLayer->drawImageFileFit("dumbdisplay.png");


}
void loop() {
  if (flashOn.record()) {
    if (flashOn) {
      flashLayer->writeCenteredLine("FLASH ON");
    } else {
      flashLayer->writeCenteredLine("FLASH OFF");
    }  
  }

  const DDFeedback* feedback = NULL;
  
  feedback = flashLayer->getFeedback();
  if (feedback != NULL) {
    flashOn = !flashOn;
  }

  feedback = imageLayer->getFeedback();
  if (feedback != NULL) {
#ifdef HAS_ESP32_CAM
    captureAndSaveImage(flashOn);
#else    
    dumbdisplay.capture(imageName, 640, 480);
#endif
    imageLayer->unloadImageFile(imageName);
    imageLayer->drawImageFileFit(imageName);
  }
}




#ifdef HAS_ESP32_CAM



const bool serialDebug = 1;                            // show debug info. on serial port (1=enabled, disable if using pins 1 and 3 as gpio)





#include "esp_camera.h"         // https://github.com/espressif/esp32-camera


// #include <base64.h>             // for encoding buffer to display image on page
// #include <WiFi.h>
// #include <WebServer.h>
// #include <HTTPClient.h>
// #include "driver/ledc.h"        // used to configure pwm on illumination led

// // spiffs used to store images if no sd card present
#include <SPIFFS.h>
#include <FS.h>                // gives file access on spiffs


 #include "SD_MMC.h"                         // sd card - see https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
//  #include <SPI.h>
//  #include <FS.h>                             // gives file access
 #define SD_CS 5                             // sd chip select pin = 5


 
 //#define useMCP23017 0                                  // if MCP23017 IO expander chip is being used (on pins 12 and 13)

 // Camera related
   bool flashRequired = 1;                              // If flash to be used when capturing image (1 = yes)
   framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_VGA;        // Image resolution:
                                                        //               default = "const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_VGA"
                                                        //               160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA),
                                                        //               320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA),
                                                        //               1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
   #define PIXFORMAT PIXFORMAT_JPEG                     // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
   int cameraImageExposure = 0;                         // Camera exposure (0 - 1200)   If gain and exposure both set to zero then auto adjust is enabled
   int cameraImageGain = 0;                             // Image gain (0 - 30)
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


// uint32_t lastStatus = millis();           // last time status light changed status (to flash all ok led)
//  bool sdcardPresent;                       // flag if an sd card is detected
//  int imageCounter;                         // image file name on sd card counter
//  String spiffsFilename = "/image.jpg";     // image name to use when storing in spiffs
//  String ImageResDetails = "Unknown";       // image resolution info








bool cameraImageSettings() {

  if (serialDebug) Serial.println("Applying camera settings");

   sensor_t *s = esp_camera_sensor_get();
   // something to try?:     if (s->id.PID == OV3660_PID)
   if (s == NULL) {
     if (serialDebug) Serial.println("Error: problem reading camera sensor settings");
     return 0;
   }

   // if both set to zero enable auto adjust
   if (cameraImageExposure == 0 && cameraImageGain == 0) {
     // enable auto adjust
       s->set_gain_ctrl(s, 1);                       // auto gain on
       s->set_exposure_ctrl(s, 1);                   // auto exposure on
       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
       s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness
   } else {
     // Apply manual settings
       s->set_gain_ctrl(s, 0);                       // auto gain off
       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
       s->set_exposure_ctrl(s, 0);                   // auto exposure off
       s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness
       s->set_agc_gain(s, cameraImageGain);          // set gain manually (0 - 30)
       s->set_aec_value(s, cameraImageExposure);     // set exposure manually  (0-1200)
   }

   return 1;
}  // cameraImageSettings



bool initialiseCamera() {

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
   config.frame_size = FRAME_SIZE_IMAGE;         // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA),
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

   //#if defined(CAMERA_MODEL_ESP_EYE)
   //  pinMode(13, INPUT_PULLUP);
   //  pinMode(14, INPUT_PULLUP);
   //#endif

   esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
   if (camerr != ESP_OK) {
     if (serialDebug) Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
   }

   cameraImageSettings();                        // apply custom camera settings

   return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}

// change illumination LED brightness
void brightLed(byte ledBrightness){
   //brightLEDbrightness = ledBrightness;    // store setting
   ledcWrite(ledChannel, ledBrightness);   // change LED brightness (0 - 255)
   //if (serialDebug) Serial.println("Brightness changed to " + String(ledBrightness) );
}



void setupFlashPWM() {
    ledcSetup(ledChannel, ledFreq, ledRresolution);
    ledcAttachPin(brightLED, ledChannel);
    brightLed(32/*brightLEDbrightness*/);
    brightLed(0);
}


// void flashLED(int reps) {
//  for(int x=0; x < reps; x++) {
//    digitalWrite(indicatorLED,LOW);
//    delay(1000);
//    digitalWrite(indicatorLED,HIGH);
//    delay(500);
//  }
// }


void captureAndSaveImage(bool useFlash) {

  if (useFlash) brightLed(255);   // change LED brightness (0 - 255)
  camera_fb_t *fb = esp_camera_fb_get();             // capture image frame from camera
  if (useFlash) brightLed(0);   // change LED brightness back to previous state
  if (fb == NULL) {
    if (serialDebug) Serial.println("Error: Camera capture failed");
     return;
  }

  if (serialDebug) {
     Serial.print("Captured image with ");
     Serial.print(fb->len);
     Serial.println(" bytes");
  }

  dumbdisplay.writeComment("Saving image (" + String(fb->len) + ") ...");
  dumbdisplay.saveImage(imageName, fb->buf, fb->len);
  dumbdisplay.writeComment("... save image");


  esp_camera_fb_return(fb);        // return frame so memory can be released
}



#endif