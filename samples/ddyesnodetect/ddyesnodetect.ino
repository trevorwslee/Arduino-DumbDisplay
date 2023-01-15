
#include "dumbdisplay.h"



#if defined (ESP32)

#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("BT32"));

#else
// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));

#endif


const char* YesWavFileName = "voice_yes.wav";
const char* NoWavFileName = "voice_no.wav";
const char* WellWavFileName = "voice_well.wav";


// declare "YES" / "NO" lcd layers, acting as buttons ... they will be created in setup block
LcdDDLayer* voiceYesLayer;
LcdDDLayer* voiceNoLayer;
LcdDDLayer* voiceWellLayer;
LcdDDLayer* detectYesLayer;
LcdDDLayer* detectNoLayer;
LcdDDLayer* detectWellLayer;

// declears "status" lcd layer ... it will be created in setup block
LcdDDLayer* statusLayer;

// "tunnel" to send detect request api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAuthorization = WIT_AUTHORIZATION;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");

void setup() {
  // create "YES" lcd layer, acting as a button
  voiceYesLayer = dumbdisplay.createLcdLayer(16, 3);
  voiceYesLayer->writeCenteredLine("Yes", 1);
  voiceYesLayer->border(3, "green", "round");
  voiceYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "NO" lcd layer, acting as a button
  voiceNoLayer = dumbdisplay.createLcdLayer(16, 3);
  voiceNoLayer->writeCenteredLine("No", 1);
  voiceNoLayer->border(3, "green", "round");
  voiceNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "WELL" lcd layer, acting as a button
  voiceWellLayer = dumbdisplay.createLcdLayer(16, 3);
  voiceWellLayer->writeCenteredLine("Well", 1);
  voiceWellLayer->border(3, "red", "round");
  voiceWellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "YES" lcd layer, acting as a button
  detectYesLayer = dumbdisplay.createLcdLayer(16, 3);
  detectYesLayer->writeCenteredLine("Detect Yes", 1);
  detectYesLayer->border(3, "green", "round");
  detectYesLayer->backgroundColor("yellow");
  detectYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "NO" lcd layer, acting as a button
  detectNoLayer = dumbdisplay.createLcdLayer(16, 3);
  detectNoLayer->writeCenteredLine("Detect No", 1);
  detectNoLayer->border(3, "green", "round");
  detectNoLayer->backgroundColor("yellow");
  detectNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "WELL" lcd layer, acting as a button
  detectWellLayer = dumbdisplay.createLcdLayer(16, 3);
  detectWellLayer->writeCenteredLine("Detect Well", 1);
  detectWellLayer->border(3, "red", "round");
  detectWellLayer->backgroundColor("yellow");
  detectWellLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "status" lcd layer
  statusLayer = dumbdisplay.createLcdLayer(16, 1);
  statusLayer->pixelColor("darkblue");
  statusLayer->border(3, "blue");
  statusLayer->backgroundColor("white");

  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAuthorization);
  witEndpoint.addHeader("Content-Type", "audio/wav");
  witEndpoint.addParam("text");

  DDAutoPinConfigBuilder<1> autoPinBuilder('V');
  autoPinBuilder.
    beginGroup('H').
      addLayer(voiceYesLayer).
      addLayer(voiceNoLayer).
      addLayer(voiceWellLayer).
    endGroup().
    beginGroup('H').
      addLayer(detectYesLayer).
      addLayer(detectNoLayer).
      addLayer(detectWellLayer).
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
  }
  if (detectSound != NULL) {
    witEndpoint.resetSoundAttachment(detectSound);
    witTunnel->reconnectToEndpoint(witEndpoint);
    //statusLayer->clear();
    statusLayer->writeCenteredLine("detecting");
    String detected = "";
    while (!witTunnel->eof()) {
      String fieldId;
      String fieldValue;
      if (witTunnel->read(fieldId, fieldValue)) {
        dumbdisplay.writeComment(fieldValue);
        detected = fieldValue;
        statusLayer->writeCenteredLine(String("detected") + " [" + detected + "]");
      }
    }
    detectSound = NULL;
    if (detected == "Yes") {
      detectSound = YesWavFileName;
    } else if (detected == "No") {
      detectSound = NoWavFileName;
    }
  
    if (detectSound == NULL) {
      statusLayer->writeCenteredLine("Not Yes/No!");
      dumbdisplay.tone(800, 100);
    } else {
      statusLayer->writeCenteredLine(String("detected") + " " + detected + "!");
      dumbdisplay.tone(2000, 100);
      delay(200);
      dumbdisplay.playSound(detectSound);
    }
    return;
  }

  String status = "";
  if (voiceYesLayer->getFeedback()) {
    // play the pre-installed "YES" WAV file
    dumbdisplay.playSound(YesWavFileName);
    status = "sounded YES";
  } else if (voiceNoLayer->getFeedback()) {
    // play the pre-installed "NO" WAV file
    dumbdisplay.playSound(NoWavFileName);
    status = "sounded NO";
  } else if (voiceWellLayer->getFeedback()) {
    // play the pre-installed "WELL" WAV file
    dumbdisplay.playSound(WellWavFileName);
    status = "sounded WELL";
  }
  if (status != "") {
    //statusLayer->clear();
    statusLayer->writeCenteredLine(status);
  }
}


