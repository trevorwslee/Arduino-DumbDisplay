#include "ssdumbdisplay.h"

//#define BLUETOOTH

#ifdef BLUETOOTH

// assume HC-06 connected, to pin 2 and 3; and assume it is using baud 9600
DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 9600));

#else

//otherwise, can use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8
DumbDisplay dumbdisplay(new DDInputOutput(57600));

#endif

SevenSegmentRowDDLayer *sevenSeg;
void setup() {
  // start recording the commands to setup DD (app side)
  dumbdisplay.recordLayerSetupCommands();

  // create a 7-seg layer for 4 digits
  sevenSeg = dumbdisplay.create7SegmentRowLayer(4);
  sevenSeg->border(15, "darkblue", "round");
  sevenSeg->padding(10);
  sevenSeg->resetSegmentOffColor(DD_HEX_COLOR(0xeeddcc));

  // stop recording and play back the recorded commands
  // more importantly, a "id" is given so that
  // the records commands can be reused during restart of DD app 
  dumbdisplay.playbackLayerSetupCommands("up4howlong");
}

long startMillis = millis();
void loop() {
  long howLong = (millis() - startMillis) / 1000;
  sevenSeg->showNumber(howLong);
  delay(1000);
}

