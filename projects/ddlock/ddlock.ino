
//#include "pico.h"

#ifdef PICO_SDK_VERSION_MAJOR
  // GP8 => RX of HC06; GP9 => TX of HC06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  /* HC-06 connectivity */
  DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


LcdDDLayer *lcd;
GraphicalDDLayer *graphical;
SimpleToolDDTunnel *tunnel_unlocked;
SimpleToolDDTunnel *tunnel_locked;

void setup() {
  dumbdisplay.recordLayerSetupCommands();


  // create a LCD layer for display some info about the picture shown
  lcd = dumbdisplay.createLcdLayer(16, 2);
  lcd->bgPixelColor("darkgray");
  lcd->pixelColor("lightblue");
  lcd->border(2, "blue");
  lcd->writeCenteredLine("... ...");

  // create a graphical layer for drawing the web images to
  graphical = dumbdisplay.createGraphicalLayer(200, 300);
  graphical->padding(0);
  graphical->margin(2);
  graphical->border(2, "darkgreen");
  graphical->enableFeedback();

  // auto "pin" the two layers vertically, one above the other
  dumbdisplay.configAutoPin(DD_AP_VERT);

  dumbdisplay.playbackLayerSetupCommands("rc-lock");


  // create tunnels for downloading web images ... and save to your phone
  tunnel_unlocked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", "lock-unlocked.png");
  tunnel_locked = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", "lock-locked.png");
}


bool locked = false;

bool allReady = false;
long lastShownMillis = 0;

DDConnectVersionTracker cvTracker;

void loop() {
  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int lockedResult = tunnel_locked->checkResult();
  int unlockedResult = tunnel_unlocked->checkResult();
  const DDFeedback* feedback = graphical->getFeedback();
 
  bool refresh = false;

  if (!allReady) {
    // all all ready ... if not refresh for a while (1000 ms), refresh
    long now = millis();
    long diff = now - lastShownMillis;
    if (diff > 1000) {
      locked = !locked;
      refresh = true;
    }
  } else {
    // if all ready ... see if double clicked ... if so, refresh
    if (feedback != NULL) {
      if (feedback->type == DOUBLECLICK) {
        locked = !locked;
        refresh = true;
      }
    }
  }

  if (!refresh) {
    // check if reconnected ... if so, refresh
    if (cvTracker.checkChanged(dumbdisplay)) {
      refresh = true;
    } 
  }
  
  if (!refresh) {
    return;
  }


  allReady = lockedResult == 1 && unlockedResult == 1;

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
