// if want Bluetooth, uncomment the following line
// #define BLUETOOTH "ESP32CamBT"
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
#else
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#endif



#include "esp_camera.h" 
#include <driver/i2s.h>
 


// #define CAM_FORMAT           PIXFORMAT_JPEG                     // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
// #define CAM_SIZE             FRAMESIZE_VGA
// #define CAM_FORMAT           PIXFORMAT_RGB565                   // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
// #define CAM_SIZE             FRAMESIZE_96X96
#define CAM_FORMAT           PIXFORMAT_GRAYSCALE                   // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
#define CAM_SIZE             FRAMESIZE_96X96



const int imageLayerWidth = 1024;
const int imageLayerHeight = 768;
const char* imageName = "esp32camwmic.jpg";


LcdDDLayer* flashLayer;
LcdDDLayer* cameraLayer;
LcdDDLayer* micLayer;
GraphicalDDLayer* imageLayer;


LcdDDLayer* createAndSetupButton(DumbDisplay& dumbdisplay, const char* bgColor = DD_COLOR_blue) {
  LcdDDLayer* buttonLayer = dumbdisplay.createLcdLayer(11, 1);
  buttonLayer->border(2, DD_COLOR_gray, "round");
  buttonLayer->padding(1);
  buttonLayer->backgroundColor(bgColor);
  //buttonLayer->pixelColor(DD_COLOR_white);
  buttonLayer->enableFeedback("f");
  return buttonLayer;
}

GraphicalDDLayer * createAndSetupImageLayer(DumbDisplay& dumbdisplay) {
  GraphicalDDLayer* imgLayer = dumbdisplay.createGraphicalLayer(imageLayerWidth, imageLayerHeight);
  imgLayer->backgroundColor(DD_COLOR_ivory);
  imgLayer->padding(10);
  imgLayer->border(20, DD_COLOR_blue);
  //imgLayer->enableFeedback("f");
  return imgLayer;
}


bool initialiseCamera(framesize_t frameSize);
bool captureImage(bool useFlash);


#define I2S_PORT  I2S_NUM_0
void i2s_install();
void i2s_setpin();


struct MicInfo {
  int sampleRate;
  int numChannels;
  esp_err_t result;
  int samplesRead;
  int16_t *sampleStreamBuffer;
};

void readMicData(MicInfo &micInfo);

const int MaxAmplifyFactor = 20;
const int DefAmplifyFactor = 10;

bool micStarted = false;
int micAmplifyFactor = DefAmplifyFactor;//10;
int micSoundChunkId = -1; // when started sending sound [chunk], the allocated "chunk id"
long micStreamingMillis = 0;
int micStreamingTotalSampleCount = 0;



void setup() {
  Serial.begin(115200);


  Serial.println("SETUP MIC ...");

  // set up I2S
  i2s_install();
  i2s_setpin();
  i2s_zero_dma_buffer(I2S_PORT);
  i2s_start(I2S_PORT);

  Serial.println("... DONE SETUP MIC");


  flashLayer = createAndSetupButton(dumbdisplay);
  cameraLayer = createAndSetupButton(dumbdisplay);
  micLayer = createAndSetupButton(dumbdisplay);

  imageLayer = createAndSetupImageLayer(dumbdisplay);

  dumbdisplay.configAutoPin(DD_AP_VERT_2(
    DD_AP_HORI_3(flashLayer->getLayerId(), cameraLayer->getLayerId(), micLayer->getLayerId()),
    imageLayer->getLayerId()
  ));

  // set when DD idle handler ... here is a lambda expression
  dumbdisplay.setIdleCallback([](long idleForMillis, DDIdleConnectionState connectionState) {
    if (connectionState == DDIdleConnectionState::IDLE_RECONNECTING) {
      micStarted = false;  // if idle, e.g. disconnected, stop whatever
    }
  });


  imageLayer->drawImageFileFit("dumbdisplay.png");
}


bool cameraReady = false;
DDValueRecord<bool> flashOn(false, true);
DDValueRecord<bool> cameraOn(false, true);
DDValueRecord<bool> micOn(false, true);


