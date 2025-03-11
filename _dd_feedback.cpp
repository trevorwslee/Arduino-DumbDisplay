#include "Arduino.h"
#include "_dd_feedback.h"
#include "__dd_cpp_include.h"

DDFeedbackManager::DDFeedbackManager(/*int8_t bufferSize*/) {
  this->nextArrayIdx = 0;
  this->validArrayIdx = 0;
}
const DDFeedback* DDFeedbackManager::getFeedback() {
#ifdef ENABLE_FEEDBACK
  if (nextArrayIdx == validArrayIdx) return NULL;
  const DDFeedback* pFeedback = &feedbackArray[validArrayIdx];
  validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
  return pFeedback;
#else
  return NULL;
#endif    
}
void DDFeedbackManager::pushFeedback(DDFeedbackType type, int16_t x, int16_t y, const char* pText) {
#ifdef ENABLE_FEEDBACK
  feedbackArray[nextArrayIdx].type = type;
  feedbackArray[nextArrayIdx].x = x;
  feedbackArray[nextArrayIdx].y = y;
  feedbackArray[nextArrayIdx].text = pText != NULL ? pText : "";
  nextArrayIdx = (nextArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
  if (nextArrayIdx == validArrayIdx)
    validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
#endif
}
