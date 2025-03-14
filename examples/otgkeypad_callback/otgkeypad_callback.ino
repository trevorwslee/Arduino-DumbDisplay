/**
 * to run and see the result of this sketch, you will need two addition things:
 * . you will need to install Android DumbDisplay app from Play store
 *   https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay
 * . although there are several ways for microcontroller board to establish connection
 *   with DumbDisplay app, here, the simple OTG USB connection is assume;
 *   hence, you will need an OTG adaptor cable for connecting your microcontroller board
 *   to your Android phone
 * . after uploading the sketch to your microcontroller board, plug the USB cable
 *   to the OTG adaptor connected to your Android phone
 * . open the DumbDisplay app and make connection to your microcontroller board via the USB cable;
 *   hopefully, the UI is obvious enough :)
 * . for more details on DumbDisplay Arduino Library, please refer to
 *   https://github.com/trevorwslee/Arduino-DumbDisplay#readme
 * there is a related post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

const char Keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

LcdDDLayer *keyLayers[4][3];


// to be defined later in the sketch
LcdDDLayer *CreateKeyLayer(int r, int c);


void setup()
{
  // loop through the keys and create the corresponding LCD layer [for each on of the key]
  for (int r = 0; r < 4; r++)
  {
    for (int c = 0; c < 3; c++)
    {
      LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
      keyLayers[r][c] = keyLayer;
    }
  }
  // auto "pin" the key LCD layers int the desired way -- 4 rows by 3 columns
  dumbdisplay.configAutoPin(DD_AP_VERT_4(
      DD_AP_HORI_3(keyLayers[0][0]->getLayerId(), keyLayers[0][1]->getLayerId(), keyLayers[0][2]->getLayerId()),
      DD_AP_HORI_3(keyLayers[1][0]->getLayerId(), keyLayers[1][1]->getLayerId(), keyLayers[1][2]->getLayerId()),
      DD_AP_HORI_3(keyLayers[2][0]->getLayerId(), keyLayers[2][1]->getLayerId(), keyLayers[2][2]->getLayerId()),
      DD_AP_HORI_3(keyLayers[3][0]->getLayerId(), keyLayers[3][1]->getLayerId(), keyLayers[3][2]->getLayerId())));
}

void loop()
{
  DDYield(); // need to call this so that DumbDisplay lib can check for "feedback"
}

// "feedback" handler
void FeedbackHandler(DDLayer *pLayer, DDFeedbackType type, const DDFeedback &feedback)
{
  if (type == DDFeedbackType::CLICK)
  {
    char key = pLayer->customData.charAt(0);
    dumbdisplay.writeComment("key [" + String(key) + "]");
  }
}

// create key LCD layer; key defined by r and c
LcdDDLayer *CreateKeyLayer(int r, int c)
{
  String key = String(Keys[r][c]);
  LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32);
  keyLayer->pixelColor("navy");
  keyLayer->border(5, "darkgray", "raised");
  keyLayer->padding(1);
  keyLayer->writeLine(key);
  keyLayer->setFeedbackHandler(FeedbackHandler, "fl");  // set the "feedback" handler
  keyLayer->customData = key;  // set the key to the "custom data" of the layer
  return keyLayer;
}
