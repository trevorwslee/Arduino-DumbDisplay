#include <Arduino.h>
#include <esp_camera.h> 


// *** For board selection, uncomment one of following ; note that ESP32CAM is AI Thinker board and TCAMERA is v1.7
//#define FOR_ESP32CAM
//#define FOR_LILYGO_TCAMERAPLUS
//#define FOR_LILYGO_TSIMCAM
//#define FOR_LILYGO_TCAMERA

// *** Strongly suggest to use Bluetooth (if board supported), in such a case uncomment the following BLUETOOTH macro, which defines the name of the Bluetooth device (the board)
// *** Otherwise will assume WIFI connectivity; need WIFI_SSID and WIFI_PASSWORD macros
//#define BLUETOOTH     "CamMicBT"
//#define WIFI_SSID     "<wifi-ssid>"
//#define WIFI_PASSWORD "<wifi-password>" 
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
#else
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#endif

#define SUPPORT_FACE_DETECTION

#if defined(FOR_ESP32CAM)
  // *** If attached button, uncomment the following MIC_BUTTON_PIN macro 
  #define MIC_BUTTON_PIN       15
  #define FLASH_PIN             4
  // ***  For ESP_CAM, if INMP441 not attached, comments out the following I2S_WS macro 
  #define I2S_WS               12
  #define I2S_SD               13
  #define I2S_SCK              14
  #define I2S_SAMPLE_BIT_COUNT 32
  #define I2S_SAMPLE_RATE      8000
#elif defined(FOR_LILYGO_TCAMERAPLUS)
  #define TFT_BL_PIN           2
  #include <TFT_eSPI.h>
  TFT_eSPI tft = TFT_eSPI();
  // for the built-in mic of LiLyGO TCameraPlus
  #define I2S_WS               32
  #define I2S_SD               33
  #define I2S_SCK              14
  #define I2S_SAMPLE_BIT_COUNT 32
  #define I2S_SAMPLE_RATE      8000
#elif defined(FOR_LILYGO_TSIMCAM)
  // for the built-in mic of LiLyGO TSimCam
  #define I2S_WS               42
  #define I2S_SD                2
  #define I2S_SCK              41
  #define I2S_SAMPLE_BIT_COUNT 32
  #define I2S_SAMPLE_RATE      8000
#endif


// #define I2S_SAMPLE_RATE    8000
// #define SOUND_CHANNEL_COUNT  1
// #if defined(I2S_WS)
//   #include <driver/i2s.h>
//   #define I2S_PORT             I2S_NUM_0
// #endif

#if defined(SUPPORT_FACE_DETECTION)
  #include "human_face_detect_msr01.hpp"
  #include "human_face_detect_mnp01.hpp"
  HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.75F/*0.3F*/);  // 0.75F is adjusted for 96x96; original 0.3F is for 240x240
  HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);
#endif


// ====================
// ====================

void setup() {
  Serial.begin(115200);
#if defined(FLASH_PIN)
  pinMode(FLASH_PIN, OUTPUT);
#endif  
#if defined(MIC_BUTTON_PIN)
  pinMode(MIC_BUTTON_PIN, INPUT_PULLDOWN);
#endif  
#if defined(TFT_BL_PIN)
  tft.init(); 
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, LOW);
#endif
}


// ====================
// ====================

#if defined(I2S_WS)
  #include <driver/i2s.h>
  #define I2S_PORT             I2S_NUM_0
  //#define I2S_SAMPLE_RATE    8000
  #define SOUND_CHANNEL_COUNT  1

  const int I2S_DMA_BUF_COUNT = 8;
  const int I2S_DMA_BUF_LEN = 1024;

  // name of recorded WAV file; since only a single name; hence new one will always overwrite old one
  const char* SoundName = "recorded_sound";

  void i2s_install();
  void i2s_uninstall();
  void i2s_setpin();

  struct MicInfo {
    esp_err_t result;
    int samplesRead;
    int16_t *sampleStreamBuffer;
  };
  void readMicData(MicInfo &micInfo);
