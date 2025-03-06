
/**
 * this is an extension to otgwebimage example; so, you want to try that one first
 * . the web image is actually a lock, two web images are downloaded
 *   . one for unlocked
 *   . one for locked
 * . it has an additional LCD text on to show the status of the lock (locked or unlocked)
 * . after all download done, you will need to double click / tap the lock to toggle locked / unlocked 
 */ 


#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));


GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;

LcdDDLayer *lcd;

void setup() {
  // create a LCD layer for display some status
  lcd = dumbdisplay.createLcdLayer(16, 2);
  lcd->bgPixelColor("darkgray");
  lcd->pixelColor("lightblue");
  lcd->border(2, "blue");
  lcd->writeCenteredLine("... ...");

  // create a graphical layer for drawing the web images (the lock) to
  graphical = dumbdisplay.createGraphicalLayer(200, 300);
  graphical->padding(0);
  graphical->margin(2);
  graphical->border(2, "darkgreen");
  graphical->enableFeedback();

  // auto "pin" the two layers vertically, one above the other
  dumbdisplay.configAutoPin(DD_AP_VERT);

  // create tunnels for downloading web images (the lock) ... and save to your phone
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", "lock-unlocked.png");
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", "lock-locked.png");
}


bool locked = false;

bool allReady = false;
long lastShownMillis = 0;

void loop() {
  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int lockedResult = tunnel_locked->checkResult();
  int unlockedResult = tunnel_unlocked->checkResult();
  const DDFeedback* feedback = graphical->getFeedback();
 
  bool refresh = false;

  if (!allReady) {
    // not all ready ... if not refresh for a while (1000 ms), refresh
    long now = millis();
    long diff = now - lastShownMillis;
    if (diff > 1000) {
      locked = !locked;
      refresh = true;
      dumbdisplay.writeComment("... waiting ...");
    }
  } else {
    // if all ready ... see if double clicked ... if so, refresh
    if (feedback != NULL) {
      if (feedback->type == DDFeedbackType::DOUBLECLICK) {
        locked = !locked;
        refresh = true;
      }
    }
  }

  if (!refresh) {
    return;
  }

  if (!allReady) {
    allReady = lockedResult == 1 && unlockedResult == 1;
    if (allReady) {
      dumbdisplay.writeComment("... READY");
    }
  }

  int result;
  const char* imageFileName;
  if (locked) {
    imageFileName = "lock-locked.png";
    result = lockedResult;
  } else {
    imageFileName = "lock-unlocked.png";
    result = unlockedResult;
  }
  if (result == 1) {
    graphical->drawImageFile(imageFileName);
    lcd->writeCenteredLine(locked ? "Lock locked" : "Lock unlocked");
    lcd->writeCenteredLine(allReady ? "(READY)" : "...", 1);
  } else if (result == 0) {
    // downloading
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("... ...");
    graphical->println(imageFileName);
    graphical->println("... ...");
    lcd->writeCenteredLine("...");
  } else if (result == -1) {
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("XXX failed to download XXX");
    lcd->writeCenteredLine("failed");
  }

  lastShownMillis = millis();
}
