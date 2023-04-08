#define BLUETOOTH "ESP32BT"

#if defined BLUETOOTH

#include "esp32dumbdisplay.h"

DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));

#else

#include "wifidumbdisplay.h"
DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));


#endif

#include <TensorFlowLite_ESP32.h>

// #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"


// see: mnist_model.ipynb
#include "mnist_model.h"

// // see: https://github.com/frogermcs/MNIST-TFLite
// #include "frogermcs_mnist_model.h"


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
GraphicalDDLayer* copyLayer;
LcdDDLayer* clearBtn;
LcdDDLayer* centerBtn;
LcdDDLayer* influenceBtn;
SevenSegmentRowDDLayer* resultLayer;


// const framesize_t FrameSize = FRAMESIZE_96X96;        // should agree with kNumCols and kNumRows
// const pixformat_t PixelFormat = PIXFORMAT_GRAYSCALE;  // should be grayscale
// bool initialiseCamera();
// camera_fb_t* captureImage(bool useFlash);
// void releaseCapturedImage(camera_fb_t* fb);
// bool cameraReady;

const uint8_t thickerLineShade = 223;  // 0 to disable; 191 / 223 / 255

bool autoCenter = false;

int lastX = -1;
int lastY = -1;
//uint8_t Data[28 * 28];
uint8_t Pixels[28][28];



void _DrawPixel(int x, int y, uint8_t shade) {
  if (Pixels[x][y] < shade) {
    String color = DD_RGB_COLOR(shade, shade, shade);
    drawLayer->drawPixel(x, y, color);
    Pixels[x][y] = shade;
  }
}

void DrawPixel(int x, int y) {
  if (thickerLineShade > 0) {
    //dumbdisplay.recordLayerCommands();
    if (x > 0) {
      _DrawPixel(x - 1, y, thickerLineShade);
    }
    if (x < 27) {
      _DrawPixel(x + 1, y, thickerLineShade);
    }
    if (y > 0) {
      _DrawPixel(x, y - 1, thickerLineShade);
    }
    if (y < 27) {
      _DrawPixel(x, y + 1, thickerLineShade);
    }
  }
  _DrawPixel(x, y, 255);
  // if (thickerLineShade > 0) {
  //   dumbdisplay.playbackLayerCommands();
  // }
}

bool DrawLine(int x1, int y1, int x2, int y2) {
  int deltX = x2 - x1;
  int deltY = y2 - y1;
  int steps;
  float incX;
  float incY;
  if (abs(deltX) > abs(deltY)) {
    steps = abs(deltX);
    if (steps == 0) return false;
    incX = deltX < 0 ? -1 : 1;
    incY = (float) deltY / steps;
  } else {
    steps = abs(deltY);
    if (steps == 0) return false;
    incY = deltY < 0 ? -1 : 1;
    incX = (float) deltX / steps;
  }
  float x = x1;
  float y = y1;
  dumbdisplay.recordLayerCommands();
  for (int i = 0; i < steps; i++) {
    DrawPixel(round(x), round(y));
    x += incX;
    y += incY;
  }
  dumbdisplay.playbackLayerCommands();
  return true;
}


void ResetPixels() {
    lastX = -1;
    lastY = -1;
    drawLayer->clear();
    for (int x = 0; x < 28; x++) {
      for (int y = 0; y < 28; y++) {
        Pixels[x][y] = 0;
      }
    }
}

void DrawPixels(GraphicalDDLayer* targetLayer) {
    dumbdisplay.recordLayerCommands();
    targetLayer->clear();
    for (int x = 0; x < 28; x++) {
      for (int y = 0; y < 28; y++) {
        uint8_t shade = Pixels[x][y];
        if (shade != 0) {
          targetLayer->drawPixel(x, y, DD_RGB_COLOR(shade, shade, shade));
        }
      }
    }
    dumbdisplay.playbackLayerCommands();
}


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
  drawLayer->border(1, "lightgray", "round", 0.5);
  drawLayer->enableFeedback("fs:drag");

  copyLayer = dumbdisplay.createGraphicalLayer(28, 28);
  copyLayer->border(2, "blue", "round", 1);

  clearBtn = dumbdisplay.createLcdLayer(7, 1);   
  clearBtn->backgroundColor("lightgreen");
  clearBtn->pixelColor("darkblue");
  clearBtn->writeCenteredLine("clear");
  clearBtn->border(2, "darkgreen", "raised");
  clearBtn->enableFeedback("f");

  centerBtn = dumbdisplay.createLcdLayer(8, 1);
  centerBtn->writeCenteredLine("center");
  centerBtn->enableFeedback("fl");


  influenceBtn = dumbdisplay.createLcdLayer(3, 3);
  influenceBtn->pixelColor("darkblue");
  influenceBtn->writeCenteredLine(">>>", 1);
  influenceBtn->border(2, "gray", "raised");
  influenceBtn->enableFeedback("f");

  resultLayer = dumbdisplay.create7SegmentRowLayer();
  resultLayer->border(10, "blue", "round", 5);
  resultLayer->segmentColor("darkblue");
  //resultLayer->showFormatted("_");




  dumbdisplay.configAutoPin(
    DDAutoPinConfig('V')
      .beginGroup('H')
        .addLayer(clearBtn)
        .addLayer(centerBtn)
      .endGroup()
      .addLayer(drawLayer)
      .beginGroup('H')
        .addLayer(copyLayer)
        .addLayer(influenceBtn)
        .addLayer(resultLayer)  
      .endGroup()
      .build()
    );

  
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

  dumbdisplay.setIdleCalback([](long idleForMillis) {
#if defined(ESP32)    
    ESP.restart();
#endif    
  });

  ResetPixels();

  // cameraReady = initialiseCamera(); 
  // if (cameraReady) {
  //   dumbdisplay.writeComment("Initialized camera!");
  // } else {
  //   dumbdisplay.writeComment("Failed to initialize camera!");
  // }

}