#endif

#define MIC_PLOTTER_FIXED_RATE
#define MAX_MIC_SAMPLE_AMPLITUDE 5000

const int UIAmplifyMultiplier = 10;
const int MaxAmplifyFactor = 20;
int micAmplifyFactor = 5;  // 0 means no software-based amplification

int micSoundChunkId = -1; // when started sending sound [chunk], the allocated "chunk id"
long micStreamingMillis = 0;
int micStreamingTotalSampleCount = 0;


// ====================
// ====================

const int imageLayerWidth = 640;
const int imageLayerHeight = 480;
const int defImageLayerTextSize = 30;
const char* imageName = "captured_image.jpg";

GraphicalDDLayer* imageLayer = NULL;  // initially assign it NULL
LcdDDLayer* flashLayer;
LcdDDLayer* faceLayer;
LcdDDLayer* sizeLayer;
LcdDDLayer* fpsLayer;
LcdDDLayer* micLayer;
PlotterDDLayer* plotterLayer;
LcdDDLayer* amplifyLblLayer;
JoystickDDLayer* amplifySlider;

bool initializeCamera(framesize_t frameSize, pixformat_t pixelFormat);
void deinitializeCamera();
bool captureImage();

framesize_t cameraSize;
pixformat_t cameraFormat;

bool cameraReady = false;
bool micReady = false;

bool flashOn = false;
bool micOn = false;
bool faceOn = false;
int frameSize = 2;
int frameRate = 0;  // 0 means AFAP (as fast as possible)

long lastCaptureImageMs = 0;

long fps_lastMs = -1;
long fps_lastLastMs = -1;

enum CameraState { CAM_OFF, CAM_TURNING_ON, CAM_RUNNING };
enum MicState { MIC_OFF, MIC_TURNING_ON, MIC_RUNNING };
CameraState cameraState = CAM_TURNING_ON;
MicState micState = MIC_OFF;

LcdDDLayer* createAndSetupButton(const char* bgColor = DD_COLOR_blue) {
  LcdDDLayer* buttonLayer = dumbdisplay.createLcdLayer(10, 1);
  buttonLayer->border(2, DD_COLOR_darkblue, "round");
  buttonLayer->padding(2);
  buttonLayer->backgroundColor(bgColor);
  buttonLayer->enableFeedback("f");
  return buttonLayer;
}

GraphicalDDLayer* createAndSetupImageLayer() {
  GraphicalDDLayer* imageLayer = dumbdisplay.createGraphicalLayer(imageLayerWidth, imageLayerHeight);
  imageLayer->setTextFont("DL::Roboto");
  imageLayer->setTextSize(defImageLayerTextSize);
  imageLayer->backgroundColor(DD_COLOR_ivory);
  imageLayer->padding(10);
  imageLayer->border(20, DD_COLOR_blue);
  return imageLayer;
}


PlotterDDLayer* createAndSetupPlotterLayer() {
#if defined(MIC_PLOTTER_FIXED_RATE)
  PlotterDDLayer* plotterLayer = dumbdisplay.createFixedRatePlotterLayer(1000, 300, 2);
#else
  PlotterDDLayer* plotterLayer = dumbdisplay.createPlotterLayer(1000, 300, 120);
#endif
  plotterLayer->backgroundColor(DD_COLOR_azure);
  plotterLayer->padding(10);
  plotterLayer->border(5, DD_COLOR_crimson);
  return plotterLayer;
} 

LcdDDLayer* createAndSetupAmplifyLblLayer() {
  LcdDDLayer* amplifyLblLayer = dumbdisplay.createLcdLayer(4, 1);
  amplifyLblLayer->pixelColor(DD_COLOR_red);
  amplifyLblLayer->noBackgroundColor();
  return amplifyLblLayer;
}

