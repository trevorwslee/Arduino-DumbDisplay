  #include <ssdumbdisplay.h>

  DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2,3), DUMBDISPLAY_BAUD, true));
  LedGridDDLayer *led;

  void setup() {
      // create a LED layer
      led = dumbdisplay.createLedGridLayer();
  }

  void loop() {
      led->toggle();
      delay(1000);
  }
