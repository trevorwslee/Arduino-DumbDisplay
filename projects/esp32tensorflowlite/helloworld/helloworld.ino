#include <Arduino.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"


#include "sine_model_data.cc"


// Set up logging
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;


// Map the model into a usable data structure. This doesn't involve any
// copying or parsing, it's a very lightweight operation.
const tflite::Model* model = ::tflite::GetModel(g_sine_model_data);

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



void setup() {
  Serial.begin(115200);

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter.AllocateTensors();

  // Obtain a pointer to the model's input tensor
  input = interpreter.input(0);
}


float in = -1.5;

void loop() {
  delay(1000);
  // Serial.println("===");
  // Serial.print(". MODEL DESC:");
  // Serial.println(model->description()->c_str());

  //float in = 3.14 * ((float) rand() / (float) RAND_MAX);

  // Provide an input value
  input->data.f[0] = in;

  // Run the model on this input and check that it succeeds
  TfLiteStatus invoke_status = interpreter.Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed\n");
  }

  TfLiteTensor* output = interpreter.output(0);

  // Obtain the output value from the tensor
  float out = output->data.f[0];

  Serial.print("X:");
  Serial.print(in);
  Serial.print(" y:");
  Serial.println(out);

  in += 0.1;
}
