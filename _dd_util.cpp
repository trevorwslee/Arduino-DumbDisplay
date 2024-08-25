
#include "Arduino.h"
#include "_dd_util.h"

extern bool _DDDisableParamEncoding;

char* _DDEncodeInt(int32_t i, char* buffer, int bufferLen) {
  if (_DDDisableParamEncoding) {
    return itoa(i, buffer, 10);
  }
  bool isNeg;
  if (i < 0) {
    isNeg = true;
    i = -i;
  } else {
    isNeg = false;
  }
  char *encoded = buffer + (isNeg ? 1 : 0);
  if (i <= 78) {
    encoded[0] = i + '0'; 
    encoded[1] = 0;
  } else if (i <= 99) {
    encoded[0] = (i / 10) + '0';
    encoded[1] = (i % 10) + '0';
    encoded[2] = 0;
  } else {
    encoded[bufferLen - 1] = 0;
    int idx = bufferLen - 2;
    while (idx >= 0 && i > 0) {
      int r = i % 78;
      i = i / 78;
      int c = r + '0';
      if (i == 0) {
        c += 10;
      }
      encoded[idx--] = c;
    }
    encoded += idx + 1;
  }
  if (isNeg) {
    encoded--;
    encoded[0] = '-';
  } 
  return encoded;
}

void DDParseGetDataTimeResponse(const String& response, DDDateTime& dateTime, int* pTZMins) {
    String res = response;
    int idx = res.indexOf('-');
    String yyyy = res.substring(0, idx);
    res = res.substring(idx + 1);
    idx = res.indexOf('-');
    String MM = res.substring(0, idx);
    res = res.substring(idx + 1);
    idx = res.indexOf('-');
    String dd = res.substring(0, idx);
    res = res.substring(idx + 1);
    idx = res.indexOf('-');
    String hh = res.substring(0, idx);
    res = res.substring(idx + 1);
    idx = res.indexOf('-');
    String mm = res.substring(0, idx);
    res = res.substring(idx + 1);
    idx = res.indexOf('-');
    String ss = res.substring(0, idx);
    String tz = res.substring(idx + 1);
    int tz_int = tz.toInt();
    int tz_mins = 60 * (tz_int / 100) + (tz_int % 100);
    dateTime.year = yyyy.toInt();
    dateTime.month = MM.toInt();
    dateTime.day = dd.toInt();
    dateTime.hour = hh.toInt();
    dateTime.minute = mm.toInt();
    dateTime.second = ss.toInt();
    if (pTZMins != NULL) {
      *pTZMins = tz_mins;
    }
}


#if defined(ESP32)
void Esp32SetDateTime(const DDDateTime& dateTime, int tz_minuteswest, int tz_dsttime) {
  tm timeinfo;
  timeinfo.tm_year = dateTime.year - 1900;
  timeinfo.tm_mon = dateTime.month - 1;
  timeinfo.tm_mday = dateTime.day;
  timeinfo.tm_hour = dateTime.hour;
  timeinfo.tm_min = dateTime.minute;
  timeinfo.tm_sec = dateTime.second;
  time_t epoch = mktime(&timeinfo);
  int ms = 0;

  struct timeval tv;
  if (true) {
    tv.tv_sec = epoch;  // epoch time (seconds)
  } else {
    bool overflow;
    if (epoch > 2082758399){
      overflow = true;
      tv.tv_sec = epoch - 2082758399;  // epoch time (seconds)
    } else {
      overflow = false;
      tv.tv_sec = epoch;  // epoch time (seconds)
    }
  }
  tv.tv_usec = ms;
  timezone tz;
  tz.tz_minuteswest = tz_minuteswest;  
  tz.tz_dsttime = tz_dsttime;
  settimeofday(&tv, &tz);  
}
bool Esp32GetDateTime(DDDateTime& dateTime) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return false;
  }
  dateTime.year = timeinfo.tm_year + 1900;
  dateTime.month = timeinfo.tm_mon + 1;
  dateTime.day = timeinfo.tm_mday;
  dateTime.hour = timeinfo.tm_hour;
  dateTime.minute = timeinfo.tm_min;
  dateTime.second = timeinfo.tm_sec;
  return true;
}
#endif

