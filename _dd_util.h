#ifndef _dd_util_h
#define _dd_util_h

#define GENERAL_SERVICE_CONNECT_FOR_GET_DATE_TIME "now:yyyy-MM-dd-HH-mm-ss"
void ParseGeneralServiceGetDataTimeResponse(const String& response, int& year, int& month, int& day, int& hour, int& minute, int& second);

#if defined(ESP32)
void Esp32SetDateTime(int year, int month, int day, int hour, int minute, int second);
bool Esp32GetDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second);
#endif


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