JoystickDDLayer* createAndSetupAmplifySlider() {
  JoystickDDLayer* amplifySlider = dumbdisplay.createJoystickLayer(UIAmplifyMultiplier * MaxAmplifyFactor, "hori");
  amplifySlider->padding(1);
  amplifySlider->border(1, DD_COLOR_navy);
  return amplifySlider;
}

void setupDumbdisplay() {
  flashLayer = createAndSetupButton();
  faceLayer = createAndSetupButton();
  sizeLayer = createAndSetupButton();
  fpsLayer = createAndSetupButton();
  micLayer = createAndSetupButton();
  imageLayer = createAndSetupImageLayer();
  amplifyLblLayer = createAndSetupAmplifyLblLayer();
  amplifySlider = createAndSetupAmplifySlider();
  plotterLayer = createAndSetupPlotterLayer();
  dumbdisplay.configAutoPin(
    DDAutoPinConfig('V')
      .beginGroup('H')
        .beginGroup('V')
          .addLayer(flashLayer)
          .addLayer(faceLayer)
          .addLayer(micLayer)
        .endGroup()
        .beginGroup('V')
          .addLayer(plotterLayer)
          .beginGroup('S')  // stacked, one on top of another
            .addLayer(amplifyLblLayer)  
            .addLayer(amplifySlider)
          .endGroup() 
        .endGroup()
      .endGroup()
      .addLayer(imageLayer)
      .beginGroup('H')
        .addLayer(sizeLayer)
        .addLayer(fpsLayer)
      .endGroup()
    .build()
  );

#if !defined(FLASH_PIN)// && !defined(TFT_BL_PIN)
  flashLayer->disabled(true);
#endif  
#if !defined(SUPPORT_FACE_DETECTION)
  faceLayer->disabled(true);
#endif
#if !defined(I2S_WS) || defined(MIC_BUTTON_PIN)
  micLayer->disabled(true);
#endif  
#if !defined(I2S_WS)
  amplifySlider->disabled(true);
#endif  

  imageLayer->drawImageFileFit("dumbdisplay.png");
}

void switchFlash(bool flashOn) {
#if defined(FLASH_PIN)
    digitalWrite(FLASH_PIN, flashOn ? HIGH : LOW);
#endif
}

bool simultaneousMicCamera = false;

