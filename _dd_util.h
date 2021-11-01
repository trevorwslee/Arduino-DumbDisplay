#ifndef _dd_util_h
#define _dd_util_h


// /* log line to serial making sure not affect DD */
// void DDLogToSerial(const String& logLine);
/* the same usage as standard delay(), but it allows DD chances to handle feedback */
void DDDelay(unsigned long ms);
/* give DD a chance to handle feedback */
void DDYield();

void DDDebugOnly(int32_t i);



char* _DDEncodeInt(int i, char* buffer, int bufferLen);

class DDIntEncoder {
  public:
    DDIntEncoder(int32_t i): str(_DDEncodeInt(i, buffer, 20)) {
    }
    const String& encoded() { return str; }
  private:
    char buffer[21];
    String str;
};

#endif