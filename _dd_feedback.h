#ifndef _dd_feedback_h
#define _dd_feedback_h

#define DD_FEEDBACK_BUFFER_SIZE 4

enum DDFeedbackType { CLICK, DOUBLECLICK, LONGPRESS };


struct DDFeedback {
  DDFeedbackType type;
  int16_t x;
  int16_t y;
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