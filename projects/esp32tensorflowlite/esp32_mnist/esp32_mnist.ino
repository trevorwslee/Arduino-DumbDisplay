#include <Arduino.h>


//#define BLUETOOTH "ESP32BT"

#if defined BLUETOOTH

#include "esp32dumbdisplay.h"

DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));

#else

#include "wifidumbdisplay.h"
DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));


#endif



// #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"


// *****
// * make see https://github.com/frogermcs/MNIST-TFLite
// *****
#include "mnist_model.h"


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
const tflite::Model* model = ::tflite::GetModel(mnist_model_tflite);
const int tensor_arena_size = 81 * 1024;  // the same as used by esp32cam_person (person detection)
uint8_t* tensor_arena;
tflite::MicroInterpreter* interpreter = NULL;
TfLiteTensor* input;
// constexpr int kNumCols = 96;
// constexpr int kNumRows = 96;
// constexpr int kPersonIndex = 1;
// constexpr int kNotAPersonIndex = 2;
// const float PersonScoreThreshold = 0.6;


// const char* imageName = "esp32cam_gs";
// const int imageWidth = kNumCols;
// const int imageHeight = kNumRows;
// GraphicalDDLayer* detectImageLayer;
// GraphicalDDLayer* personImageLayer;
// LcdDDLayer* statusLayer;

GraphicalDDLayer* drawLayer;


// const framesize_t FrameSize = FRAMESIZE_96X96;        // should agree with kNumCols and kNumRows
// const pixformat_t PixelFormat = PIXFORMAT_GRAYSCALE;  // should be grayscale
// bool initialiseCamera();
// camera_fb_t* captureImage(bool useFlash);
// void releaseCapturedImage(camera_fb_t* fb);
// bool cameraReady;


void setup() {
  dumbdisplay.connect();
  dumbdisplay.writeComment("*** MNIST ***");

  // create and setup [top] graphical layer for showing candidate image for person detection;
  // clicking it will invoke person detection
  // detectImageLayer = dumbdisplay.createGraphicalLayer(imageWidth, imageHeight);
  // detectImageLayer->padding(3);
  // detectImageLayer->border(3, "blue", "round");
  // detectImageLayer->backgroundColor("blue");
  // detectImageLayer->enableFeedback("fl");

  // create and setup [middle] LCD layer for showing person detection status
  // statusLayer = dumbdisplay.createLcdLayer(16, 4);
  // statusLayer->padding(5);

  // create and setup [bottom] graphical layer for showing the image used for image detection
  // personImageLayer = dumbdisplay.createGraphicalLayer(imageWidth, imageHeight);
  // personImageLayer->padding(3);
  // personImageLayer->border(3, "blue", "round");
  // personImageLayer->backgroundColor("blue");

  // auto pin the layers vertically
  //dumbdisplay.configAutoPin(DD_AP_VERT);

  drawLayer = dumbdisplay.createGraphicalLayer(28, 28);
  drawLayer->border(2, "blue", "round", 1);
  drawLayer->enableFeedback("fs:drag");
  //drawLayer->enableFeedback("fs:rpt50");

  
  dumbdisplay.writeComment(String("Preparing Mnist TFLite model version ") + model->version() + " ...");

  // check version to make sure supported
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d not equal to supported version %d.",
    model->version(), TFLITE_SCHEMA_VERSION);
  }

  // allocation memory for tensor_arena ... in similar fashion as espcam_person.ino
  tensor_arena = (uint8_t *) heap_caps_malloc(tensor_arena_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (tensor_arena == NULL) {
    error_reporter->Report("heap_caps_malloc() failed");
    return;
  }

  // pull in all the operation implementations
  tflite::AllOpsResolver* resolver = new tflite::AllOpsResolver();

  // build an interpreter to run the model with
  interpreter = new tflite::MicroInterpreter(model, *resolver, tensor_arena, tensor_arena_size, error_reporter);

  // allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  // obtain a pointer to the model's input tensor
  input = interpreter->input(0);

  dumbdisplay.writeComment("Done preparing Mnist TFLite model!");


  // cameraReady = initialiseCamera(); 
  // if (cameraReady) {
  //   dumbdisplay.writeComment("Initialized camera!");
  // } else {
  //   dumbdisplay.writeComment("Failed to initialize camera!");
  // }

}


