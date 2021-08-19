#include <dumbdisplay.h>


/* please use DumbDisplayWiFiBridge for connection */
DumbDisplay dumbdisplay(new DDInputOutput(57600));

MbDDLayer *mb;
int heading;

void setup() {
  // create Micro:bit layer
  mb = dumbdisplay.createMicrobitLayer();
  // set background color
  mb->backgroundColor(DD_HEX_COLOR(0xF4A460));
}

void loop() {
  // set LED color
  String ledColor = DD_RGB_COLOR(128, 15 * heading, 255);
  mb->ledColor(ledColor);

  // show arrow
  MbArrow arrow = static_cast<MbArrow>(heading);  
  mb->showArrow(arrow);

  heading++;
  if (heading == 8)
    heading = 0;
  delay(1000);
}

