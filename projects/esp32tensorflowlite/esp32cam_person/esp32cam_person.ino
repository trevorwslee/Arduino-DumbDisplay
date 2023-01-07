#include <Arduino.h>

#include "esp_camera.h" 
#include "esp32dumbdisplay.h"


DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32CAM"));


#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// *****
// * copied from TensorFlowLite_ESP32 library example person_detection
// * e.g. C:\Users\trevorlee\Documents\Arduino\libraries\TensorFlowLite_ESP32\examples\person_detection\person_detect_model_data.cpp
// * . renamed to person_detect_model_data.cc
// * . commented out the line
// *   #include "person_detect_model_data.h" 
// *****
#include "person_detect_model_data.cc"


class DDTFLErrorReporter : public tflite::ErrorReporter {
public:
  virtual int Report(const char* format, va_list args) {
    int len = strlen(format);
    char buffer[max(32, 2 * len)];  // assume 2 times format len is big enough
    sprintf(buffer, format, args);
    dumbdisplay.writeComment(buffer);
    return 0;
  }
};

tflite::ErrorReporter* error_reporter = new DDTFLErrorReporter();

const tflite::Model* model = ::tflite::GetModel(g_person_detect_model_data);

const int tensor_arena_size = 81 * 1024;
uint8_t* tensor_arena;

tflite::MicroInterpreter* interpreter = NULL;

TfLiteTensor* input;


constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kNumChannels = 1;


//constexpr int kCategoryCount = 3;
constexpr int kPersonIndex = 1;
constexpr int kNotAPersonIndex = 2;

// const char* kCategoryLabels[kCategoryCount] = {
//     "unused",
//     "person",
//     "notperson",
// };


const char* imageName = "esp32cam_gs";
const int imageWidth = kNumCols;
const int imageHeight = kNumRows;

GraphicalDDLayer* detectImageLayer;
GraphicalDDLayer* personImageLayer;
LcdDDLayer* statusLayer;



// // ***** ????? *****
// const uint8_t* g_person_data = new uint8_t[kNumCols * kNumRows];


const framesize_t FrameSize = FRAMESIZE_96X96;        // should agree with kNumCols and kNumRows
const pixformat_t PixelFormat = PIXFORMAT_GRAYSCALE;  // should be grayscale
bool initialiseCamera();
camera_fb_t* captureImage(bool useFlash);
void releaseCapturedImage(camera_fb_t* fb);

bool cameraReady;



void setup() {
  detectImageLayer = dumbdisplay.createGraphicalLayer(imageWidth, imageHeight);
  detectImageLayer->padding(3);
  detectImageLayer->border(3, "blue", "round");
  detectImageLayer->backgroundColor("blue");
  detectImageLayer->enableFeedback("fl");

  statusLayer = dumbdisplay.createLcdLayer(16, 4);
  statusLayer->padding(5);

  personImageLayer = dumbdisplay.createGraphicalLayer(imageWidth, imageHeight);
  personImageLayer->padding(3);
  personImageLayer->border(3, "blue", "round");
  personImageLayer->backgroundColor("blue");


  dumbdisplay.configAutoPin(DD_AP_VERT);


  dumbdisplay.writeComment(String("Preparing TFLite model version ") + model->version() + " ...");

  // check version to make sure supported
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d not equal to supported version %d.",
    model->version(), TFLITE_SCHEMA_VERSION);
  }

  // allocation memory for tensor_arena
  tensor_arena = (uint8_t *) heap_caps_malloc(tensor_arena_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (tensor_arena == NULL) {
    error_reporter->Report("heap_caps_malloc() failed");
    return;
  }


  // pull in only the operation implementations needed
  tflite::MicroMutableOpResolver<5>* micro_op_resolver = new tflite::MicroMutableOpResolver<5>();
  micro_op_resolver->AddAveragePool2D();
  micro_op_resolver->AddConv2D();
  micro_op_resolver->AddDepthwiseConv2D();
  micro_op_resolver->AddReshape();
  micro_op_resolver->AddSoftmax();


  // build an interpreter to run the model with
  interpreter = new tflite::MicroInterpreter(model, *micro_op_resolver, tensor_arena, tensor_arena_size, error_reporter);

  // allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  // obtain a pointer to the model's input tensor
  input = interpreter->input(0);

  dumbdisplay.writeComment("Done preparing TFLite model!");


  cameraReady = initialiseCamera(); 
  if (cameraReady) {
    dumbdisplay.writeComment("Initialized camera!");
  } else {
    dumbdisplay.writeComment("Failed to initialize camera!");
  }

}


// int detection_count = 0;
// float avg_taken_s = 0;

// uint8_t last_person_score = 0;
// uint8_t last_no_person_score = 0;