uint8_t Data[28 * 28];

void loop() {
  if (interpreter == NULL) {
    error_reporter->Report("Not Initialized!");
    delay(2000);
    return;
  }

  const DDFeedback *feedback = drawLayer->getFeedback();
  if (feedback != NULL) {
    int x = feedback->x;
    int y = feedback->y;
    if (true) {
      dumbdisplay.writeComment(String(x) + "," + String(y));
    }
    drawLayer->drawPixel(x, y, DD_RGB_COLOR(255, 255, 255));
  }

//  delay(2000);

  // capture candidate image for person detection
  // camera_fb_t* capturedImage = captureImage(false);
  // if (capturedImage == NULL) {
  //   error_reporter->Report("Error: Camera capture failed");
  //   return;
  // }

  // detectImageLayer->cachePixelImageGS(imageName, capturedImage->buf, imageWidth, imageHeight);  // cache image for drawing
  // detectImageLayer->drawImageFileFit(imageName);

  // check if detectImageLayer (top; candidate image) clicked
  // if clicked, invoke person detection
  // if (detectImageLayer->getFeedback() != NULL) {
  //   statusLayer->clear();s
  //   statusLayer->pixelColor("red");
  //   statusLayer->writeCenteredLine(".. detecting ..", 1);

  //   dumbdisplay.writeComment("start ... ");

    // copy an image with a person into the memory area used for the 
    
    if (false) {
      for (int i = 0; i < (28 * 28); ++i) {
        input->data.f[i] = ((float) Data[i]) / 255.0; 
        //input->data.int8[i] = person_data[i] ^ 0x80;  // signed int8_t quantized ==> input images must be converted from unisgned to signed format
      }

      // run the model on this input and make sure it succeeds
      dumbdisplay.writeComment("<<<");
      long detect_start_millis = millis();
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed");
      }
      long detect_taken_millis = millis() - detect_start_millis;

      // process the inference (person detection) results
      TfLiteTensor* output = interpreter->output(0);
      for (int i = 0; i < 10; i++) {
        float p = output->data.f[i];
        dumbdisplay.writeComment(String(". ") + String(i) + ": " + String(p, 3));

      }
      dumbdisplay.writeComment(String(">>> in ") + detect_taken_millis + " ms");
    }
    
    // int8_t _person_score = output->data.int8[kPersonIndex];
    // int8_t _no_person_score = output->data.int8[kNotAPersonIndex];
    // float person_score = (_person_score - output->params.zero_point) * output->params.scale;  // person_score should be chance from 0 to 1
    // float no_person_score = (_no_person_score - output->params.zero_point) * output->params.scale;
    // bool detected_person = person_score > PersonScoreThreshold;

    // dumbdisplay.writeComment(String("... person score: ") + String(person_score) + " ...");
    // dumbdisplay.writeComment(String("... NO person score: ") + String(no_person_score) + " ...");
    // dumbdisplay.writeComment("... done");

    // personImageLayer->unloadImageFile(imageName);  // remove any previous caching
    // if (detected_person) {
    //   // save image to phone
    //   dumbdisplay.savePixelImageGS(imageName, capturedImage->buf, imageWidth, imageHeight);
    //   dumbdisplay.writeComment("detected ... save image to phone");
    // } else {
    //   // only cache image for drawing
    //   personImageLayer->cachePixelImageGS(imageName, capturedImage->buf, imageWidth, imageHeight);
    // }
    // personImageLayer->drawImageFileFit(imageName);

    // statusLayer->clear();
    // if (detected_person) {
    //   personImageLayer->backgroundColor("green");
    //   statusLayer->pixelColor("darkgreen");
    //   statusLayer->writeCenteredLine("Detected!", 0);
    // } else {
    //   personImageLayer->backgroundColor("gray");
    //   statusLayer->pixelColor("darkgray");
    //   statusLayer->writeCenteredLine("NO person!", 0);
    // }
    // statusLayer->writeLine(String("  SCORE : ") + String((int8_t) (100 * person_score)) + "%", 2);
    // statusLayer->writeLine(String("  IN    : ") + String((float) detect_taken_millis / 1000.0) + "s", 3);

    //delay(1000);
  //}

  //releaseCapturedImage(capturedImage);
}


