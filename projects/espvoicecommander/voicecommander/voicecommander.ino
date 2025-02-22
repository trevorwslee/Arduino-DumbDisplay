

// if no ESP-NOW clients, make sure the following line is commented out
#define ENABLE_ESPNOW_REMOTE_COMMANDS

// define ESP-NOW clients ... comment out what not used
#define LIGHT_ESP_NOW_MAC   0x94, 0xB5, 0x55, 0xC7, 0xCD, 0x60
#define DOOR_ESP_NOW_MAC    0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15
#define FAN_ESP_NOW_MAC     0x84, 0xF3, 0xEB, 0xD8, 0x41, 0x53




// I2S driver
#include <driver/i2s.h>


// INMP441 I2S pin assignment
#if defined(FOR_LILYGO_TSIMCAM)
  // for the mic built-in to LiLyGO TSimCam
  #define I2S_WS               42
  #define I2S_SD               2
  #define I2S_SCK              41
#elif defined(FOR_VCC_S3EYE)
  #define I2S_WS               42
  #define I2S_SD               2
  #define I2S_SCK              41
#else
  #define I2S_WS               25
  #define I2S_SD               33
  #define I2S_SCK              32
#endif


// I2S processor
#define I2S_PORT I2S_NUM_0



#include "wifidumbdisplay.h"
DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));


const int I2S_DMA_BUF_COUNT = 4;
const int I2S_DMA_BUF_LEN = 1024;

const int SoundSampleRate = 16000;  // will be 16-bit per sample
const int SoundNumChannels = 1;



// 16000 sample per second (32000 bytes per second; since 16 bits per sample) ==> 8192 bytes = 256 ms per read
const int StreamBufferNumBytes = 8192;
const int StreamBufferLen = StreamBufferNumBytes / 2;
int16_t StreamBuffer[StreamBufferLen];

// sound sample (16 bits) amplification
const int MaxAmplifyFactor = 40;
const int DefAmplifyFactor = 10;

const int32_t SilentThreshold = 200;
const int VoiceMinOverSilentThresholdCount = 5;
const long StopCacheSilentMillis = 1500;
const long MaxCacheVoiceMillis = 30000;


void i2s_install();
void i2s_setpin();

#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)
  bool initESPNow();
#endif

const char* MicVoiceName = "mic_voice";
const char* OkSoundName = "voice_ok.wav";


LcdDDLayer* micLayer;
LcdDDLayer* replayLayer;
LcdDDLayer* amplifyLblLayer;
LedGridDDLayer* amplifyMeterLayer;
LcdDDLayer* statusLayer;

// declare "tunnel" etc to send detect request to api.wit.ai ... please get "access token" from api.wit.ai
JsonDDTunnel* witTunnel;
const char* witAccessToken = WIT_VC_ACCESS_TOKEN;
DDTunnelEndpoint witEndpoint("https://api.wit.ai/speech");


DDConnectVersionTracker cvTracker;  // it is for tracking [new] DD connection established 
bool replayVoiceAfterCache = false;
int amplifyFactor = DefAmplifyFactor;
bool cachingVoice = false;


