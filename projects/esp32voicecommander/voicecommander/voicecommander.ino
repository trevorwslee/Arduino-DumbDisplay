

#define ENABLE_ESPNOW_REMOTE_COMMANDS
#define LIGHT_ESP_NOW_MAC   0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15
#define DOOR_ESP_NOW_MAC    0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15



// I2S driver
#include <driver/i2s.h>

// INMP441 I2S pin assignment
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32

// I2S processor
#define I2S_PORT I2S_NUM_0



// ESP32 Bluetooth with name ESP32
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));



const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;

const int SoundSampleRate = 8000;  // will be 16-bit per sample
const int SoundNumChannels = 1;


// 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 2048 bytes = 128 ms per read
const int StreamBufferNumBytes = 2048;
const int StreamBufferLen = StreamBufferNumBytes / 2;
int16_t StreamBuffer[StreamBufferLen];

// sound sample (16 bits) amplification
const int MaxAmplifyFactor = 40;
const int DefAmplifyFactor = 20;

const int32_t SilentThreshold = 200;
const long StopCacheSilentMillis = 1000;
const long MaxCacheVoiceMillis = 30000;


void i2s_install();
void i2s_setpin();

#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)
  bool espnow_init();
#endif


const char* MicVoiceName = "mic_voice";
const char* OkSoundName = "voice_ok.wav";


LcdDDLayer* micLayer;
LcdDDLayer* amplifyLblLayer;
LedGridDDLayer* amplifyMeterLayer;
LcdDDLayer* statusLayer;

// declare "tunnel" etc to send detect request to api.wit.ai ... please get "access token" from api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAccessToken = WIT_VC_ACCESS_TOKEN;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");


DDConnectVersionTracker cvTracker(-1);  // it is for tracking [new] DD connection established 
int amplifyFactor = DefAmplifyFactor;
bool cachingVoice = false;


void setup() {

  dumbdisplay.connect();  // explicitly connect ... so that can write comments

  dumbdisplay.writeComment("set up I2S ...");
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  dumbdisplay.writeComment("... done set up I2S");


#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)
  dumbdisplay.writeComment("init ESP-NOW ...");
  espnow_init();
  dumbdisplay.writeComment("... done init ESP-NOW");
#endif  


  dumbdisplay.recordLayerSetupCommands();  // start recording the layout commands

  micLayer = dumbdisplay.createLcdLayer(16, 3);
  micLayer->border(3, "darkgreen", "round");
  micLayer->backgroundColor("lightgreen");
  micLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "amplify" label on top the the "amplify" meter layer (to be created next)
  amplifyLblLayer = dumbdisplay.createLcdLayer(12, 1);
  amplifyLblLayer->pixelColor("darkred");
  amplifyLblLayer->noBackgroundColor();

  // create "amplify" meter layer
  amplifyMeterLayer = dumbdisplay.createLedGridLayer(MaxAmplifyFactor, 1, 1, 5);
  amplifyMeterLayer->onColor("darkblue");
  amplifyMeterLayer->offColor("lightgray");
  amplifyMeterLayer->border(0.2, "blue");
  amplifyMeterLayer->enableFeedback("fa:rpt50");  // rep50 means auto repeat every 50 milli-seconds

  // create "status" layer
  statusLayer = dumbdisplay.createLcdLayer(18, 1);
  statusLayer->pixelColor("darkblue");
  statusLayer->border(3, "blue");
  statusLayer->backgroundColor("white");
  statusLayer->writeCenteredLine("not ready");

  // create / setup "tunnel" etc to send detect request
  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addHeader("Content-Type", "audio/wav");
  witEndpoint.addParam("text");  // "text" is not absolutely needed
  witEndpoint.addParam("value");


  // auto pin the layers in the desired way
  DDAutoPinConfigBuilder<1> autoPinBuilder('V');
  autoPinBuilder
    .addLayer(micLayer)
    .beginGroup('S')  // stacked, one on top of another
      .addLayer(amplifyLblLayer)  
      .addLayer(amplifyMeterLayer)
    .endGroup()  
    .addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinBuilder.build());

  dumbdisplay.playbackLayerSetupCommands("esp32ddvoicecommander");  // playback the stored layout commands, as well as persist the layout to phone, so that can reconnect

  // set when DD idle handler ... here is a lambda expression
  dumbdisplay.setIdleCalback([](long idleForMillis) {
    cachingVoice = false;  // if idle, e.g. disconnected, stop whatever
  });
}


