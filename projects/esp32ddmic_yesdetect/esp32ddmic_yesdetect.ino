#define WITH_I2S_MIC


// ESP32 Bluetooth with name BT32
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));


#if defined(WITH_I2S_MIC)
  // I2S driver
  #include <driver/i2s.h>

  // INMP441 I2S pin assignment
  #define I2S_WS 25
  #define I2S_SD 33
  #define I2S_SCK 32
 
  // I2S processor
  #define I2S_PORT I2S_NUM_0

  const int I2S_DMA_BUF_COUNT = 8;
  const int I2S_DMA_BUF_LEN = 1024;

  const int SoundSampleRate = 8000;  // will be 16-bit per sample
  const int SoundNumChannels = 1;


  // 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
  const int StreamBufferNumBytes = 2048;//256;
  const int StreamBufferLen = StreamBufferNumBytes / 2;
  int16_t StreamBuffer[StreamBufferLen];

  // sound sample (16 bits) amplification
  //const int MaxAmplifyFactor = 20;
  //const int DefAmplifyFactor = 10;
  const int AmplifyFactor = 20;
  //const int QuiteThreshold = 1000;

  void i2s_install();
  void i2s_setpin();
#endif



const char* YesWavFileName = "voice_yes.wav";
const char* NoWavFileName = "voice_no.wav";
const char* WellWavFileName = "voice_well.wav";
const char* BarkWavFileName = "sound_bark.wav";
const char* MicSound = "mic_sound";


// declare "YES" (etc) lcd layers, acting as buttons ... they will be created in setup block
LcdDDLayer* yesLayer;
LcdDDLayer* noLayer;
LcdDDLayer* wellLayer;
LcdDDLayer* barkLayer;
LcdDDLayer* detectYesLayer;
LcdDDLayer* detectNoLayer;
LcdDDLayer* detectWellLayer;
LcdDDLayer* detectBarkLayer;

#if defined(WITH_I2S_MIC)
  LcdDDLayer* micLayer;
#endif

// declears "status" lcd layer ... it will be created in setup block
LcdDDLayer* statusLayer;

// declare "tunnel" etc to send detect request to api.wit.ai ... please get "access token" from api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAccessToken = WIT_ACCESS_TOKEN;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");

void setup() {

#if defined(WITH_I2S_MIC)
  // set up I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
#endif

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

#if defined(WITH_I2S_MIC)
  micLayer = dumbdisplay.createLcdLayer(16, 3);
  micLayer->writeCenteredLine("MIC", 1);
  micLayer->border(3, "darkgreen", "round");
  micLayer->backgroundColor("lightgreen");
  micLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked
#endif

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
#if defined(WITH_I2S_MIC)
    addLayer(micLayer).
#endif
    addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinBuilder.build());
}

#if defined(WITH_I2S_MIC)
  bool cacheMicSound();
#endif

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
  } else {
#if defined(WITH_I2S_MIC)
    if (micLayer->getFeedback()) {
      detectSound = MicSound;
    }
#endif
  }
#if defined(WITH_I2S_MIC)
  if (detectSound == MicSound) {
    if (!cacheMicSound()) {
      detectSound = NULL;
    }
  }
#endif
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

#if defined(WITH_I2S_MIC)
  bool cacheMicSound() {
    dumbdisplay.writeComment("using mic");
    micLayer->writeCenteredLine("stop", 1);
    statusLayer->writeCenteredLine("... listening ...");
    long startMillis = -1;//millis();
    long totalSampleCount = 0;
    long lastHighMillis = -1;
    int chunkId = dumbdisplay.cacheSoundChunked16(MicSound, SoundSampleRate, SoundNumChannels);
    //bool ok = true;
    while (true) {
      if (micLayer->getFeedback()) {
        break;
      }
      size_t bytesRead = 0;
      esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
      if (result != ESP_OK) {
        startMillis = -1;  // signal something is wrong
        break;
      }
      int samplesRead = bytesRead / 2;  // 16 bit per sample
      if (samplesRead > 0) {
        int32_t maxAbsVal = 0;
        for (int i = 0; i < samplesRead; ++i) {
          int32_t val = StreamBuffer[i];
          val = AmplifyFactor * val;
          if (val > 32700) {
            val = 32700;
          } else if (val < -32700) {
            val = -32700;
          }
          StreamBuffer[i] = val;
          int32_t absVal = abs(val);
          if (absVal > maxAbsVal) {
            maxAbsVal = absVal;
          }
        }
        if (maxAbsVal >= (200 * AmplifyFactor)) {
          //dumbdisplay.writeComment(String(maxAbsVal));
          lastHighMillis = millis();
        }
        if (startMillis == -1) {
          if (lastHighMillis != -1) {
            startMillis = millis();
            statusLayer->writeCenteredLine("... hearing ...");
          }
        }
        if (startMillis != -1) {
          totalSampleCount += samplesRead;
          dumbdisplay.sendSoundChunk16(chunkId, StreamBuffer, samplesRead, false);
        }
      }
      if (startMillis != -1) {
        if (lastHighMillis != -1) {
          if ((millis() - lastHighMillis) >= 1000) {
            // if silent for more than a second, stop it
            break;
          }
        }
        if ((millis() - startMillis) >= 30000) {
          // recording too long, force stop it
          break;
        }
      }
    }
    dumbdisplay.sendSoundChunk16(chunkId, NULL, 0, true);
    micLayer->writeCenteredLine("MIC", 1);
    bool ok = startMillis != -1 && totalSampleCount > 0;
    if (ok) {
      float forHowLongS = (float) totalSampleCount / 8000;
      statusLayer->writeCenteredLine("... got it ...");
      dumbdisplay.playSound(MicSound);
      delay(1000 * (1 + forHowLongS));
    }
    statusLayer->clear();
    return ok;
  }
#endif




#if defined(WITH_I2S_MIC)

  void i2s_install() {
    const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SoundSampleRate,
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0,
      .dma_buf_count = I2S_DMA_BUF_COUNT/*8*/,
      .dma_buf_len = I2S_DMA_BUF_LEN/*1024*/,
      .use_apll = false
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  }
  
  void i2s_setpin() {
    const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = -1,
      .data_in_num = I2S_SD
    };
    i2s_set_pin(I2S_PORT, &pin_config);
  }


#endif