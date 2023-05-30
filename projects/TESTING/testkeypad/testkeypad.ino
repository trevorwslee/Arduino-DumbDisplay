#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));

const int RowCount = 4;//4;
const int ColCount = 3;//3;

const char Keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

LcdDDLayer *keyLayers[RowCount][ColCount];


// to be defined later in the sketch
LcdDDLayer *CreateKeyLayer(int r, int c);
// to be defined later in the sketch
char CheckKeyPressed();


void setup() {
  // will only allow single click, which also makes clicking more responsive
  dumbdisplay.setFeedbackSingleClickOnly();
  
  // loop through the keys and create the corresponding LCD layer [for each on of the key]
  for (int r = 0; r < RowCount; r++) {
    for (int c = 0; c < ColCount; c++) {
      LcdDDLayer *keyLayer = CreateKeyLayer(r, c);
      keyLayers[r][c] = keyLayer;
    }
  }

  dumbdisplay.configAutoPin(DD_AP_VERT);

  // auto "pin" the key LCD layers int the desired way -- 4 rows by 3 columns
  // dumbdisplay.configAutoPin(DD_AP_VERT_4(
  //     DD_AP_HORI_3(keyLayers[0][0]->getLayerId(), keyLayers[0][1]->getLayerId(), keyLayers[0][2]->getLayerId()),
  //     DD_AP_HORI_3(keyLayers[1][0]->getLayerId(), keyLayers[1][1]->getLayerId(), keyLayers[1][2]->getLayerId()),
  //     DD_AP_HORI_3(keyLayers[2][0]->getLayerId(), keyLayers[2][1]->getLayerId(), keyLayers[2][2]->getLayerId()),
  //     DD_AP_HORI_3(keyLayers[3][0]->getLayerId(), keyLayers[3][1]->getLayerId(), keyLayers[3][2]->getLayerId())));
}
 
void loop() {
  char keyPressed = CheckKeyPressed();
  if (keyPressed != 0) {
    dumbdisplay.writeComment("key [" + String(keyPressed) + "]");
  }
}

// create key LCD layer; key defined by r and c
LcdDDLayer *CreateKeyLayer(int r, int c) {
  String key = String(Keys[r][c]);
  LcdDDLayer *keyLayer = dumbdisplay.createLcdLayer(1, 1, 32);
  keyLayer->pixelColor("navy");
  keyLayer->border(5, "darkgray", "raised");
  keyLayer->padding(1);
  keyLayer->writeLine(key);
  keyLayer->enableFeedback("fl");
  keyLayer->customData = key;  // set the key to the "custom data" of the layer
  return keyLayer;
}

// check if any key is pressed; return the key pressed, or 0 if none
char CheckKeyPressed() {
  // loop through the key LCD layers and check if any one has "feedback" (i.e. pressed)
  for (int r = 0; r < RowCount; r++) {
    for (int c = 0; c < ColCount; c++) {
      LcdDDLayer* keyLayer = keyLayers[r][c];
      const DDFeedback* feedback = keyLayer->getFeedback();
      if (feedback != NULL) {
        char keyPressed = keyLayer->customData.charAt(0);
        return keyPressed;
      }
    }
  }
  return 0;
}