bool cacheMicVoice(int amplifyFactor, bool playback);
bool sendCommand(const String& commandTarget, const String& commandAction);


void loop() {

  bool updateAmplifyFactor = false;
  if (cvTracker.checkChanged(dumbdisplay)) {
    micLayer->writeCenteredLine("MIC", 1);
    statusLayer->clear();
    updateAmplifyFactor = true;
  }

  const DDFeedback* feedback = amplifyMeterLayer->getFeedback();
  if (feedback != NULL) {
      amplifyFactor = feedback->x + 1;
      updateAmplifyFactor = true;
  }

  if (updateAmplifyFactor) {
    amplifyMeterLayer->horizontalBar(amplifyFactor);
    amplifyLblLayer->writeLine(String(amplifyFactor), 0, "R");
  }

  bool cachedMicVoice = false;
  if (micLayer->getFeedback()) {
    cachedMicVoice = cacheMicVoice(amplifyFactor, false);
  }

  if (cachedMicVoice) {
    dumbdisplay.tone(2000, 100);
    witEndpoint.resetSoundAttachment(MicVoiceName);
    witTunnel->reconnectToEndpoint(witEndpoint);
    micLayer->writeCenteredLine("...", 1);
    statusLayer->writeCenteredLine("... detecting ...");
    dumbdisplay.writeComment("detecting ...");
    String entity;
    String trait;
    while (!witTunnel->eof()) {
      String fieldId;
      String fieldValue;
      if (witTunnel->read(fieldId, fieldValue)) {
        //dumbdisplay.writeComment(String(". [") + fieldId + "]=" + fieldValue);
        if (fieldId.startsWith("entities.") && fieldId.endsWith(".value")) {
          entity = fieldValue;
        } else if (fieldId.startsWith("traits.") && fieldId.endsWith(".value")) {
          trait = fieldValue;
        } else if (fieldId == "text") {
          dumbdisplay.writeComment(String("   {") + fieldValue + "}");
        }
      }
    }
    if (entity.length() > 0 && trait.length() > 0) {
      dumbdisplay.writeComment("... detected:");
      dumbdisplay.writeComment(String(". ENTITY : ") + entity);
      dumbdisplay.writeComment(String(". TRAIT  : ") + trait);
      dumbdisplay.playSound(OkSoundName);
      // send the entity and trait got as commend
      sendCommand(entity, trait);
    } else {
      dumbdisplay.writeComment("... nothing");
      dumbdisplay.tone(1500, 100);
    }
    micLayer->writeCenteredLine("MIC", 1);
    statusLayer->clear();
  }
}

bool cacheMicVoice(int amplifyFactor, bool playback) {
  cachingVoice = true;
  int32_t silentThreshold = SilentThreshold * amplifyFactor;
  micLayer->writeCenteredLine("done", 1);
  statusLayer->writeCenteredLine("... hearing ...");
  long startMillis = -1;
  long totalSampleCount = 0;
  long lastHighMillis = -1;
  int chunkId = dumbdisplay.cacheSoundChunked16(MicVoiceName, SoundSampleRate, SoundNumChannels);
  while (true) {
    if (micLayer->getFeedback()) {
      break;
    }
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
    if (result != ESP_OK || !cachingVoice) {
      startMillis = -1;  // signal something is wrong .. cancel it
      break;
    }
    int samplesRead = bytesRead / 2;  // 16 bit per sample
    if (samplesRead > 0) {
      int32_t maxAbsVal = 0;
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = StreamBuffer[i];
        val = amplifyFactor * val;
        if (amplifyFactor > 1) {
          if (val > 32700) {
            val = 32700;
          } else if (val < -32700) {
            val = -32700;
          }
          StreamBuffer[i] = val;
        }
        int32_t absVal = abs(val);
        if (absVal > maxAbsVal) {
          maxAbsVal = absVal;
        }
      }
      if (maxAbsVal >= silentThreshold) {
        lastHighMillis = millis();
      }
      if (startMillis == -1) {
        if (lastHighMillis != -1) {
          startMillis = millis();
          statusLayer->writeCenteredLine("... listening ...");
        } 
      }
      if (startMillis != -1) {
        totalSampleCount += samplesRead;
        dumbdisplay.sendSoundChunk16(chunkId, StreamBuffer, samplesRead, false);
      }
    }
    if (startMillis != -1) {
      if (lastHighMillis != -1) {
        if ((millis() - lastHighMillis) >= StopCacheSilentMillis) {
          // if silent for some time, stop it
          break;
        }
      }
      if ((millis() - startMillis) >= MaxCacheVoiceMillis) {
        // recording for too long, force stop it
        break;
      }
    }
  }
  dumbdisplay.sendSoundChunk16(chunkId, NULL, 0, true);
  micLayer->writeCenteredLine("MIC", 1);
  bool ok = startMillis != -1 && totalSampleCount > 0;
  if (ok && playback) {
    float forHowLongS = (float) totalSampleCount / 8000;
    statusLayer->writeCenteredLine("... got it ...");
    dumbdisplay.playSound(MicVoiceName);
    delay(1000 * (1 + forHowLongS));
  }
  statusLayer->clear();
  cachingVoice = false;
  return ok;
}