void loop() {

  framesize_t oriCameraSize = cameraSize;
  pixformat_t oriCameraFormat = cameraFormat;
  //int oriState = state;
  MicState oriMicState = micState;

  // "passively" make connection with DumbDisplay app non-block
  DDConnectPassiveStatus connectStatus;
  dumbdisplay.connectPassive(&connectStatus);
  if (!connectStatus.connected) {
    // not connected ==> return immediately
    return;
  }

  if (connectStatus.reconnecting) {
    //  reconnecting (i.e. lost previous connection) ==> "master reset" DumbDisplay and reset other things, then return immediately
    if (micReady) {
      micState = MIC_OFF;
    } else {
      dumbdisplay.masterReset();
      imageLayer = NULL;
      if (cameraReady) {
        deinitializeCamera();
        cameraReady = false;
      }
  // #if defined(I2S_WS)
  //     if (micReady) {
  //       i2s_uninstall();
  //       micReady = false;
  //     }
  // #endif
      switchFlash(false);
      flashOn = false;
      micOn = false;
      Serial.println("***********");
      Serial.println("*** OFF ***");
      Serial.println("***********");
      return;
    }
  }

  boolean updateUI = false;
  if (imageLayer == NULL) {
    Serial.println("**********");
    Serial.println("*** ON ***");
    Serial.println("**********");
    // just connected ==> set things up like freshly initialized
    setupDumbdisplay();
    cameraState = CAM_TURNING_ON;
    micState = MIC_OFF;
    updateUI = true;
  }

  // framesize_t oriCameraSize = cameraSize;
  // pixformat_t oriCameraFormat = cameraFormat;
  // //int oriState = state;
  // MicState oriMicState = micState;

#if defined(MIC_BUTTON_PIN)
  bool pressed = digitalRead(MIC_BUTTON_PIN) == HIGH;
  if (pressed) {
    if (cameraState == CAM_RUNNING) {
      micOn = true;
      updateUI = true;
    }
  } else {
    if (micState == MIC_RUNNING) {
      micOn = false;
      updateUI = true;
    }
  }
#else  
  if (micLayer->getFeedback()) {
    micOn = !micOn;
    updateUI = true;
  }
#endif

  if (sizeLayer->getFeedback()) {
    frameSize = (frameSize + 1) % 4;
    updateUI = true;
    cameraState = CAM_TURNING_ON;
  }
  if (fpsLayer->getFeedback()) {
    if (frameRate == 0) {
      frameRate = 1;
    } else if (frameRate <= 4) {
      frameRate = 2 * frameRate;
    } else {
      frameRate = 0;
    }
    updateUI = true;
  }
  if (flashLayer->getFeedback()) {
    flashOn = !flashOn;
    updateUI = true;
  }
  if (faceLayer->getFeedback()) {
    faceOn = !faceOn;
    updateUI = true;
  }

  
  if (updateUI) {
    const char* sizeLabel;
    if (frameSize == 0) {
      sizeLabel = "QQVGA";
    } else  if (frameSize == 1) {
      sizeLabel = "QVGA";
    } else if (frameSize == 2) {
      sizeLabel = "VGA";
    } else if (frameSize == 3) {
      sizeLabel = "SVGA";
    } else {
      sizeLabel = "---";
    }
    sizeLayer->pixelColor(DD_COLOR_red);
    sizeLayer->writeCenteredLine(String("SIZE ") + sizeLabel);
    const char* fpsLabel;
    if (frameRate == 0) {
      fpsLabel = "DEF";
    } else if (frameRate == 1) {
      fpsLabel = "1";
    } else if (frameRate == 2) {
      fpsLabel = "2";
    } else if (frameRate == 4) {
      fpsLabel = "4";
    } else if (frameRate == 8) {
      fpsLabel = "8";
    } else {
      fpsLabel = "---";
    }
    fpsLayer->pixelColor(DD_COLOR_red);
    fpsLayer->writeCenteredLine(String("RATE ") + fpsLabel);
    if (flashOn) {
      flashLayer->pixelColor(DD_COLOR_red);
      flashLayer->writeCenteredLine("FLASH ON");
    } else {
      flashLayer->pixelColor(DD_COLOR_white);
      flashLayer->writeCenteredLine("FLASH OFF");
    }  
    switchFlash(flashOn);
    if (micOn) {
      micLayer->pixelColor(DD_COLOR_red);
      micLayer->writeCenteredLine("MIC ON");
      //state = STATE_TO_MIC;
      micState = MIC_TURNING_ON;
      if (!simultaneousMicCamera) {
        cameraState = CAM_OFF;
      }
    } else {
      micLayer->pixelColor(DD_COLOR_white);
      micLayer->writeCenteredLine("MIC OFF");
      //state = STATE_TO_CAMERA;
      micState = MIC_OFF;
      cameraState = CAM_TURNING_ON;
      if (faceOn) {
        faceLayer->pixelColor(DD_COLOR_red);
        faceLayer->writeCenteredLine("FACE ON");
        sizeLayer->disabled(true);
      } else {
        faceLayer->pixelColor(DD_COLOR_white);
        faceLayer->writeCenteredLine("FACE OFF");
        sizeLayer->disabled(false);
      }
    }  
  }

  const DDFeedback* fb = amplifySlider->getFeedback();
  if (fb != NULL) {
    micAmplifyFactor = fb->x / UIAmplifyMultiplier;
  }
  if (fb != NULL || updateUI) {
    amplifyLblLayer->writeLine(String(micAmplifyFactor), 0, "R");
  }

  if (micState == MIC_TURNING_ON/*state == STATE_TO_MIC*/) {
#if defined(TFT_BL_PIN)
    tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(3);
    tft.drawString("TALKING ...", 20, 40, 1);
    digitalWrite(TFT_BL_PIN, HIGH);
#endif
    if (cameraReady && !simultaneousMicCamera) {
      deinitializeCamera();
      cameraReady = false;
      imageLayer->drawImageFile(imageName, 0, 0, 0, 0, "f:;l:GS");
      imageLayer->setTextSize(72);
      imageLayer->drawStr(50, imageLayerHeight - 100, "TALK ...", "red");
      imageLayer->setTextSize(defImageLayerTextSize);
    }
  }

  if (cameraState == CAM_TURNING_ON && (simultaneousMicCamera || !micReady)) {
    plotterLayer->clear();
#if defined(TFT_BL_PIN)
    //tft.fillScreen(TFT_WHITE);
    digitalWrite(TFT_BL_PIN, flashOn ? HIGH : LOW);
#endif
    if (faceOn) {
      cameraSize = FRAMESIZE_96X96;
      cameraFormat = PIXFORMAT_RGB565;
    } else {
      if (frameSize == 0) {
        cameraSize = FRAMESIZE_QQVGA;
      } else if (frameSize == 1) {
        cameraSize = FRAMESIZE_QVGA;
      } else if (frameSize == 2) {
        cameraSize = FRAMESIZE_VGA;
      } else {
        cameraSize = FRAMESIZE_SVGA;
      }
      cameraFormat = PIXFORMAT_JPEG;
    }
    if (cameraReady) {
      if (oriCameraSize != cameraSize || oriCameraFormat != oriCameraFormat) {
        cameraReady = false;
      }
    }
    if (!cameraReady) {
      dumbdisplay.writeComment("Initializing camera ...");
      cameraReady = initializeCamera(cameraSize, cameraFormat); 
      if (cameraReady) {
        dumbdisplay.writeComment("... initialized camera!");
      } else {
        dumbdisplay.writeComment("... failed to initialize camera!");
      }
    }
    if (cameraReady) {
      imageLayer->clear();
      //state = STATE_CAMERA_RUNNING;
      cameraState = CAM_RUNNING;
#if defined(TFT_BL_PIN)
      if (faceOn) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(2);
        tft.drawString("Look at the camera", 10, 40, 1);
        tft.drawString("... and smile!", 10, 60, 1);
        digitalWrite(TFT_BL_PIN, HIGH);
      }
#endif
    }
  }

  if (micState == MIC_TURNING_ON && (simultaneousMicCamera || !cameraReady)) {
#if defined(I2S_WS)    
    if (!micReady) {
      dumbdisplay.writeComment("SETUP MIC ...");
      i2s_install();
      i2s_setpin();
      i2s_zero_dma_buffer(I2S_PORT);
      i2s_start(I2S_PORT);
      dumbdisplay.writeComment("... DONE SETUP MIC");
      micReady = true;
    }
    if (micReady) {
      //state = STATE_MIC_RUNNING;
      micState = MIC_RUNNING;
      fps_lastMs = -1;
      fps_lastLastMs = -1;
    }
#endif
  }

  if (cameraState == CAM_RUNNING && cameraReady) {
    boolean doCapture = true;
    if (frameRate > 0) {
      int captureImageGapMs = 1000 / frameRate;
      long diffMs = millis() - lastCaptureImageMs;
      if ((captureImageGapMs - diffMs) > 30) {
        doCapture = false;
      }
    }
    if (doCapture) {
      if (captureImage()) {
        imageLayer->drawImageFileFit(imageName);
        int xOff = (imageLayerWidth - imageLayerHeight) / 2;
        fps_lastLastMs = fps_lastMs;
        fps_lastMs = millis();
        if (fps_lastLastMs != -1) {
          long diffMs = fps_lastMs - fps_lastLastMs;
          double fps = 1000.0 / (double) diffMs;
          String str = "FPS:" + String(fps, 2);
          imageLayer->drawStr(xOff + 7, 5, str, DD_COLOR_darkred, "a50%yellow");
        }
      } else {
        dumbdisplay.writeComment("Failed to capture image!");
        delay(1000);
      }
      lastCaptureImageMs = millis();
    }
  }

