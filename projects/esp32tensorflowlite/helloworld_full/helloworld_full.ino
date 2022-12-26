//#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"

//#include "tensorflow/lite/micro/testing/micro_test.h"

#include "tensorflow/lite/schema/schema_generated.h"
//#include "tensorflow/lite/version.h"



#include "tensorflow/lite/micro/micro_error_reporter.h"
namespace micro_test {                      
  int tests_passed;                            
  int tests_failed;                            
  bool is_test_complete;                       
  bool did_test_fail;                          
  tflite::ErrorReporter* reporter;             
}    
#define TF_LITE_MICRO_EXPECT(x)                                                \
  do {                                                                         \
    if (!(x)) {                                                                \
      micro_test::reporter->Report(#x " failed at %s:%d", __FILE__, __LINE__); \
      micro_test::did_test_fail = true;                                        \
    }                                                                          \
  } while (false)

#define TF_LITE_MICRO_EXPECT_EQ(x, y)                                          \
  do {                                                                         \
    if ((x) != (y)) {                                                          \
      micro_test::reporter->Report(#x " == " #y " failed at %s:%d (%d vs %d)", \
                                   __FILE__, __LINE__, (x), (y));              \
      micro_test::did_test_fail = true;                                        \
    }                                                                          \
  } while (false)

#define TF_LITE_MICRO_EXPECT_NE(x, y)                                         \
  do {                                                                        \
    if ((x) == (y)) {                                                         \
      micro_test::reporter->Report(#x " != " #y " failed at %s:%d", __FILE__, \
                                   __LINE__);                                 \
      micro_test::did_test_fail = true;                                       \
    }                                                                         \
  } while (false)

// TODO(wangtz): Making it more generic once needed.
#define TF_LITE_MICRO_ARRAY_ELEMENT_EXPECT_NEAR(arr1, idx1, arr2, idx2, \
                                                epsilon)                \
  do {                                                                  \
    auto delta = ((arr1)[(idx1)] > (arr2)[(idx2)])                      \
                     ? ((arr1)[(idx1)] - (arr2)[(idx2)])                \
                     : ((arr2)[(idx2)] - (arr1)[(idx1)]);               \
    if (delta > epsilon) {                                              \
      micro_test::reporter->Report(                                     \
          #arr1 "[%d] (%f) near " #arr2 "[%d] (%f) failed at %s:%d",    \
          static_cast<int>(idx1), static_cast<float>((arr1)[(idx1)]),   \
          static_cast<int>(idx2), static_cast<float>((arr2)[(idx2)]),   \
          __FILE__, __LINE__);                                          \
      micro_test::did_test_fail = true;                                 \
    }                                                                   \
  } while (false)

#define TF_LITE_MICRO_EXPECT_NEAR(x, y, epsilon)                              \
  do {                                                                        \
    auto delta = ((x) > (y)) ? ((x) - (y)) : ((y) - (x));                     \
    if (delta > epsilon) {                                                    \
      micro_test::reporter->Report(                                           \
          #x " (%f) near " #y " (%f) failed at %s:%d", static_cast<float>(x), \
          static_cast<float>(y), __FILE__, __LINE__);                         \
      micro_test::did_test_fail = true;                                       \
    }                                                                         \
  } while (false)

#define TF_LITE_MICRO_EXPECT_GT(x, y)                                        \
  do {                                                                       \
    if ((x) <= (y)) {                                                        \
      micro_test::reporter->Report(#x " > " #y " failed at %s:%d", __FILE__, \
                                   __LINE__);                                \
      micro_test::did_test_fail = true;                                      \
    }                                                                        \
  } while (false)

#define TF_LITE_MICRO_EXPECT_LT(x, y)                                        \
  do {                                                                       \
    if ((x) >= (y)) {                                                        \
      micro_test::reporter->Report(#x " < " #y " failed at %s:%d", __FILE__, \
                                   __LINE__);                                \
      micro_test::did_test_fail = true;                                      \
    }                                                                        \
  } while (false)

#define TF_LITE_MICRO_EXPECT_GE(x, y)                                         \
  do {                                                                        \
    if ((x) < (y)) {                                                          \
      micro_test::reporter->Report(#x " >= " #y " failed at %s:%d", __FILE__, \
                                   __LINE__);                                 \
      micro_test::did_test_fail = true;                                       \
    }                                                                         \
  } while (false)

#define TF_LITE_MICRO_EXPECT_LE(x, y)                                         \
  do {                                                                        \
    if ((x) > (y)) {                                                          \
      micro_test::reporter->Report(#x " <= " #y " failed at %s:%d", __FILE__, \
                                   __LINE__);                                 \
      micro_test::did_test_fail = true;                                       \
    }                                                                         \
  } while (false)

#define TF_LITE_MICRO_EXPECT_TRUE(x)                                   \
  do {                                                                 \
    if (!x) {                                                          \
      micro_test::reporter->Report(#x " was not true failed at %s:%d", \
                                   __FILE__, __LINE__);                \
      micro_test::did_test_fail = true;                                \
    }                                                                  \
  } while (false)

#define TF_LITE_MICRO_EXPECT_FALSE(x)                                   \
  do {                                                                  \
    if (x) {                                                            \
      micro_test::reporter->Report(#x " was not false failed at %s:%d", \
                                   __FILE__, __LINE__);                 \
      micro_test::did_test_fail = true;                                 \
    }                                                                   \
  } while (false)

#include "sine_model_data.cc"
// extern const unsigned char g_sine_model_data[];
// extern const int g_sine_model_data_len;





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



uint32_t version;
void setup() {
  Serial.begin(115200);


  Serial.println("*** check version");
  version = model->version(); 
  if (version != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
      "Model provided is schema version %d not equal "
      "to supported version %d.\n",
      model->version(), TFLITE_SCHEMA_VERSION);
      version = -1;
  }

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter.AllocateTensors();


  // Obtain a pointer to the model's input tensor
  input = interpreter.input(0);

  // Make sure the input has the properties we expect
  TF_LITE_MICRO_EXPECT_NE(nullptr, input);
  // The property "dims" tells us the tensor's shape. It has one element for
  // each dimension. Our input is a 2D tensor containing 1 element, so "dims"
  // should have size 2.
  TF_LITE_MICRO_EXPECT_EQ(2, input->dims->size);
  // The value of each element gives the length of the corresponding tensor.
  // We should expect two single element tensors (one is contained within the
  // other).
  TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
  TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
  // The input is a 32 bit floating point value
  TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, input->type);
}

void loop() {
  delay(1000);
  Serial.println("===");
  Serial.print(". VERSION:");
  Serial.println(version);
  Serial.print(". MODEL DESC:");
  Serial.println(model->description()->c_str());

  float in = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

  // Provide an input value
  input->data.f[0] = in;

  // Run the model on this input and check that it succeeds
  TfLiteStatus invoke_status = interpreter.Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.print("XXX INVOKE FAILED XXX");
    error_reporter->Report("Invoke failed\n");
  }
  TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, invoke_status);

  TfLiteTensor* output = interpreter.output(0);
  TF_LITE_MICRO_EXPECT_EQ(2, output->dims->size);
  TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
  TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
  TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, output->type);

  // Obtain the output value from the tensor
  float out = output->data.f[0];
  Serial.print(". INFERENCE: ");
  Serial.print(in);
  Serial.print(" -> ");
  Serial.println(out);
}
