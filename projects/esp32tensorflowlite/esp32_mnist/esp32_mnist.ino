// if want Bluetooth, uncomment the following line
// #define BLUETOOTH "ESP32BT"
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
#endif


#include <TensorFlowLite_ESP32.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"


// *****
// * make sure signature of the model is
// *   const unsigne char mnist_model_tflite[] = ...
// *****
#include "mnist_model.h"
//#include "frogermcs_mnist_model.h"


// error reporter for TensorFlow Lite
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
const int tensor_arena_size = 81 * 1024;  // guess ... the same as used by esp32cam_person (person detection)
uint8_t* tensor_arena;
tflite::MicroInterpreter* interpreter = NULL;
TfLiteTensor* input;


GraphicalDDLayer* drawLayer;
GraphicalDDLayer* copyLayer;
LcdDDLayer* clearBtn;
LcdDDLayer* centerBtn;
LcdDDLayer* inferenceBtn;
SevenSegmentRowDDLayer* resultLayer;


const uint8_t thickerLineShade = 223;  // 0 to disable; other values can be 191 / 255


bool autoCenter = false;

int lastX = -1;
int lastY = -1;
uint8_t Pixels[28][28];  // pixels data



// set draw the pixel and set the corresponding position of the pixels data
void _RenderPixel(int x, int y, uint8_t shade) {
  if (Pixels[x][y] < shade) {
    String color = DD_RGB_COLOR(shade, shade, shade);
    drawLayer->drawPixel(x, y, color);
    Pixels[x][y] = shade;
  }
}

void DrawPixel(int x, int y) {
  if (thickerLineShade > 0) {
    if (x > 0) {
      _RenderPixel(x - 1, y, thickerLineShade);
    }
    if (x < 27) {
      _RenderPixel(x + 1, y, thickerLineShade);
    }
    if (y > 0) {
      _RenderPixel(x, y - 1, thickerLineShade);
    }
    if (y < 27) {
      _RenderPixel(x, y + 1, thickerLineShade);
    }
  }
  _RenderPixel(x, y, 255);
}

bool DrawLine(int x1, int y1, int x2, int y2) {
  int deltX = x2 - x1;
  int deltY = y2 - y1;
  int steps;
  float incX;
  float incY;
  if (abs(deltX) > abs(deltY)) {
    steps = abs(deltX);
    if (steps == 0) return false;  // nothing to draw
    incX = deltX < 0 ? -1 : 1;
    incY = (float) deltY / steps;
  } else {
    steps = abs(deltY);
    if (steps == 0) return false;  // nothing to draw
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


// reset the pixels data
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

// draw the pixels data to the given layer
void DrawPixelsTo(GraphicalDDLayer* targetLayer) {
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
  drawLayer = dumbdisplay.createGraphicalLayer(28, 28);
  drawLayer->border(1, DD_COLOR_lightgray, "round", 0.5);
  drawLayer->enableFeedback("fs:drag");

  copyLayer = dumbdisplay.createGraphicalLayer(28, 28);
  copyLayer->border(2, DD_COLOR_blue, "round", 1);

  clearBtn = dumbdisplay.createLcdLayer(7, 1);   
  clearBtn->backgroundColor(DD_COLOR_lightgreen);
  clearBtn->pixelColor(DD_COLOR_darkblue);
  clearBtn->writeCenteredLine("clear");
  clearBtn->border(2, DD_COLOR_darkgreen, "raised");
  clearBtn->enableFeedback("f");

  centerBtn = dumbdisplay.createLcdLayer(8, 1);
  centerBtn->writeCenteredLine("center");
  centerBtn->enableFeedback("fl");

  inferenceBtn = dumbdisplay.createLcdLayer(3, 3);
  inferenceBtn->pixelColor(DD_COLOR_darkblue);
  inferenceBtn->writeCenteredLine(">>>", 1);
  inferenceBtn->border(2, DD_COLOR_gray, "raised");
  inferenceBtn->enableFeedback("f");

  resultLayer = dumbdisplay.create7SegmentRowLayer();
  resultLayer->border(10, DD_COLOR_blue, "round", 5);
  resultLayer->segmentColor(DD_COLOR_darkblue);

  dumbdisplay.configAutoPin(
    DDAutoPinConfig('V')
      .beginGroup('H')
        .addLayer(clearBtn)
        .addLayer(centerBtn)
      .endGroup()
      .addLayer(drawLayer)
      .beginGroup('H')
        .addLayer(copyLayer)
        .addLayer(inferenceBtn)
        .addLayer(resultLayer)  
      .endGroup()
      .build()
    );


  // set "idle callback restart ESP32 if idle (i.e. disconnected)
  dumbdisplay.setIdleCallback([](long idleForMillis, DDIdleConnectionState connectionState) {
    if (connectionState == DDIdleConnectionState::IDLE_RECONNECTING) {
      ESP.restart(); 
    }
  });

  
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

  ResetPixels();
}


bool started = false;
void loop() {
  if (interpreter == NULL) {
    error_reporter->Report("Not Initialized!");
    delay(2000);
    return;
  }

  bool toogleAutoCenter = centerBtn->getFeedback() != NULL;
  if (!started) {
    started = true;
    toogleAutoCenter = true;
  }
  if (toogleAutoCenter) {
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

  bool doInference = inferenceBtn->getFeedback() != NULL;
  if (doInference) {
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

    // set the pixels as the model's input
    int idx = 0;
    for (int y = 0; y < 28; y++) {
      for (int x = 0; x < 28; x++) {
        input->data.f[idx++] = ((float) Pixels[x][y]) / 255.0; 
      }
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
    DrawPixelsTo(copyLayer);
    ResetPixels();
    drawLayer->disabled(false);
  }
   
}

