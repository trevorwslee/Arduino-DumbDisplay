#include "dumbdisplay.h"


/* for connection, please use DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
/* use a lower baud rate since Arduino Uno Serial buffer size is not very big                      */
DumbDisplay dumbdisplay(new DDInputOutput(9600));

GraphicalDDLayer *pLayer;
BasicDDTunnel *pTunnel;

bool isGettingQuote = true;

void setup() {
  // setup a "graphial" list 
  pLayer = dumbdisplay.createGraphicalLayer(200, 150);  // size 200x150
  pLayer->border(10, "azure", "round");                 // a round border of size 10  
  pLayer->noBackgroundColor();                          // initial no background color
  pLayer->penColor("navy");                             // set pen color

  // setup a "tunnel" to access "quote of the day" by djxmmx.net
  pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17");  
}

void loop() {
    if (!pTunnel->eof()) {
      // not "reached" EOF (end-of-file)
      if (pTunnel->count() > 0) {
        // got something to read
        if (isGettingQuote) {
          // if just started to get the quote, reset something
          pLayer->clear();           // clear the "graphical" layer
          pLayer->setCursor(0, 10);  // set "cursor" to (0, 10)
        }
        String data = pTunnel->readLine();  // read what got so far
        pLayer->print(data);                // print out to the "graphical" layer what got so far
        isGettingQuote = false; 
      } else {
        pLayer->print(".");
      }
    } else {
      // "reached" EOF (i.e. got everything)
      // setup layer for getting "feedback" 
      pLayer->backgroundColor("azure");  // set background color
      pLayer->enableFeedback("f");       // enable "auto feedback" 
      while (true) {                     // loop and wait for layer clicked
        if (pLayer->getFeedback() != NULL) {
          break;
        }
      }
      // clicked ==> reset
      pLayer->noBackgroundColor();  // no background color 
      pLayer->disableFeedback();    // disable "feedback"
      pTunnel->reconnect();         // reconnect to djxmmx.net to get another quote
      isGettingQuote = true;       // indicating that a new quote is coming
      pLayer->setCursor(0, 0);  // set "cursor" to (0, 10)
    }
    //dumbdisplay.writeComment("<");
    DDDelay(500);  // delay a bit before another round; this also give DumbDisplay libary a chance to so its work
    //dumbdisplay.writeComment(">");
}
