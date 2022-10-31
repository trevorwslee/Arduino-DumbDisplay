
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
  lcd = dumbdisplay.createLcdLayer(16, 1);
  lcd->bgPixelColor("darkgray");
  lcd->pixelColor("lightblue");
  lcd->border(2, "blue");

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

long nextMillis = 0;
bool locked = false;
bool blinking = true;

void loop() {
  const DDFeedback* feedback = graphical->getFeedback();
  if (feedback != NULL) {
    if (feedback->type == DOUBLECLICK) {
      blinking = !blinking;
    }
  }
  long nowMillis = millis();
  long diffMillis =  nextMillis - nowMillis;
  if (diffMillis > 0) {
    return;
  }
  nextMillis = nowMillis + 1000;

  // get result whether web image downloaded .. 0: downloading; 1: downloaded ok; -1: failed to download 
  int result_unlocked = tunnel_unlocked->checkResult();
  int result_locked = tunnel_locked->checkResult();

  int result;
  const char* image_file_name;
  if (locked) {
    image_file_name = "lock-locked.png";
    result = result_locked;
  } else {
    image_file_name = "lock-unlocked.png";
    result = result_unlocked;
  }
  if (result == 1) {
    graphical->drawImageFile(image_file_name);
    if (blinking) {
      lcd->writeCenteredLine(locked ? "Lock locked" : "Lock unlocked");
    } else {
      lcd->writeCenteredLine("paused");
    }
  } else if (result == 0) {
    // downloading
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("... ...");
    graphical->println(image_file_name);
    graphical->println("... ...");
    lcd->writeCenteredLine("...");
  } else if (result == -1) {
    graphical->clear();
    graphical->setCursor(0, 10);
    graphical->println("XXX failed to download XXX");
    lcd->writeCenteredLine("failed");
  }
  if (blinking) {
    locked = !locked;
  }
}
