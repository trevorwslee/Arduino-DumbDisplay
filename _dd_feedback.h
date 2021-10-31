struct DDFeedback {
  int x;
  int y;
#ifdef DD_SUPPORT_FEEDBACK_TEXT
  String text;
#endif
};


class DDFeedbackManager {
  public: 
    DDFeedbackManager(int bufferSize);
    ~DDFeedbackManager();
    const DDFeedback* getFeedback();
#ifdef DD_SUPPORT_FEEDBACK_TEXT
    void pushFeedback(int x, int y, const String& text);
#else
    void pushFeedback(int x, int y);
#endif
  private:
    DDFeedback* feedbackArray;
    int arraySize;
    int nextArrayIdx;
    int validArrayIdx;
};

enum DDFeedbackType { CLICK, DOUBLECLICK, LONGPRESS };