void setup() {

  dumbdisplay.connect();  // explicitly connect ... so that can write comments to DD

  dumbdisplay.writeComment("set up I2S ...");
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  dumbdisplay.writeComment("... done set up I2S");


#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)
  dumbdisplay.writeComment("init ESP-NOW ...");
  initESPNow();
  dumbdisplay.writeComment("... done init ESP-NOW");
#endif  


  dumbdisplay.recordLayerSetupCommands();  // start recording the layout commands

  micLayer = dumbdisplay.createLcdLayer(16, 3);
  micLayer->border(2, DD_COLOR_darkgreen, "round", 2);
  micLayer->backgroundColor(DD_COLOR_lightgreen);
  micLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  replayLayer = dumbdisplay.createLcdLayer(10, 1);
  //replayLayer->border(0.5, "blue");
  replayLayer->enableFeedback("fl");  // enable "feedback" ... i.e. it can be clicked

  // create "amplify" label on top the the "amplify" meter layer (to be created next)
  amplifyLblLayer = dumbdisplay.createLcdLayer(12, 1);
  amplifyLblLayer->pixelColor(DD_COLOR_darkred);
  amplifyLblLayer->noBackgroundColor();

  // create "amplify" meter layer
  amplifyMeterLayer = dumbdisplay.createLedGridLayer(MaxAmplifyFactor, 1, 1, 5);
  amplifyMeterLayer->onColor(DD_COLOR_darkblue);
  amplifyMeterLayer->offColor(DD_COLOR_lightgray);
  amplifyMeterLayer->border(0.2, DD_COLOR_blue);
  amplifyMeterLayer->enableFeedback("fa:rpt50");  // rep50 means auto repeat every 50 milli-seconds

  // create "status" layer
  statusLayer = dumbdisplay.createLcdLayer(18, 1);
  statusLayer->pixelColor(DD_COLOR_darkblue);
  statusLayer->border(3, "blue");
  statusLayer->backgroundColor(DD_COLOR_white);
  statusLayer->writeCenteredLine("not ready");

  // create / setup "tunnel" etc to send detect request
  witTunnel = dumbdisplay.createJsonTunnel("", false);
  witEndpoint.addHeader("Authorization", String("Bearer ") + witAccessToken);
  witEndpoint.addHeader("Content-Type", "audio/wav");
  witEndpoint.addParam("text");  // "text" is not absolutely needed
  witEndpoint.addParam("value");


  // auto pin the layers in the desired way
  DDAutoPinConfig autoPinConfig('V');
  autoPinConfig
    .addLayer(micLayer)
    .beginGroup('H')
      .addLayer(replayLayer)
      .beginGroup('S')  // stacked, one on top of another
        .addLayer(amplifyLblLayer)  
        .addLayer(amplifyMeterLayer)
      .endGroup()
    .endGroup()    
    .addLayer(statusLayer);
  dumbdisplay.configAutoPin(autoPinConfig.build());

  dumbdisplay.playbackLayerSetupCommands("esp32ddvoicecommander");  // playback the stored layout commands, as well as persist the layout to phone, so that can reconnect

  // set when DD idle handler ... here is a lambda expression
  dumbdisplay.setIdleCallback([](long idleForMillis, DDIdleConnectionState connectionState) {
    if (connectionState == DDIdleConnectionState::IDLE_RECONNECTING) {
      cachingVoice = false;
    }
  });
}


bool cacheMicVoice(int amplifyFactor, bool playback);
bool sendCommand(const String& commandTarget, const String& commandAction);


