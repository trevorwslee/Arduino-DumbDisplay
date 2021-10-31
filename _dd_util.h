// /* log line to serial making sure not affect DD */
// void DDLogToSerial(const String& logLine);
/* the same usage as standard delay(), but it allows DD chances to handle feedback */
void DDDelay(unsigned long ms);
/* give DD a chance to handle feedback */
void DDYield();

void DDDebugOnly(int i);



char* _DDEncodeInt(int i, char* buffer, int bufferLen);

class DDIntEncoder {
  public:
    DDIntEncoder(int i): str(_DDEncodeInt(i, buffer, 20)) {
    }
    const String& encoded() { return str; }
  private:
    char buffer[20];
    String str;
};
