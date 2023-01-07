#include <Arduino.h>

#include "esp_camera.h" 

#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("BT32"));


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
    //Serial.println(buffer);
    dumbdisplay.writeComment(buffer);
    return 0;
  }
};


// Set up logging
//DDTFLErrorReporter error_reporter_impl;
//tflite::ErrorReporter* error_reporter = &error_reporter_impl;
tflite::ErrorReporter* error_reporter = new DDTFLErrorReporter();


// Map the model into a usable data structure. This doesn't involve any
// copying or parsing, it's a very lightweight operation.
const tflite::Model* model = ::tflite::GetModel(g_person_detect_model_data);

// // This pulls in all the operation implementations we need
// tflite::AllOpsResolver resolver;

// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
//const int tensor_arena_size = 70 * 1024;
//uint8_t tensor_arena[tensor_arena_size];  
const int tensor_arena_size = 81 * 1024;
uint8_t* tensor_arena;


tflite::MicroInterpreter* interpreter = NULL;

// // Build an interpreter to run the model with
// tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
//                                      tensor_arena_size, error_reporter);


// Obtain a pointer to the model's input tensor
TfLiteTensor* input;


constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kNumChannels = 1;


constexpr int kCategoryCount = 3;
constexpr int kPersonIndex = 1;
constexpr int kNotAPersonIndex = 2;

// const char* kCategoryLabels[kCategoryCount] = {
//     "unused",
//     "person",
//     "notperson",
// };



//PlotterDDLayer* plotterLayer;
//GraphicalDDLayer* graphicalLayer;


// const float start_in = -1.4;
// const float max_in = 7.6;

// const int width = 640;
// const int height = 360;
// const float xScaleFactor = width / (max_in - start_in);
// const float yScaleFactor = height / 2;
// const int xOffset = -start_in * xScaleFactor;
// const int yOffset = yScaleFactor;



// ***** ????? *****
const uint8_t* g_person_data = new uint8_t[96 * 96];


const framesize_t FrameSize = FRAMESIZE_96X96;
const pixformat_t PixelFormat = PIXFORMAT_GRAYSCALE;
bool initialiseCamera();

bool cameraReady;