void loop() {
  if (flashOn.record()) {
    if (flashOn) {
      flashLayer->pixelColor(DD_COLOR_red);
      flashLayer->writeCenteredLine("FLASH ON");
    } else {
      flashLayer->pixelColor(DD_COLOR_white);
      flashLayer->writeCenteredLine("FLASH OFF");
    }  
  }

  bool cameraWasOn = cameraOn;
  if (cameraOn.record()) {
    if (cameraOn) {
      cameraLayer->pixelColor(DD_COLOR_red);
      cameraLayer->writeCenteredLine("CAM ON");
      //micStarted = true;
    } else {
      cameraLayer->pixelColor(DD_COLOR_white);
      cameraLayer->writeCenteredLine("CAM OFF");
      //micStarted = false;
    }  
  }


  bool micRunning = micStarted;
  if (micOn.record()) {
    if (micOn) {
      micLayer->pixelColor(DD_COLOR_red);
      micLayer->writeCenteredLine("MIC ON");
      micStarted = true;
    } else {
      micLayer->pixelColor(DD_COLOR_white);
      micLayer->writeCenteredLine("MIC OFF");
      micStarted = false;
    }  
  }

  if (!cameraReady) {
    //imageLayer->clear();
    dumbdisplay.writeComment("Initializing camera ...");
    cameraReady = initialiseCamera(CAM_SIZE); 
    if (cameraReady) {
      dumbdisplay.writeComment("... initialized camera!");
    } else {
      dumbdisplay.writeComment("... failed to initialize camera!");
    }
  }

  if (flashLayer->getFeedback()) {
    flashOn = !flashOn;
  }
  if (cameraLayer->getFeedback()) {
    cameraOn = !cameraOn;
  }
  if (micLayer->getFeedback()) {
    micOn = !micOn;
  }

  if (cameraReady) {
    if (cameraOn) {
      if (captureImage(flashOn)) {
        imageLayer->drawImageFileFit(imageName);
      } else {
        dumbdisplay.writeComment("Failed to capture image!");
        delay(1000);
      }
    } else if (cameraWasOn) {
      //imageLayer->clear();
      imageLayer->drawImageFileFit("dumbdisplay.png");
    }
  }

  if (micRunning) {
    MicInfo micInfo;
    readMicData(micInfo);
    if (micSoundChunkId == -1) {
      // while started ... if no allocated "chunk id" (i.e. not yet started sending sound)
      // start streaming sound, and get the assigned "chunk id"
      micSoundChunkId = dumbdisplay.streamSound16(micInfo.sampleRate, micInfo.numChannels);
      dumbdisplay.writeComment(String("STARTED mic streaming with chunk id [") + micSoundChunkId + "]");
      micStreamingMillis = millis();
      micStreamingTotalSampleCount = 0;
    }
    if (micInfo.result == ESP_OK) {
      if (micSoundChunkId != -1) {
        // send sound samples read
        bool isFinalChunk = !micStarted;  // it is the final chink if justed turned to stop
        dumbdisplay.sendSoundChunk16(micSoundChunkId, micInfo.sampleStreamBuffer, micInfo.samplesRead, isFinalChunk);
        micStreamingTotalSampleCount += micInfo.samplesRead;
        if (isFinalChunk) {
          dumbdisplay.writeComment(String("DONE streaming with chunk id [") + micSoundChunkId + "]");
          long forMillis = millis() - micStreamingMillis;
          int totalSampleCount = micStreamingTotalSampleCount;
          dumbdisplay.writeComment(String(". total streamed samples: ") + totalSampleCount + " in " + String(forMillis / 1000.0) + "s");
          dumbdisplay.writeComment(String(". stream sample rate: ") + String(1000.0 * ((float) totalSampleCount / forMillis)));
          micSoundChunkId = -1;
        }
      }
    }
  }
}




const bool serialDebug = 1;                            // show debug info. on serial port (1=enabled, disable if using pins 1 and 3 as gpio)


//#define PIXFORMAT PIXFORMAT_JPEG                     // image format, Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
int cameraImageBrightness = 0;                       // Image brightness (-2 to +2)

const int brightLED = 4;                             // onboard Illumination/flash LED pin (4)
const int ledFreq = 5000;                            // PWM settings
const int ledChannel = 15;                           // camera uses timer1
const int ledRresolution = 8;                        // resolution (8 = from 0 to 255)


#if defined(FOR_LILYGO_TCAMERA)
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
#else
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

   return 1;
}  // cameraImageSettings



void setupFlashPWM();