void loop() {

  bool updateReplayUI = false;
  bool updateAmplifyFactorUI = false;
  if (cvTracker.checkChanged(dumbdisplay)) {
    micLayer->writeCenteredLine("Start", 1);
    statusLayer->clear();
    updateReplayUI = true;
    updateAmplifyFactorUI = true;
  }

  if (replayLayer->getFeedback()) {
    replayVoiceAfterCache = !replayVoiceAfterCache;
    updateReplayUI = true;
  }

  const DDFeedback* feedback = amplifyMeterLayer->getFeedback();
  if (feedback != NULL) {
      amplifyFactor = feedback->x + 1;
      updateAmplifyFactorUI = true;
  }

  if (updateReplayUI) {
    if (replayVoiceAfterCache) {
        replayLayer->border(1, "darkred", "flat");
        replayLayer->pixelColor("darkred");
        replayLayer->writeCenteredLine("no replay");
    } else {
        replayLayer->border(1, "darkgray", "hair");
        replayLayer->pixelColor("blue");
        replayLayer->writeCenteredLine("replay");
    }
  }
  if (updateAmplifyFactorUI) {
    amplifyMeterLayer->horizontalBar(amplifyFactor);
    amplifyLblLayer->writeLine(String(amplifyFactor), 0, "R");
  }

  if (micLayer->getFeedback()) {
    replayLayer->disabled(true);
    amplifyMeterLayer->disabled(true);

    while (true) {
      
      micLayer->writeCenteredLine("Stop", 1);
      
      // get voice command
      if (!cacheMicVoice(amplifyFactor, replayVoiceAfterCache)) {
        break;
      }

      // got voice command ... call Wit.ai (via DD) to recognize it
      dumbdisplay.tone(2000, 100);
      witEndpoint.resetSoundAttachment(MicVoiceName);
      witTunnel->reconnectToEndpoint(witEndpoint);
      micLayer->disabled(true);
      micLayer->writeCenteredLine("...", 1);
      statusLayer->writeCenteredLine("... detecting ...");
      dumbdisplay.writeComment("detecting ...");
      

      // gather Wit.ai result
      String entity;
      String trait;
      while (!witTunnel->eof()) {
        String fieldId;
        String fieldValue;
        if (witTunnel->read(fieldId, fieldValue)) {
          // if (true) {
          //   dumbdisplay.writeComment(String(". [") + fieldId + "]=" + fieldValue);
          // }
          if (fieldId.startsWith("entities.") && fieldId.endsWith(".value")) {
            entity = fieldValue;
          } else if (fieldId.startsWith("traits.") && fieldId.endsWith(".value")) {
            trait = fieldValue;
          } else if (fieldId == "text") {
            dumbdisplay.writeComment(String("   {") + fieldValue + "}");   // for display only
          }
        }
      }

      // intepret Wit.ai result
      if (entity.length() > 0 && trait.length() > 0) {
        // detected voice command ... send it out
        statusLayer->writeCenteredLine("detected");
        dumbdisplay.writeComment("... detected:");
        dumbdisplay.writeComment(String(". ENTITY : ") + entity);
        dumbdisplay.writeComment(String(". TRAIT  : ") + trait);
        dumbdisplay.playSound(OkSoundName);
        // send the entity and trait got as commend
        sendCommand(entity, trait);
      } else {
        // voice command not detected
        statusLayer->writeCenteredLine("nothing");
        dumbdisplay.writeComment("... nothing");
        dumbdisplay.tone(1500, 100);
      }
      delay(2000);  // delay a bit listening again (so that the ok / beep sound will not affect voice command recording)
      statusLayer->clear();
      micLayer->disabled(false);
    }

    micLayer->writeCenteredLine("Start", 1);
    replayLayer->disabled(false);
    amplifyMeterLayer->disabled(false);
  }
}

bool cacheMicVoice(int amplifyFactor, bool playback) {
  cachingVoice = true;
  int32_t silentThreshold = SilentThreshold * amplifyFactor;
  statusLayer->writeCenteredLine("... hearing ...");
  long startMillis = -1;
  long totalSampleCount = 0;
  long lastHighMillis = -1;
  int chunkId = dumbdisplay.cacheSoundChunked16(MicVoiceName, SoundSampleRate, SoundNumChannels);
  while (true) {
    if (micLayer->getFeedback()) {
      startMillis = -1;  // cancel it
      break;
    }
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
    if (result != ESP_OK || !cachingVoice) {
      startMillis = -1;  // cancel it
      break;
    }
    int samplesRead = bytesRead / 2;  // 16 bit per sample
    if (samplesRead > 0) {
      int overThresholdCount = 0;
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
        if (absVal > silentThreshold) {
          overThresholdCount += 1;
        }
      }
      if (overThresholdCount >= VoiceMinOverSilentThresholdCount) {
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
        // caching for too long, force stop it
        break;
      }
    }
  }
  dumbdisplay.sendSoundChunk16(chunkId, NULL, 0, true);
  bool ok = startMillis != -1 && totalSampleCount > 0;
  if (ok && playback) {
    float forHowLongS = (float) totalSampleCount / SoundSampleRate;
    statusLayer->writeCenteredLine("... replaying ...");
    dumbdisplay.playSound(MicVoiceName);
    delay(1000 * (1 + forHowLongS));
  }
  statusLayer->writeCenteredLine("... got it ...");
  statusLayer->clear();
  cachingVoice = false;
  return ok;
}



