#include "dumbdisplay.h"


/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
DumbDisplay dumbdisplay(new DDInputOutput(57600));


SevenSegmentRowDDLayer *sevenSeg;


void setup() {
   sevenSeg = dumbdisplay.create7SegmentRowLayer(1);
}

void loop() {

   String segs[] = { "g", "f", "a", "b", "e", "d", "c", "." };
   for (int i = 0; i < 8; i++) {
      sevenSeg->turnOn(segs[i]);
      delay(1000);
   }

   char c = '0';
   while (1) {
      if (c == '.') {
         sevenSeg->setOn(".");
         c = '?';
      } else if (c == '?') {
         sevenSeg->showFormatted("E");
         c = '0';
      } else {   
         sevenSeg->showNumber(c - '0');
         if (c == '9')
            c = '.';
         else    
            c = c + 1;         
      }
      delay(1000);
   }
}
