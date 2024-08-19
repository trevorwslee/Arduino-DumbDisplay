
#include "Arduino.h"


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

void ParseGeneralServiceGetDataTimeResponse(const String& response, int& year, int& month, int& day, int& hour, int& minute, int& second) {
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
    String ss = res.substring(idx + 1);
    year = yyyy.toInt();
    month = MM.toInt();
    day = dd.toInt();
    hour = hh.toInt();
    minute = mm.toInt();
    second = ss.toInt();
}


#if defined(ESP32)
void Esp32SetDateTime(int year, int month, int day, int hour, int minute, int second) {
  struct tm timeinfo;
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = second;
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
  settimeofday(&tv, NULL);  
}
bool Esp32GetDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return false;
  }
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
  return true;
}
#endif

