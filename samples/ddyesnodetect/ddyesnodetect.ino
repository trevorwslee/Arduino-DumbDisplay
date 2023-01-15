
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


// declare "YES" / "NO" lcd layers, acting as buttons ... they will be created in setup block
LcdDDLayer* voiceYesLayer;
LcdDDLayer* voiceNoLayer;
LcdDDLayer* detectYesLayer;
LcdDDLayer* detectNoLayer;

// declears "status" lcd layer ... it will be created in setup block
LcdDDLayer* statusLayer;

// "tunnel" to send detect request api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAuthorization = WIT_AUTHORIZATION;
//DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech?v=20230115");
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
  voiceNoLayer->border(3, "red", "round");
  voiceNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "YES" lcd layer, acting as a button
  detectYesLayer = dumbdisplay.createLcdLayer(16, 3);
  detectYesLayer->writeCenteredLine("Detect Yes", 1);
  detectYesLayer->border(3, "green", "round");
  detectYesLayer->backgroundColor("yellow");
  detectYesLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create detect "NO" lcd layer, acting as a button
  detectNoLayer = dumbdisplay.createLcdLayer(16, 3);
  detectNoLayer->writeCenteredLine("Detect No", 1);
  detectNoLayer->border(3, "red", "round");
  detectNoLayer->backgroundColor("yellow");
  detectNoLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "status" lcd layer
  statusLayer = dumbdisplay.createLcdLayer(16, 3);
  statusLayer->pixelColor("darkblue");
  statusLayer->border(3, "blue");
  statusLayer->backgroundColor("white");

  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAuthorization);
  witEndpoint.addHeader("Content-Type", "audio/wav");

  DDAutoPinConfigBuilder<1> autoPinBuilder('V');
  autoPinBuilder.
    beginGroup('H').
      addLayer(voiceYesLayer).
      addLayer(voiceNoLayer).
    endGroup().
    beginGroup('H').
      addLayer(detectYesLayer).
      addLayer(detectNoLayer).
    endGroup().
    addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinBuilder.build());
}

void loop() {

  const char* detectSound = NULL;
  if (detectYesLayer->getFeedback()) {
    detectSound = YesWavFileName;
  }

  const char* status = NULL;
  if (detectSound != NULL) {
    witEndpoint.resetAttachmentId(String("sound:") + detectSound);
    witTunnel->reconnectToEndpoint(witEndpoint);
    status = "detecting";
  } else {
    if (voiceYesLayer->getFeedback()) {
      // play the pre-installed "YES" WAV file
      dumbdisplay.playSound(YesWavFileName);
      status = "sounded YES";
    } else if (voiceNoLayer->getFeedback()) {
      // play the pre-installed "NO" WAV file
      dumbdisplay.playSound(NoWavFileName);
      status = "sounded NO";
    }
  }

  if (status != NULL) {
    statusLayer->clear();
    statusLayer->writeCenteredLine(status, 1);
  }
}


