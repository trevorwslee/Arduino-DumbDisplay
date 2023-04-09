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


#include "sine_model.h"


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


// Set up logging
DDTFLErrorReporter error_reporter_impl;
tflite::ErrorReporter* error_reporter = &error_reporter_impl;


// Map the model into a usable data structure. This doesn't involve any
// copying or parsing, it's a very lightweight operation.
const tflite::Model* model = ::tflite::GetModel(sine_model_tflite);

// This pulls in all the operation implementations we need
tflite::AllOpsResolver resolver;

// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
const int tensor_arena_size = 2 * 1024;
uint8_t tensor_arena[tensor_arena_size];  


// Build an interpreter to run the model with
tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                     tensor_arena_size, error_reporter);


// Obtain a pointer to the model's input tensor
TfLiteTensor* input;



PlotterDDLayer* plotterLayer;
GraphicalDDLayer* graphicalLayer;


const float start_in = -1.4;
const float max_in = 7.6;

const int width = 640;
const int height = 360;
const float xScaleFactor = width / (max_in - start_in);
const float yScaleFactor = height / 2;
const int xOffset = -start_in * xScaleFactor;
const int yOffset = yScaleFactor;



void setup() {
  // create a plotter layer for plotting the inference result value
  plotterLayer = dumbdisplay.createPlotterLayer(width, height);

  // create a graphical layer for drawing out properly positioned and scaled inference value
  graphicalLayer = dumbdisplay.createGraphicalLayer(width, height);
  graphicalLayer->backgroundColor("ivory");
  graphicalLayer->drawLine(0, yOffset, width, yOffset, "blue");
  graphicalLayer->drawLine(xOffset, 0, xOffset, height, "blue");

  // static the two layers, one on top of the other
  dumbdisplay.configAutoPin(DD_AP_VERT);

  // check version to make sure supported
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d not equal to supported version %d.\n",
    model->version(), TFLITE_SCHEMA_VERSION);
  }

  // allocate memory from the tensor_arena for the model's tensors
  interpreter.AllocateTensors();

  // obtain a pointer to the model's input tensor
  input = interpreter.input(0);
}


float in = start_in;
int color = 0; // 0: "red"; 1: "green"

void loop() {
  delay(250);

  // provide an input value
  input->data.f[0] = in;

  // run the model on this input and check that it succeeds
  TfLiteStatus invoke_status = interpreter.Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed\n");
  }

  TfLiteTensor* output = interpreter.output(0);

  // obtain the output value from the tensor
  float out = output->data.f[0];

  // plot the input and output value to plotter layer as x and y
  plotterLayer->set("x", in, "y", out);

  // properly position and scale the in / out values, and draw it as a dot on the graphical layer
  int x = xOffset + in * xScaleFactor;
  int y = yOffset - out * yScaleFactor; 
  graphicalLayer->fillCircle(x, y, 2, color == 0 ? "red" : "green");

  // increment the in value, by some randomized amount
  float inc = (float) random(10) / 1000.0;
  in += 0.04 + inc;

  if (in > max_in) {
    in = start_in + (inc / 3.0);
    color = (color + 1) % 2;
  }
}
