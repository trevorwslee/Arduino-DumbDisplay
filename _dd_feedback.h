#ifndef _dd_feedback_h
#define _dd_feedback_h

enum DDFeedbackType { CLICK, DOUBLECLICK, LONGPRESS };


struct DDFeedback {
  DDFeedbackType type;
  int x;
  int y;
  String text;
};

class DDFeedbackManager {
  public: 
    DDFeedbackManager(int bufferSize);
    ~DDFeedbackManager();
    const DDFeedback* getFeedback();
    void pushFeedback(DDFeedbackType type, int x, int y, const char* pText);
  private:
    DDFeedback* feedbackArray;
    int arraySize;
    int nextArrayIdx;
    int validArrayIdx;
};


#endif