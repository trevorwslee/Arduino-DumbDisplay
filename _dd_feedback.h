#ifndef _dd_feedback_h
#define _dd_feedback_h


#ifndef DD_NO_FEEDBACK
  #ifndef DD_FEEDBACK_BUFFER_SIZE
    #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
      #define DD_FEEDBACK_BUFFER_SIZE 2
    #else
      #define DD_FEEDBACK_BUFFER_SIZE 4
    #endif
  #endif
  #if DD_FEEDBACK_BUFFER_SIZE < 2
    #error "DD_FEEDBACK_BUFFER_SIZE must be at least 2"
  #endif 
#else
  #define DD_FEEDBACK_BUFFER_SIZE 0
#endif


/// @struct DDFeedbackType
/// @brief
/// The enum indicating the type of "feedback". See DDFeedback
enum DDFeedbackType { CLICK, LONGPRESS, DOUBLECLICK, MOVE, UP, DOWN, CUSTOM };  // DOUBLECLICK needs special option for DumbDisplay

/// @struct DDFeedback
/// @brief
/// The struct that captures info about "feedback". See DDLayer::enableFeedback()
struct DDFeedback {
  /// type of the "feedback"
  DDFeedbackType type;
  /// x of the "area" on the layer where was clicked
  int16_t x;
  /// y of the "area" on the layer where was clicked
  int16_t y;
  /// text input (if any)
  String text;
};

/// Class for internal use to track "feedbacks".
class DDFeedbackManager {
  public: 
    DDFeedbackManager(/*int8_t bufferSize*/);
    const DDFeedback* getFeedback();
    void pushFeedback(DDFeedbackType type, int16_t x, int16_t y, const char* pText);
  private:
    DDFeedback feedbackArray[DD_FEEDBACK_BUFFER_SIZE];
    int8_t nextArrayIdx;
    int8_t validArrayIdx;
};


#endif