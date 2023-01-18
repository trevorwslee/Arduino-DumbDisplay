
#if defined(ESP32)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("BT32", true, 115200));
#elif defined(PICO_SDK_VERSION_MAJOR)
  // assume Pico with Bluetooth
  // GP8 => RX of HC-06; GP9 => TX of HC-06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  DumbDisplay dumbdisplay(new DDPicoUart1IO(115200, true, 115200));
#else
  // create the DumbDisplay object; assuming USB connection with 115200 baud
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
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
  yesLayer->border(3, "green", "round");
  yesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "NO" lcd layer, acting as a button
  noLayer = dumbdisplay.createLcdLayer(16, 3);
  noLayer->writeCenteredLine("NO", 1);
  noLayer->border(3, "green", "round");
  noLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "WELL" lcd layer, acting as a button
  wellLayer = dumbdisplay.createLcdLayer(16, 3);
  wellLayer->writeCenteredLine("WELL", 1);
  wellLayer->border(3, "red", "round");
  wellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "bark" lcd layer, acting as a button
  barkLayer = dumbdisplay.createLcdLayer(16, 3);
  barkLayer->writeCenteredLine("bark", 1);
  barkLayer->border(3, "red", "round");
  barkLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "YES" lcd layer, acting as a button
  detectYesLayer = dumbdisplay.createLcdLayer(16, 3);
  detectYesLayer->writeCenteredLine("Detect YES", 1);
  detectYesLayer->border(3, "green", "round");
  detectYesLayer->backgroundColor("yellow");
  detectYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "NO" lcd layer, acting as a button
  detectNoLayer = dumbdisplay.createLcdLayer(16, 3);
  detectNoLayer->writeCenteredLine("Detect NO", 1);
  detectNoLayer->border(3, "green", "round");
  detectNoLayer->backgroundColor("yellow");
  detectNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "WELL" lcd layer, acting as a button
  detectWellLayer = dumbdisplay.createLcdLayer(16, 3);
  detectWellLayer->writeCenteredLine("Detect WELL", 1);
  detectWellLayer->border(3, "red", "round");
  detectWellLayer->backgroundColor("yellow");
  detectWellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "bark" lcd layer, acting as a button
  detectBarkLayer = dumbdisplay.createLcdLayer(16, 3);
  detectBarkLayer->writeCenteredLine("Detect bark", 1);
  detectBarkLayer->border(3, "red", "round");
  detectBarkLayer->backgroundColor("yellow");
  detectBarkLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "status" lcd layer
  statusLayer = dumbdisplay.createLcdLayer(18, 1);
  statusLayer->pixelColor("darkblue");
  statusLayer->border(3, "blue");
  statusLayer->backgroundColor("white");

  // create / setup "tunnel" etc to send detect request
  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addHeader("Content-Type", "audio/wav");
  witEndpoint.addParam("text");

  // auto pin the layers in the desired way
  DDAutoPinConfigBuilder<1> autoPinBuilder('V');
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