#if defined(I2S_WS)    
  if ((micState == MIC_RUNNING || oriMicState == MIC_RUNNING) && micReady) {
    MicInfo micInfo;
    readMicData(micInfo);
    if (micSoundChunkId == -1) {
      // while started ... if no allocated "chunk id" (i.e. not yet started sending sound)
      // start streaming sound, and get the assigned "chunk id"
      micSoundChunkId = dumbdisplay.streamSound16(I2S_SAMPLE_RATE, SOUND_CHANNEL_COUNT);
      dumbdisplay.writeComment(String("STARTED mic streaming with chunk id [") + micSoundChunkId + "]");
      micStreamingMillis = millis();
      micStreamingTotalSampleCount = 0;
    }
    if (micInfo.result == ESP_OK) {
      if (micSoundChunkId != -1) {
        // send sound samples read
        bool isFinalChunk = micState != MIC_RUNNING;  // it is the final chunk if mic not keep running
        dumbdisplay.sendSoundChunk16(micSoundChunkId, micInfo.sampleStreamBuffer, micInfo.samplesRead, isFinalChunk);
        micStreamingTotalSampleCount += micInfo.samplesRead;
        if (isFinalChunk) {
          dumbdisplay.writeComment(String("DONE streaming with chunk id [") + micSoundChunkId + "]");
          long forMillis = millis() - micStreamingMillis;
          int totalSampleCount = micStreamingTotalSampleCount;
          dumbdisplay.writeComment(String(". total streamed samples: ") + totalSampleCount + " in " + String(forMillis / 1000.0) + "s");
          dumbdisplay.writeComment(String(". stream sample rate: ") + String(1000.0 * ((float) totalSampleCount / forMillis)));
          micSoundChunkId = -1;
          i2s_uninstall();
          micReady = false;
        }
      }
    }
  }
