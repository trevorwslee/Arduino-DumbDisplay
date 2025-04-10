#include <Arduino.h>
#include "dumbdisplay.h"
#include "_dd_misc.h"





DDAutoPinGroupHeader DDAutoPinStackedGroupHeader(int left, int top, int right, int bottom) {
  return DDAutoPinGroupHeader{String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom)};
}
DDAutoPinGroupHeader DDAutoPinGridGroupHeader(int columnCount, int rowCount, const String& align, bool sameColumnWidth, bool sameRowHeight) {
  String header = String("G") + String(columnCount) + "x" + String(rowCount);
  if (align.length() > 0 || sameColumnWidth || sameRowHeight) {
    header += ":" + align;
  }
  if (sameColumnWidth) {
    header += ";scw";
  }
  if (sameRowHeight) {
    header += ";srh";
  }
  return DDAutoPinGroupHeader{header};
}
DDAutoPinGroupHeader DDAutoPinGridCellHeader(int columnSpan, int rowSpan, const String& align) {
  String header = String("C");
  if (columnSpan > 1 || rowSpan > 1) {
    header += String(columnSpan) + "x" + String(rowSpan);
  }
  if (align.length() > 0) {
    header += ":" + align;
  }
  return DDAutoPinGroupHeader{header};
}
DDAutoPinGroupHeader DDAutoPinGridCellAlignHeader(const String& align, int columnSpan, int rowSpan) {
  return DDAutoPinGridCellHeader(columnSpan, rowSpan, align);
}