bool initialiseCamera(framesize_t frameSize) {
  esp_camera_deinit();     // disable camera
  delay(50);
  setupFlashPWM();    // configure PWM for the illumination LED

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
  config.pixel_format = CAM_FORMAT;              // Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
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


bool captureImage(bool useFlash) {

  //if (useFlash) brightLed(255);            // change LED brightness (0 - 255)
  camera_fb_t *fb = esp_camera_fb_get();   // capture image frame from camera
  //if (useFlash) brightLed(0);              // change LED brightness back to previous state
  if (fb == NULL) {
    if (serialDebug) Serial.println("Error: Camera capture failed");
     return false;
  }

  if (CAM_FORMAT == PIXFORMAT_JPEG) {
    imageLayer->cacheImage(imageName, fb->buf, fb->len);
  } else if (CAM_FORMAT == PIXFORMAT_RGB565) {
    imageLayer->cachePixelImage16(imageName, (const uint16_t*) fb->buf, fb->width, fb->height, "sbo");
  } else if (CAM_FORMAT == PIXFORMAT_GRAYSCALE) {
    imageLayer->cachePixelImageGS(imageName, fb->buf, fb->width, fb->height);
  }

  esp_camera_fb_return(fb);        // return frame so memory can be released

  return true;
}



// INMP441 I2S pin assignment
#if defined(FOR_LILYGO_TCAMERAPLUS)
  // for the mic built-in to LiLyGO TCamerPlus
  #define I2S_WS               32
  #define I2S_SD               33
  #define I2S_SCK              14
  #define I2S_SAMPLE_BIT_COUNT 32
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
#elif defined(FOR_LILYGO_TSIMCAM)
  // for the mic built-in to LiLyGO TSimCam
  #define I2S_WS               42
  #define I2S_SD                2
  #define I2S_SCK              41
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    8000
  #define SOUND_CHANNEL_COUNT  1
#else
  #define I2S_WS               25
  #define I2S_SD               33
  #define I2S_SCK              32
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    8000
  #define SOUND_CHANNEL_COUNT  1
#endif

// name of recorded WAV file; since only a single name; hence new one will always overwrite old one
const char* SoundName = "recorded_sound";


const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;


#if I2S_SAMPLE_BIT_COUNT == 32
  const int StreamBufferNumBytes = 256;//512;
  const int StreamBufferLen = StreamBufferNumBytes / 4;
  int32_t StreamBuffer[StreamBufferLen];
#else
  #if SOUND_SAMPLE_RATE == 16000
    // for 16 bits ... 16000 sample per second (32000 bytes per second; since 16 bits per sample) ==> 512 bytes = 16 ms per read
    const int StreamBufferNumBytes = 256;//512;
  #else
    // for 16 bits ... 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
    const int StreamBufferNumBytes = 128;//256;
  #endif  
  const int StreamBufferLen = StreamBufferNumBytes / 2;
  int16_t StreamBuffer[StreamBufferLen];
#endif

// sound sample (16 bits) amplification
// const int MaxAmplifyFactor = 20;
// const int DefAmplifyFactor = 10;

void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SOUND_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BIT_COUNT),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = I2S_DMA_BUF_COUNT/*8*/,
    .dma_buf_len = I2S_DMA_BUF_LEN/*1024*/,
    .use_apll = false
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
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

void readMicData(MicInfo &micInfo) {
  // read I2S data and place in data buffer
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
  int samplesRead = 0;
#if I2S_SAMPLE_BIT_COUNT == 32
  int16_t sampleStreamBuffer[StreamBufferLen];
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
      //float sumVal = 0;
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = StreamBuffer[i];
#if I2S_SAMPLE_BIT_COUNT == 32
        val = val / 0x0000ffff;
#endif
        if (micAmplifyFactor > 1) {
          val = micAmplifyFactor * val;
          if (val > 32700) {
            val = 32700;
          } else if (val < -32700) {
            val = -32700;
          }
        }
        sampleStreamBuffer[i] = val;
        //sumVal += val;
      }
      //float meanVal = sumVal / samplesRead;
      //plotterLayer->set(meanVal);
    }
  }
  micInfo.sampleRate = SOUND_SAMPLE_RATE;
  micInfo.numChannels = SOUND_CHANNEL_COUNT;
  micInfo.result = result;
  micInfo.samplesRead = samplesRead;
  micInfo.sampleStreamBuffer = sampleStreamBuffer;

}
