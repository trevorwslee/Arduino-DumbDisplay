

#if defined(ESP32)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("BT32", true, 115200));
#elif defined(PICO_SDK_VERSION_MAJOR)
  // assume Pico with Bluetooth
  // GP8 => RX of HC-06; GP9 => TX of HC-06
  #define DD_4_PICO_TX 8
  #define DD_4_PICO_RX 9
  #include "picodumbdisplay.h"
  DumbDisplay dumbdisplay(new DDPicoSerialIO(DD_4_PICO_TX, DD_4_PICO_RX));
#else
  // create the DumbDisplay object; assuming USB connection with 115200 baud
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(115200));
#endif


const char* YesWavFileName = "voice_yes.wav";
const char* BarkWavFileName = "sound_bark.wav";


// declare "YES" (etc) lcd layers, acting as buttons ... they will be created in setup block
LcdDDLayer* yesLayer;
LcdDDLayer* barkLayer;
LcdDDLayer* detectYesLayer;
LcdDDLayer* detectBarkLayer;

// declears "status" lcd layer ... it will be created in setup block
LcdDDLayer* statusLayer;

// declare "tunnel" etc to send detect request to api.wit.ai ... please get "access token" from api.wit.ai
JsonDDTunnel* witTunnel;
const String witAccessToken = WIT_ACCESS_TOKEN;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");

void setup() {
  // create "YES" lcd layer, acting as a button
  yesLayer = dumbdisplay.createLcdLayer(16, 3);
  yesLayer->writeCenteredLine("YES", 1);
  yesLayer->border(3, "green", "round");
  yesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

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
  witEndpoint.addHeader("Content-Type", "audio/wav");
  //witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addParam("text");

  // auto pin the layers in the desired way
  DDAutoPinConfig autoPinBuilder('V');
  autoPinBuilder.
    beginGroup('H').
      addLayer(yesLayer).
      addLayer(barkLayer).
    endGroup().
    beginGroup('H').
      addLayer(detectYesLayer).
      addLayer(detectBarkLayer).
    endGroup().
    addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinBuilder.build());

  if (false) {
    dumbdisplay.writeComment(witEndpoint.headers);
    dumbdisplay.writeComment(witEndpoint.params);
    dumbdisplay.writeComment("ready");
  }
}

void loop() {

  const char* detectSound = NULL;
  if (detectYesLayer->getFeedback()) {
    // detect "YES"
    detectSound = YesWavFileName;
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
  } else if (barkLayer->getFeedback()) {
    // play the pre-installed "bark" WAV file
    dumbdisplay.playSound(BarkWavFileName);
    status = "sounded bark";
  }
  if (status != "") {
    statusLayer->writeCenteredLine(status);
  }
}