#endif    
}



// ====================
// ====================


#if defined(FOR_ESP32CAM)
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
#elif defined(FOR_LILYGO_TCAMERA)
  // for TCAMERA v1.7
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
#else
  #error board not supported
#endif

bool cameraImageSettings() {

  Serial.println("Applying camera settings");

  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    Serial.println("Error: problem reading camera sensor settings");
    return 0;
  }

  // enable auto adjust
  s->set_gain_ctrl(s, 1);                       // auto gain on
  s->set_exposure_ctrl(s, 1);                   // auto exposure on
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_brightness(s, 0);                      // (-2 to 2) - set brightness

   return 1;
}

bool initializeCamera(framesize_t frameSize, pixformat_t pixelFormat) {
  
  Serial.println("Initializing camera");

  esp_camera_deinit();     // disable camera
  delay(50);

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
  config.xclk_freq_hz = 20000000;               // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)`
  config.pixel_format = pixelFormat;            // Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
  config.frame_size = frameSize;                // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA),
                                                //              400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA),
                                                //              1600x1200 (UXGA)
  config.jpeg_quality = 15;                     // 0-63 lower number means higher quality
  config.fb_count = 1;                          // if more than one, i2s runs in continuous mode. Use only with JPEG

  // check the esp32cam board has a psram chip installed (extra memory used for storing captured images)
  //    Note: if not using "AI thinker esp32 cam" in the Arduino IDE, SPIFFS must be enabled
  if (!psramFound()) {
    Serial.println("ERROR: No PSRam found");
    //config.frame_size = FRAMESIZE_CIF;
    return false;
  }

  esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
  if (camerr != ESP_OK) {
    Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
  }

  cameraImageSettings();                        // apply custom camera settings

  return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}

void deinitializeCamera() {
  Serial.println("Deinitializing camera");
  esp_camera_deinit();     // disable camera
  delay(50);
}

