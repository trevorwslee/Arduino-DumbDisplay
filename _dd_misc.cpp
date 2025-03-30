#include <Arduino.h>
#include "dumbdisplay.h"
#include "_dd_misc.h"


DDAutoPinGroupHeader DDAutoPinStackedGroupHeader(int left, int top, int right, int bottom) {
  return DDAutoPinGroupHeader{String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom)};
}