void setup() {
  dumbdisplay.connect();

  //Serial.begin(115200);

  // // create a plotter layer for plotting the inference result value
  // plotterLayer = dumbdisplay.createPlotterLayer(width, height);

  // // create a graphical layer for drawing out properly positioned and scaled inference value
  // graphicalLayer = dumbdisplay.createGraphicalLayer(width, height);
  // graphicalLayer->backgroundColor("ivory");
  // graphicalLayer->drawLine(0, yOffset, width, yOffset, "blue");
  // graphicalLayer->drawLine(xOffset, 0, xOffset, height, "blue");

  // // static the two layers, one on top of the other
  // dumbdisplay.configAutoPin(DD_AP_VERT);



  dumbdisplay.writeComment(String("Preparing TFLite model version ") + model->version() + " ...");
  //Serial.println("%%%%%%%%%%");

  //Serial.print("MODEL VERSION:");
  //Serial.println(model->version());
  // check version to make sure supported
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d not equal to supported version %d.",
    model->version(), TFLITE_SCHEMA_VERSION);
  }

  tensor_arena = (uint8_t *) heap_caps_malloc(tensor_arena_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (tensor_arena == NULL) {
    error_reporter->Report("heap_caps_malloc() failed");
    //interpreter = NULL;
    return;
  }


  // Pull in only the operation implementations we need.
  //static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  tflite::MicroMutableOpResolver<5>* micro_op_resolver = new tflite::MicroMutableOpResolver<5>();
  micro_op_resolver->AddAveragePool2D();
  micro_op_resolver->AddConv2D();
  micro_op_resolver->AddDepthwiseConv2D();
  micro_op_resolver->AddReshape();
  micro_op_resolver->AddSoftmax();


  // Build an interpreter to run the model with.
  // static tflite::MicroInterpreter static_interpreter(model, *micro_op_resolver, tensor_arena, tensor_arena_size, error_reporter);
  // interpreter = &static_interpreter;
  interpreter = new tflite::MicroInterpreter(model, *micro_op_resolver, tensor_arena, tensor_arena_size, error_reporter);

  // allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    //interpreter = NULL;
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


//float in = start_in;
//int color = 0; // 0: "red"; 1: "green"

//void RespondToDetection(uint8_t person_score, uint8_t no_person_score);

void loop() {

  if (interpreter == NULL) {
    error_reporter->Report("Not Initialized!");
    delay(2000);
    return;
  }

  dumbdisplay.writeComment(String("start ... ") + input->bytes + " ...");

  //Serial.print("start ... ");
  //Serial.print(input->bytes);
  //Serial.println("...");

  // Copy an image with a person into the memory area used for the input.
  const uint8_t* person_data = g_person_data;
  for (int i = 0; i < input->bytes; ++i) {
      input->data.uint8[i] = person_data[i];
  }

  // Run the model on this input and make sure it succeeds.
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed");
  }

  TfLiteTensor* output = interpreter->output(0);

  // Process the inference results.
  uint8_t person_score = output->data.uint8[kPersonIndex];
  uint8_t no_person_score = output->data.uint8[kNotAPersonIndex];
  //RespondToDetection(person_score, no_person_score);

  dumbdisplay.writeComment(String("... person score: ") + person_score + " ...");
  dumbdisplay.writeComment(String("... NO person score: ") + no_person_score + " ...");
  dumbdisplay.writeComment("... done");

  // Serial.print("... person score: ");
  // Serial.print(person_score);
  // Serial.println(" ...");
  // Serial.print("... no person score: ");
  // Serial.print(no_person_score);
  // Serial.println(" ...");

//  Serial.println("... done");
  
  delay(2000);
  //yield();


  // delay(250);

  // // provide an input value
  // input->data.f[0] = in;

  // // run the model on this input and check that it succeeds
  // TfLiteStatus invoke_status = interpreter.Invoke();
  // if (invoke_status != kTfLiteOk) {
  //   error_reporter->Report("Invoke failed\n");
  // }

  // TfLiteTensor* output = interpreter.output(0);

  // // obtain the output value from the tensor
  // float out = output->data.f[0];

  // // plot the input and output value to plotter layer as x and y
  // plotterLayer->set("x", in, "y", out);

  // // properly position and scale the in / out values, and draw it as a dot on the graphical layer
  // int x = xOffset + in * xScaleFactor;
  // int y = yOffset - out * yScaleFactor; 
  // graphicalLayer->fillCircle(x, y, 2, color == 0 ? "red" : "green");

  // // increment the in value, by some randomized amount
  // float inc = (float) random(10) / 1000.0;
  // in += 0.04 + inc;

  // if (in > max_in) {
  //   in = start_in + (inc / 3.0);
  //   color = (color + 1) % 2;
  // }
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

  //if (serialDebug) Serial.println("Applying camera settings");

  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    //if (serialDebug) Serial.println("Error: problem reading camera sensor settings");
    return 0;
  }

  // enable auto adjust
  s->set_gain_ctrl(s, 1);                       // auto gain on
  s->set_exposure_ctrl(s, 1);                   // auto exposure on
  s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
  s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness

   return 1;
}  // cameraImageSettings


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
    //if (serialDebug) Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
    error_reporter->Report("Warning: No PSRam found so defaulting to image size 'CIF'");
    config.frame_size = FRAMESIZE_CIF;
  }

  esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
  if (camerr != ESP_OK) {
    //if (serialDebug) Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
    error_reporter->Report("ERROR: Camera init failed with error 0x%x", camerr);
  }

  cameraImageSettings();                        // apply custom camera settings

  return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}
