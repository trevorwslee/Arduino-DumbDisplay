#ifndef _dd_util_h
#define _dd_util_h

#define DD_CONNECT_FOR_GET_DATE_TIME "now:yyyy-MM-dd-HH-mm-ss-Z"
class DDDateTime {
  public:
    DDDateTime(int year =  0, int month =  0, int day =  0, int hour =  0, int minute =  0, int second =  0): year(year), month(month), day(day), hour(hour), minute(minute), second(second)   {
    }
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};
void DDParseGetDataTimeResponse(const String& response, DDDateTime& dateTime, int* pTZMins = NULL);

#if defined(ESP32)
void Esp32SetDateTime(const DDDateTime& dateTime, int tz_minuteswest = 0, int tz_dsttime = 0);
bool Esp32GetDateTime(DDDateTime& dateTime);
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