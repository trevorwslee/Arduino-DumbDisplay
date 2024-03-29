#ifndef _dd_util_h
#define _dd_util_h


void DDDebugOnly(int32_t i);


char* _DDEncodeInt(int32_t i, char* buffer, int bufferLen);

/// Class for internal use to encoding `int` parameters
class DDIntEncoder {
  public:
    DDIntEncoder(int32_t i): str(_DDEncodeInt(i, buffer, 20)) {
    }
    inline const String& encoded() { return str; }
  private:
    char buffer[21];
    String str;
};



#endif