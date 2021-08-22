#include "ssdumbdisplay.h"

#define USE_BLUETOOTH

DumbDisplay dumbdisplay(new DDInputOutput(9600));

GraphicalDDLayer *pLayer;
BasicDDTunnel *pTunnel;

bool gettingNewQuoto = true;

void setup() {
    pLayer = dumbdisplay.createGraphicalLayer(200, 150);
    pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17");

    pLayer->border(10, "azure", "round");
    pLayer->noBackgroundColor();
    pLayer->penColor("teal");
    pLayer->setTextWrap(true);
}

void loop() {
    if (pTunnel->eof()) {
      //dumbdisplay.writeComment("getting another one");
      pLayer->backgroundColor("azure");
      pLayer->enableFeedback("f");
      //DDDelay(5000);
      while (true) {
        if (pLayer->getFeedback() != NULL) {
          break;
        }
      }
      pLayer->noBackgroundColor();
      pLayer->disableFeedback();
      pTunnel->reconnect();
      gettingNewQuoto = true;
    } else {
      if (pTunnel->count() > 0) {
        if (gettingNewQuoto) {
          pLayer->clear();
          pLayer->setCursor(0, 10);
        }
        //String data = pTunnel->readLineDirect();
        String data;
        pTunnel->readLine(data);
        //dumbdisplay.writeComment("{" + data + "}");
        pLayer->print(data);
        gettingNewQuoto = false;
      }
    }
    DDDelay(500);
}