void loop() {

  if (interpreter == NULL) {
    error_reporter->Report("Not Initialized!");
    delay(2000);
    return;
  }

  camera_fb_t* capturedImage = captureImage(false);
  if (capturedImage == NULL) {
    error_reporter->Report("Error: Camera capture failed");
    return;
  }

  detectImageLayer->cachePixelImageGS(imageName, capturedImage->buf, imageWidth, imageHeight);
  detectImageLayer->drawImageFileFit(imageName);

  if (detectImageLayer->getFeedback() != NULL) {

    statusLayer->clear();
    statusLayer->writeCenteredLine(".. detecting ..", 1);

    dumbdisplay.writeComment("start ... ");

    // copy an image with a person into the memory area used for the input
    const uint8_t* person_data = capturedImage->buf;
    for (int i = 0; i < input->bytes; ++i) {
        input->data.uint8[i] = person_data[i];
    }

    long detect_start_millis = millis();

    // run the model on this input and make sure it succeeds
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed");
    }

    long detect_taken_millis = millis() - detect_start_millis;


    // process the inference results
    TfLiteTensor* output = interpreter->output(0);
    uint8_t person_score = output->data.uint8[kPersonIndex];
    uint8_t no_person_score = output->data.uint8[kNotAPersonIndex];

    dumbdisplay.writeComment(String("... person score: ") + person_score + " ...");
    dumbdisplay.writeComment(String("... NO person score: ") + no_person_score + " ...");
    dumbdisplay.writeComment("... done");

    float detect_taken_s = detect_taken_millis / 1000.0;

    statusLayer->clear();
    if (person_score > no_person_score) {
      personImageLayer->backgroundColor("green");
      statusLayer->writeCenteredLine("Detected!", 0);
    } else {
      personImageLayer->backgroundColor("gray");
      statusLayer->writeCenteredLine("NO person!", 0);
    }
    statusLayer->writeCenteredLine(String(person_score) + "  vs  " + String(no_person_score), 2);
    statusLayer->writeCenteredLine(String("in ") + detect_taken_s + "s", 3);
    dumbdisplay.savePixelImageGS(imageName, capturedImage->buf, imageWidth, imageHeight);
    personImageLayer->drawImageFileFit(imageName);
  }

  releaseCapturedImage(capturedImage);
}




int cameraImageBrightness = 0;                       // Image brightness (-2 to +2)

const int brightLED = 4;                             // onboard Illumination/flash LED pin (4)
const int ledFreq = 5000;                            // PWM settings
const int ledChannel = 15;                           // camera uses timer1
const int ledRresolution = 8;                        // resolution (8 = from 0 to 255)

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




void brightLed(byte ledBrightness){
   ledcWrite(ledChannel, ledBrightness);   // change LED brightness (0 - 255)
}

void setupFlashPWM() {
    ledcSetup(ledChannel, ledFreq, ledRresolution);
    ledcAttachPin(brightLED, ledChannel);
    brightLed(32);
    brightLed(0);
}


bool cameraImageSettings() {

  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    return 0;
  }

  // enable auto adjust
  s->set_gain_ctrl(s, 1);                       // auto gain on
  s->set_exposure_ctrl(s, 1);                   // auto exposure on
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness

   return 1;
}


bool initialiseCamera() {
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
  config.pixel_format = PixelFormat;            // Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
  config.frame_size = FrameSize;                // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA),
                                                //              400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA),
                                                //              1600x1200 (UXGA)
  config.jpeg_quality = 15;                     // 0-63 lower number means higher quality
  config.fb_count = 1;                          // if more than one, i2s runs in continuous mode. Use only with JPEG

  // check the esp32cam board has a psram chip installed (extra memory used for storing captured images)
  //    Note: if not using "AI thinker esp32 cam" in the Arduino IDE, SPIFFS must be enabled
  if (!psramFound()) {
    error_reporter->Report("Warning: No PSRam found so defaulting to image size 'CIF'");
    config.frame_size = FRAMESIZE_CIF;
  }

  esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
  if (camerr != ESP_OK) {
    error_reporter->Report("ERROR: Camera init failed with error 0x%x", camerr);
  }

  cameraImageSettings();                        // apply custom camera settings

  return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}


camera_fb_t* captureImage(bool useFlash) {

  if (useFlash) brightLed(255);            // change LED brightness (0 - 255)
  camera_fb_t *fb = esp_camera_fb_get();   // capture image frame from camera
  if (useFlash) brightLed(0);              // change LED brightness back to previous state
  // if (fb == NULL) {
  //   error_reporter->Report("Error: Camera capture failed");
  //   return NULL;
  // }
  return fb;
  // //dumbdisplay.writeComment("Image Size: " + String(fb->len));
  // if (cacheOnly) {
  //   imageLayer->cachePixelImageGS(imageName, fb->buf, imageWidth, imageHeight, "0>a0");
  // } else {
  //   dumbdisplay.writeComment("Saving image (" + String(fb->len) + ") ...");
  //   dumbdisplay.savePixelImageGS(imageName, fb->buf, imageWidth, imageHeight);
  //   dumbdisplay.writeComment("... saved image");
  // }

  // esp_camera_fb_return(fb);        // return frame so memory can be released

  // return true;
}
void releaseCapturedImage(camera_fb_t* fb) {
  esp_camera_fb_return(fb);        // return frame so memory can be released
}