// void DrawPixel(int x, int y);
// bool DrawLine(int x1, int y1, int x2, int y2);

bool started = false;
void loop() {
  if (interpreter == NULL) {
    error_reporter->Report("Not Initialized!");
    delay(2000);
    return;
  }

  bool update_ui = centerBtn->getFeedback() != NULL;
  if (!started) {
    started = true;
    update_ui = true;
  }
  if (update_ui) {
    autoCenter = !autoCenter;
    if (autoCenter) {
      centerBtn->pixelColor("darkblue");
      centerBtn->border(2, "gray", "flat");
    } else {
      centerBtn->pixelColor("gray");
      centerBtn->border(2, "gray", "hair");
    }
  }

  if (clearBtn->getFeedback()) {
    ResetPixels();
  }

  const DDFeedback *feedback = drawLayer->getFeedback();
  if (feedback != NULL) {
    int x = feedback->x;
    int y = feedback->y;
    if (false) {
      dumbdisplay.writeComment(String(". ") + String(x) + "," + String(y));
    }
    if (x == -1) {
      lastX = -1;
      lastY = -1;  
    } else {
      bool update = true;
      if (lastX == -1) {
        DrawPixel(x, y);
      } else {
        if (lastX != x || lastY != y) {
          update = DrawLine(lastX, lastY, x, y);
        }
      }
      if (update) {
        lastX = x;
        lastY = y;
      }
    }
  }

  bool doInfluence = influenceBtn->getFeedback() != NULL;

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
    
    if (doInfluence) {
      drawLayer->disabled(true);
      if (autoCenter) {
        int minX = 27;
        int maxX = 0;
        int minY = 27;
        int maxY = 0;
        for (int x = 0; x < 28; x++) {
          for (int y = 0; y < 28; y++) {
            if (Pixels[x][y] != 0) {
              if (x < minX) {
                minX = x;
              }
              if (x > maxX) {
                maxX = x;
              }
              if (y < minY) {
                minY = y;
              }
              if (y > maxY) {
                maxY = y;
              }
            } 
          }
        }
        int xDelta = (((27 - maxX) + minX) / 2) - minX;
        int yDelta = (((27 - maxY) + minY) / 2) - minY;
        if (xDelta != 0 || yDelta != 0) {
          if (true) {
            dumbdisplay.writeComment(String("!!! shift ") + xDelta + "," + yDelta);
          }
          uint8_t newPixels[28][28];
          for (int x = 0; x < 28; x++) {
            for (int y = 0; y < 28; y++) {
              int newX = x - xDelta;
              int newY = y - yDelta;
              uint8_t pixel = 0;
              if (newX >= 0 && newX <= 27 && newY >= 0 && newY <= 27) {
                pixel = Pixels[newX][newY];
              }
              newPixels[x][y] = pixel;
            }
          }
          for (int x = 0; x < 28; x++) {
            for (int y = 0; y < 28; y++) {
              Pixels[x][y] = newPixels[x][y];
            }
          }
        }
      }
      int idx = 0;
      for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
          input->data.f[idx++] = ((float) Pixels[x][y]) / 255.0; 
        }
      }
      // for (int i = 0; i < (28 * 28); ++i) {
      //   input->data.f[i] = ((float) Data[i]) / 255.0; 
      //   //input->data.int8[i] = person_data[i] ^ 0x80;  // signed int8_t quantized ==> input images must be converted from unisgned to signed format
      // }

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
      if (true) {
        for (int i = 0; i < 10; i++) {
          float p = output->data.f[i];
          dumbdisplay.writeComment(String(". ") + String(i) + ": " + String(p, 3));

        }
        dumbdisplay.writeComment(String(">>> in ") + detect_taken_millis + " ms");
      }
      int best = -1;
      float bestProp;
      for (int i = 0; i < 10; i++) {
        float prop = output->data.f[i];
        if (i == 0) {
          best = 0;
          bestProp = prop; 
        } else if (prop > bestProp) {
          best = i;
          bestProp = prop;
        }
      }
      dumbdisplay.writeComment(String("*** [") + best + "] (" + bestProp + ") in " + detect_taken_millis + " ms");
      resultLayer->showDigit(best);
      DrawPixels(copyLayer);
      ResetPixels();
      drawLayer->disabled(false);
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