void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SoundSampleRate,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = I2S_DMA_BUF_COUNT,
    .dma_buf_len = I2S_DMA_BUF_LEN,
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




#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)


#include <esp_now.h>
#include <WiFi.h>


#if defined LIGHT_ESP_NOW_MAC  
  uint8_t LightReceiverMACAddress[] = { LIGHT_ESP_NOW_MAC };
  esp_now_peer_info_t LightPeerInfo;
#endif
#if defined DOOR_ESP_NOW_MAC  
  uint8_t DoorReceiverMACAddress[] = { DOOR_ESP_NOW_MAC };
  esp_now_peer_info_t DoorPeerInfo;
#endif


// define a structure as an ESP Now packet
struct ESPNowCommandPacket {
  char commandTarget[16];
  char commandAction[16];
};


bool espnow_init() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);  // default is STA mode
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    dumbdisplay.writeComment("Error initializing ESP-NOW");
    return false;
  }  

  // Register "send callback" lambda expression
  esp_now_register_send_cb([](const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      dumbdisplay.writeComment("Successful sent ESP-NOW packet");
    } else {
      dumbdisplay.writeComment("Failed to send ESP-NOW packet");
    }
  });

  // Register and Add peers        
#if defined (LIGHT_ESP_NOW_MAC)  
  memcpy(LightPeerInfo.peer_addr, LightReceiverMACAddress, 6);
  LightPeerInfo.channel = 0;  
  LightPeerInfo.encrypt = false;
  if (esp_now_add_peer(&LightPeerInfo) != ESP_OK){
    dumbdisplay.writeComment("Failed to add \"Light\" peer"); }
#endif  
#if defined (DOOR_ESP_NOW_MAC)  
  memcpy(DoorPeerInfo.peer_addr, DoorReceiverMACAddress, 6);
  DoorPeerInfo.channel = 0;  
  DoorPeerInfo.encrypt = false;
  if (esp_now_add_peer(&DoorPeerInfo) != ESP_OK){
    dumbdisplay.writeComment("Failed to add \"Door\" peer");
  }
#endif  

  return true;
}

#endif


const uint8_t* getCommandReceiverMACAddress(const String& commandTraget, const String& commandAction) {
#if defined LIGHT_ESP_NOW_MAC
  if (commandAction == "on" || commandAction == "off") {
    return LightReceiverMACAddress;
  }  
#endif
#if defined (DOOR_ESP_NOW_MAC)  
  if (commandAction == "lock" || commandAction == "unlock") {
    return DoorReceiverMACAddress;
  }  
#endif
  return NULL;
}

bool sendCommand(const String& commandTarget, const String& commandAction) {
  dumbdisplay.writeComment(String("command [") + commandTarget + "] to [" + commandAction + "]");
  const uint8_t* receiverMACAddress = getCommandReceiverMACAddress(commandTarget, commandAction);
  if (receiverMACAddress == NULL) {
    dumbdisplay.writeComment("no command receiver");
    return false;
  }
  ESPNowCommandPacket packet;
  strcpy(packet.commandTarget, commandTarget.c_str());
  strcpy(packet.commandAction, commandAction.c_str());
  esp_now_send(receiverMACAddress, (const uint8_t *) &packet, sizeof(packet));
  return true;
}

