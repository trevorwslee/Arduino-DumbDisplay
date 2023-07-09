#include <LilyGoWatch.h>

void setup() {
  // create the T-Watch API object 
  TTGOClass *ttgo = TTGOClass::getWatch();

  // init the T-Watch API object
  ttgo->begin();
  ttgo->openBL();

  // from the T-Watch API object, get the TFT_eSPI object
  TFT_eSPI& tft = *ttgo->tft;

  tft.fillScreen(TFT_LIGHTGREY);
}

void loop() {
  // get the T-Watch API object (only the first call will create) 
  TTGOClass *ttgo = TTGOClass::getWatch();

  // from the T-Watch API object, get the TFT_eSPI object
  TFT_eSPI& tft = *ttgo->tft;

  tft.fillCircle(120, 120, 30, TFT_PURPLE);
  delay(1000);                     
  tft.fillCircle(120, 120, 30, TFT_LIGHTGREY);
  delay(1000);                     
}