
// if want Bluetooth, uncomment the following line
// #define BLUETOOTH "ESP32BT"
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true));
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#else
  // for direct USB connection to phone
  // . via OTG -- see https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
  // . via DumbDisplayWifiBridge -- see https://www.youtube.com/watch?v=0UhRmXXBQi8/
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
#endif


const char* YesWavFileName = "voice_yes.wav";
const char* NoWavFileName = "voice_no.wav";
const char* WellWavFileName = "voice_well.wav";
const char* BarkWavFileName = "sound_bark.wav";


// declare "YES" (etc) lcd layers, acting as buttons ... they will be created in setup block
LcdDDLayer* yesLayer;
LcdDDLayer* noLayer;
LcdDDLayer* wellLayer;
LcdDDLayer* barkLayer;
LcdDDLayer* detectYesLayer;
LcdDDLayer* detectNoLayer;
LcdDDLayer* detectWellLayer;
LcdDDLayer* detectBarkLayer;

// declears "status" lcd layer ... it will be created in setup block
LcdDDLayer* statusLayer;

// declare "tunnel" etc to send detect request to api.wit.ai ... please get "access token" from api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAccessToken = WIT_ACCESS_TOKEN;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");

void setup() {
  // create "YES" lcd layer, acting as a button
  yesLayer = dumbdisplay.createLcdLayer(16, 3);
  yesLayer->writeCenteredLine("YES", 1);
  yesLayer->border(3, DD_COLOR_green, "round");
  yesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "NO" lcd layer, acting as a button
  noLayer = dumbdisplay.createLcdLayer(16, 3);
  noLayer->writeCenteredLine("NO", 1);
  noLayer->border(3, DD_COLOR_green, "round");
  noLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "WELL" lcd layer, acting as a button
  wellLayer = dumbdisplay.createLcdLayer(16, 3);
  wellLayer->writeCenteredLine("WELL", 1);
  wellLayer->border(3, DD_COLOR_red, "round");
  wellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "bark" lcd layer, acting as a button
  barkLayer = dumbdisplay.createLcdLayer(16, 3);
  barkLayer->writeCenteredLine("bark", 1);
  barkLayer->border(3, DD_COLOR_red, "round");
  barkLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "YES" lcd layer, acting as a button
  detectYesLayer = dumbdisplay.createLcdLayer(16, 3);
  detectYesLayer->writeCenteredLine("Detect YES", 1);
  detectYesLayer->border(3, DD_COLOR_green, "round");
  detectYesLayer->backgroundColor("yellow");
  detectYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "NO" lcd layer, acting as a button
  detectNoLayer = dumbdisplay.createLcdLayer(16, 3);
  detectNoLayer->writeCenteredLine("Detect NO", 1);
  detectNoLayer->border(3, DD_COLOR_green, "round");
  detectNoLayer->backgroundColor("yellow");
  detectNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "WELL" lcd layer, acting as a button
  detectWellLayer = dumbdisplay.createLcdLayer(16, 3);
  detectWellLayer->writeCenteredLine("Detect WELL", 1);
  detectWellLayer->border(3, DD_COLOR_red, "round");
  detectWellLayer->backgroundColor("yellow");
  detectWellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "bark" lcd layer, acting as a button
  detectBarkLayer = dumbdisplay.createLcdLayer(16, 3);
  detectBarkLayer->writeCenteredLine("Detect bark", 1);
  detectBarkLayer->border(3, DD_COLOR_red, "round");
  detectBarkLayer->backgroundColor("yellow");
  detectBarkLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "status" lcd layer
  statusLayer = dumbdisplay.createLcdLayer(18, 1);
  statusLayer->pixelColor(DD_COLOR_darkblue);
  statusLayer->border(3, DD_COLOR_blue);
  statusLayer->backgroundColor(DD_COLOR_white);

  // create / setup "tunnel" etc to send detect request
  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addHeader("Content-Type", "audio/wav");
  witEndpoint.addParam("text");

  // auto pin the layers in the desired way
  DDAutoPinConfig autoPinBuilder('V');
  autoPinBuilder.
    beginGroup('H').
      addLayer(yesLayer).
      addLayer(noLayer).
    endGroup().
    beginGroup('H').
      addLayer(wellLayer).
      addLayer(barkLayer).
    endGroup().
    beginGroup('H').
      addLayer(detectYesLayer).
      addLayer(detectNoLayer).
    endGroup().
    beginGroup('H').
      addLayer(detectWellLayer).
      addLayer(detectBarkLayer).
    endGroup().
    addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinBuilder.build());
}

void loop() {

  const char* detectSound = NULL;
  if (detectYesLayer->getFeedback()) {
    // detect "YES"
    detectSound = YesWavFileName;
  } else if (detectNoLayer->getFeedback()) {
    // detect "NO"
    detectSound = NoWavFileName;
  } else if (detectWellLayer->getFeedback()) {
    // detect "WELL"
    detectSound = WellWavFileName;
  } else if (detectBarkLayer->getFeedback()) {
    // detect "bark"
    detectSound = BarkWavFileName;
  }
  if (witTunnel != NULL) {
    if (detectSound != NULL) {
      witEndpoint.resetSoundAttachment(detectSound);
      witTunnel->reconnectToEndpoint(witEndpoint);
      statusLayer->writeCenteredLine("... detecting ...");
      String detected = "";
      while (!witTunnel->eof()) {
        String fieldId;
        String fieldValue;
        if (witTunnel->read(fieldId, fieldValue)) {
          if (fieldValue != "") {
            dumbdisplay.writeComment(fieldValue);
            detected = fieldValue;
            statusLayer->writeCenteredLine(String("... ") + " [" + detected + "] ...");
          }
        }
      }
      detectSound = NULL;
      if (detected == "Yes") {
        detectSound = YesWavFileName;
      } else if (detected == "No") {
        detectSound = NoWavFileName;
      }
    
      if (detectSound == NULL) {
        statusLayer->writeCenteredLine("Not YES/NO!");
        dumbdisplay.tone(800, 100);
      } else {
        statusLayer->writeCenteredLine(String("Detected") + " " + detected + "!");
        dumbdisplay.tone(2000, 100);
        delay(200);
        dumbdisplay.playSound(detectSound);
      } 
      return;
    }
  } else {
    if (detectSound != NULL) {
      statusLayer->writeCenteredLine("No tunnel!");
      return;
    }
  }

  String status = "";
  if (yesLayer->getFeedback()) {
    // play the pre-installed "YES" WAV file
    dumbdisplay.playSound(YesWavFileName);
    status = "sounded YES";
  } else if (noLayer->getFeedback()) {
    // play the pre-installed "NO" WAV file
    dumbdisplay.playSound(NoWavFileName);
    status = "sounded NO";
  } else if (wellLayer->getFeedback()) {
    // play the pre-installed "WELL" WAV file
    dumbdisplay.playSound(WellWavFileName);
    status = "sounded bark";
  } else if (barkLayer->getFeedback()) {
    // play the pre-installed "bark" WAV file
    dumbdisplay.playSound(BarkWavFileName);
    status = "sounded bark";
  }
  if (status != "") {
    statusLayer->writeCenteredLine(status);
  }
}