void i2s_install() {
  uint32_t mode = I2S_MODE_MASTER | I2S_MODE_RX;
  #if I2S_SCK == I2S_PIN_NO_CHANGE
      mode |= I2S_MODE_PDM;
  #endif    
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(mode),
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





// *****
// * ENABLE_ESPNOW_REMOTE_COMMANDS
// *****


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
#if defined FAN_ESP_NOW_MAC  
  uint8_t FanReceiverMACAddress[] = { FAN_ESP_NOW_MAC };
  esp_now_peer_info_t FanPeerInfo;
#endif


// define a structure as ESP Now packet
struct ESPNowCommandPacket {
  char commandTarget[32];
  char commandAction[32];
};


bool initESPNow() {
  // Set device as a Wi-Fi Station and also an Access Point
  WiFi.mode(WIFI_AP_STA);
  
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
      char mac_str[18];
      sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
      dumbdisplay.writeComment(String("Failed to send ESP-NOW packet to ") + mac_str/* + " ... " + String(status)*/);
    }
  });

  // Register and Add peers        
#if defined (LIGHT_ESP_NOW_MAC)  
  memcpy(LightPeerInfo.peer_addr, LightReceiverMACAddress, 6);
  LightPeerInfo.channel = 0;  
  LightPeerInfo.encrypt = false;
  if (esp_now_add_peer(&LightPeerInfo) != ESP_OK) {
    dumbdisplay.writeComment("Failed to add \"Light\" peer");
  }
#endif  
#if defined (DOOR_ESP_NOW_MAC)  
  memcpy(DoorPeerInfo.peer_addr, DoorReceiverMACAddress, 6);
  DoorPeerInfo.channel = 0;  
  DoorPeerInfo.encrypt = false;
  if (esp_now_add_peer(&DoorPeerInfo) != ESP_OK) {
    dumbdisplay.writeComment("Failed to add \"Door\" peer");
  }
#endif  
#if defined (FAN_ESP_NOW_MAC)  
  memcpy(FanPeerInfo.peer_addr, FanReceiverMACAddress, 6);
  FanPeerInfo.channel = 0;  
  FanPeerInfo.encrypt = false;
  if (esp_now_add_peer(&FanPeerInfo) != ESP_OK) {
    dumbdisplay.writeComment("Failed to add \"Fan\" peer");
  }
#endif  

  return true;
}

#endif




const uint8_t* getCommandReceiverMACAddress(const String& commandTraget, const String& commandAction) {
#if defined (FAN_ESP_NOW_MAC)  
  if (commandTraget == "fan" && (commandAction == "on" || commandAction == "off")) {
    return FanReceiverMACAddress;
  }
#endif
#if defined (DOOR_ESP_NOW_MAC)  
  if (commandAction == "lock" || commandAction == "unlock") {
    return DoorReceiverMACAddress;
  }
#endif
#if defined (LIGHT_ESP_NOW_MAC)
  // handle all not handled  
  if (commandAction == "on" || commandAction == "off" || commandAction == "lock" || commandAction == "unlock") {
    return LightReceiverMACAddress;
  }  
#endif
  return NULL;
}

bool sendCommand(const String& commandTarget, const String& commandAction) {
  dumbdisplay.writeComment(String("command for [") + commandTarget + "] to [" + commandAction + "]");

#if defined(ENABLE_ESPNOW_REMOTE_COMMANDS)
  const uint8_t* receiverMACAddress = getCommandReceiverMACAddress(commandTarget, commandAction);
  if (receiverMACAddress == NULL) {
    dumbdisplay.writeComment("no command receiver");
    return false;
  }
  ESPNowCommandPacket packet;
  strcpy(packet.commandTarget, commandTarget.c_str());
  strcpy(packet.commandAction, commandAction.c_str());
  if (esp_now_send(receiverMACAddress, (const uint8_t *) &packet, sizeof(packet)) != ESP_OK) {
    dumbdisplay.writeComment("failed to send command");
    return false;
  }
#endif  

  return true;
}