int fd_x1 = -1;  // track the last detected one
int fd_y1 = -1;
int fd_x2 = -1;
int fd_y2 = -1;
int fd_le_x = -1;
int fd_le_y = -1;
int fd_re_x = -1;
int fd_re_y = -1;
int fd_n_x = -1;
int fd_n_y = -1;
int fd_ml_x = -1;
int fd_ml_y = -1;
int fd_mr_x = -1;
int fd_mr_y = -1;
long fd_ms = -1;

bool captureImage() {

  int xOff = (imageLayerWidth - imageLayerHeight) / 2;

  // fps_lastLastMs = fps_lastMs;
  // fps_lastMs = millis();
  // if (fps_lastLastMs != -1) {
  //   long diffMs = fps_lastMs - fps_lastLastMs;
  //   double fps = 1000.0 / (double) diffMs;
  //   String str = "FPS:" + String(fps, 2);
  //   imageLayer->drawStr(xOff + 7, 5, str, DD_COLOR_darkred, "a50%yellow");
  // }

#if defined(SUPPORT_FACE_DETECTION)
  if (cameraFormat == PIXFORMAT_RGB565) {
    if (fd_x1 != -1) {
      float scale = imageLayerHeight / 96;  // assume 96x96
      imageLayer->drawRect(xOff + scale * fd_x1, scale * fd_y1, scale * (fd_x2 - fd_x1), scale * (fd_y2 - fd_y1), DD_COLOR_green);
      if (true) {
        imageLayer->fillCircle(xOff + scale * fd_le_x, scale * fd_le_y, 4, DD_COLOR_red);  // left eye
        imageLayer->fillCircle(xOff + scale * fd_re_x, scale * fd_re_y, 4, DD_COLOR_red);  // right eye
        imageLayer->fillCircle(xOff + scale * fd_n_x, scale * fd_n_y, 6, DD_COLOR_green);  // nose
        imageLayer->fillRect(xOff + scale * fd_ml_x, scale * fd_ml_y, scale * (fd_mr_x - fd_ml_x), 4, DD_COLOR_blue);  // mouth
      }
    }
    if (fd_ms != -1) {
      String str = "FD:" + String(fd_ms) + "ms";
      imageLayer->drawStr(xOff + 7, imageLayerHeight - 40, str, DD_COLOR_darkblue, "a50%green");
    }
  }
#endif

  camera_fb_t *fb = esp_camera_fb_get();   // capture image frame from camera
  if (fb == NULL) {
     Serial.println("Error: Camera capture failed");
     return false;
  }


#if defined(SUPPORT_FACE_DETECTION)
  if (cameraFormat == PIXFORMAT_RGB565) {
    long startMs = millis();
    std::list<dl::detect::result_t> &candidates = detector.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
    std::list<dl::detect::result_t> &results = detector2.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}, candidates);
    long takenMs = millis() - startMs;
    fd_ms = takenMs;
    if (results.size()) {
      Serial.print("* FD:[]");
      int i = 0;
      for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++) {
        fd_le_x = prediction->keypoint[0];
        fd_le_y = prediction->keypoint[1];
        fd_re_x = prediction->keypoint[6];
        fd_re_y = prediction->keypoint[7];
        fd_n_x = prediction->keypoint[4];
        fd_n_y = prediction->keypoint[5];
        fd_ml_x = prediction->keypoint[2];
        fd_ml_y = prediction->keypoint[3];
        fd_mr_x = prediction->keypoint[8];
        fd_mr_y = prediction->keypoint[9];
        fd_x1 = prediction->box[0];
        fd_y1 = prediction->box[1];
        fd_x2 = prediction->box[2];
        fd_y2 = prediction->box[3];
        Serial.print(i);
        Serial.print(':');
        Serial.print(fd_x1);
        Serial.print(',');
        Serial.print(fd_y1);
        Serial.print('-');
        Serial.print(fd_x2);
        Serial.print(',');
        Serial.print(fd_y2);
      }
      Serial.println("]");
    } else {
      fd_x1 = -1;
    } 
  }
