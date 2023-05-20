
#include "Arduino.h"


extern boolean _DDDisableParamEncoding;

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
