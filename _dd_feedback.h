#ifndef _dd_feedback_h
#define _dd_feedback_h

#define DD_FEEDBACK_BUFFER_SIZE 4

enum DDFeedbackType { CLICK, LONGPRESS, DOUBLECLICK, MOVE };  // DOUBLECLICK needs special option for DumbDisplay


/// The struct for "feedback"
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

class DDFeedbackManager {
  public: 
    DDFeedbackManager(int8_t bufferSize);
    const DDFeedback* getFeedback();
    void pushFeedback(DDFeedbackType type, int16_t x, int16_t y, const char* pText);
  private:
    DDFeedback feedbackArray[DD_FEEDBACK_BUFFER_SIZE];
    int8_t nextArrayIdx;
    int8_t validArrayIdx;
};


#endif