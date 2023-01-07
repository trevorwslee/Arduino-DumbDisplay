#include <Arduino.h>

// #include "esp32dumbdisplay.h"
// DumbDisplay dumbdisplay(new DDBluetoothSerialIO("BT32"));


//#include "tensorflow/lite/micro/all_ops_resolver.h"
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
    Serial.println(buffer);
    //dumbdisplay.writeComment(buffer);
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

const char* kCategoryLabels[kCategoryCount] = {
    "unused",
    "person",
    "notperson",
};



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



const uint8_t* g_person_data = new uint8_t[96 * 96];

void setup() {
  Serial.begin(115200);

  // // create a plotter layer for plotting the inference result value
  // plotterLayer = dumbdisplay.createPlotterLayer(width, height);

  // // create a graphical layer for drawing out properly positioned and scaled inference value
  // graphicalLayer = dumbdisplay.createGraphicalLayer(width, height);
  // graphicalLayer->backgroundColor("ivory");
  // graphicalLayer->drawLine(0, yOffset, width, yOffset, "blue");
  // graphicalLayer->drawLine(xOffset, 0, xOffset, height, "blue");

  // // static the two layers, one on top of the other
  // dumbdisplay.configAutoPin(DD_AP_VERT);


  Serial.println("%%%%%%%%%%");

  Serial.print("MODEL VERSION:");
  Serial.println(model->version());
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

  Serial.print("start ... ");
  Serial.print(input->bytes);
  Serial.println("...");

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
  Serial.print("... person score: ");
  Serial.print(person_score);
  Serial.println(" ...");
  Serial.print("... no person score: ");
  Serial.print(no_person_score);
  Serial.println(" ...");

  Serial.println("... done");
  
  delay(2000);
  yield();


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