#endif

  //long startMs = millis();
  if (cameraFormat == PIXFORMAT_JPEG) {
    imageLayer->cacheImage(imageName, fb->buf, fb->len);
  } else if (cameraFormat == PIXFORMAT_RGB565) {
    imageLayer->cachePixelImage16(imageName, (const uint16_t*) fb->buf, fb->width, fb->height, "sbo");
  } else if (cameraFormat == PIXFORMAT_GRAYSCALE) {
    imageLayer->cachePixelImageGS(imageName, fb->buf, fb->width, fb->height);
  }
  // long endMs = millis();
  // long taken = endMs - startMs;
  // Serial.print(fb->len);
  // Serial.print(':');
  // Serial.print(taken);
  // Serial.println("ms");

  esp_camera_fb_return(fb);        // return frame so memory can be released

  return true;
}


// ====================
// ====================

#if defined(I2S_WS)

#if I2S_SAMPLE_BIT_COUNT == 32
  const int StreamBufferNumBytes = 512;
  const int StreamBufferLen = StreamBufferNumBytes / 4;
  int32_t StreamBuffer[StreamBufferLen];
  int16_t SampleStreamBuffer[StreamBufferLen];
#else
  const int StreamBufferNumBytes = 256;
  const int StreamBufferLen = StreamBufferNumBytes / 2;
  int16_t StreamBuffer[StreamBufferLen];
#endif


void i2s_install() {
  Serial.println("Installing I2S");
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BIT_COUNT),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = I2S_DMA_BUF_COUNT,
    .dma_buf_len = I2S_DMA_BUF_LEN,
    .use_apll = false
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}
void i2s_uninstall() {
  Serial.println("Uninstalling I2S");
  i2s_driver_uninstall(I2S_PORT);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,   
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
  i2s_set_pin(I2S_PORT, &pin_config);
}

// #if I2S_SAMPLE_BIT_COUNT == 32
//   int16_t SampleStreamBuffer[StreamBufferLen];
// #endif
void readMicData(MicInfo &micInfo) {
  // read I2S data and place in data buffer
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
  int samplesRead = 0;
#if I2S_SAMPLE_BIT_COUNT == 32
  int16_t *sampleStreamBuffer = SampleStreamBuffer;
#else
  int16_t *sampleStreamBuffer = StreamBuffer;
#endif
  if (result == ESP_OK) {
#if I2S_SAMPLE_BIT_COUNT == 32
      samplesRead = bytesRead / 4;  // 32 bit per sample
#else
      samplesRead = bytesRead / 2;  // 16 bit per sample
#endif    
    if (samplesRead > 0) {
      // find the samples mean ... and amplify the sound sample, by simply multiple it by some "amplify factor"
      float minVal = 0;
      float sumVal = 0;
      float maxVal = 0;
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = StreamBuffer[i];
#if I2S_SAMPLE_BIT_COUNT == 32
        val = val / 0x0000ffff;  // 32 bit to 16 bit
#endif
        if (micAmplifyFactor > 1) {
          val = micAmplifyFactor * val;
        }
        if (val > MAX_MIC_SAMPLE_AMPLITUDE) {
          val = MAX_MIC_SAMPLE_AMPLITUDE;
        } else if (val < -MAX_MIC_SAMPLE_AMPLITUDE) {
          val = -MAX_MIC_SAMPLE_AMPLITUDE;
        }
        sampleStreamBuffer[i] = val;
        sumVal += val;
        if (val < minVal) {
          minVal = val;
        } else if (val > maxVal) {
          maxVal = val;
        }
      }
#if defined (MIC_PLOTTER_FIXED_RATE)
      plotterLayer->set(minVal);
      plotterLayer->set(maxVal);
#else      
      float meanVal = (float) sumVal / (float) samplesRead;
      plotterLayer->set(meanVal);
#endif
    }
  }
  micInfo.result = result;
  micInfo.samplesRead = samplesRead;
  micInfo.sampleStreamBuffer = sampleStreamBuffer;
}


#endif