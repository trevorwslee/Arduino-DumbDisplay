// if want Bluetooth, uncomment the following line
// #define BLUETOOTH "ESP32BT"
#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH));
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


// I2S driver
#include <driver/i2s.h>
 
// INMP441 I2S pin assignment
#if defined(FOR_LILYGO_TCAMERAPLUS)
  // for the mic built-in to LiLyGO TCamerPlus
  #define I2S_WS               32
  #define I2S_SD               33
  #define I2S_SCK              14
  #define I2S_SAMPLE_BIT_COUNT 32
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#elif defined(FOR_LILYGO_TSIMCAM)
  // for the mic built-in to LiLyGO TSimCam
  #define I2S_WS               42
  #define I2S_SD                2
  #define I2S_SCK              41
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#elif defined(FOR_LILYGO_TWATCH)
  #define I2S_WS               0
  #define I2S_SD               2
  #define I2S_SCK              I2S_PIN_NO_CHANGE
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#elif defined(FOR_VCC_S3EYE) || defined(FOR_VCC_S3EYE_2)
  #define I2S_WS               42 //0
  #define I2S_SD               2 //2
  #define I2S_SCK              41 //I2S_PIN_NO_CHANGE
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#elif defined(FOR_XIAO_S3SENSE)
  #define I2S_WS               42
  #define I2S_SD               41
  #define I2S_SCK              I2S_PIN_NO_CHANGE
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#elif defined(FOR_ESP_SPARKBOT)
  #define I2S_WS               41  // WS
  #define I2S_SD               40  // DIN
  #define I2S_SCK              39  // BCLK
  #define I2S_SAMPLE_BIT_COUNT 32
  #define SOUND_SAMPLE_RATE    16000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_1
  //#error not yet working
#else
  #define I2S_WS               25
  #define I2S_SD               33
  #define I2S_SCK              32
  #define I2S_SAMPLE_BIT_COUNT 16
  #define SOUND_SAMPLE_RATE    8000
  #define SOUND_CHANNEL_COUNT  1
  #define I2S_PORT             I2S_NUM_0
#endif




PlotterDDLayer* plotterLayer;
LcdDDLayer* micTabLayer;
LcdDDLayer* recTabLayer;
LcdDDLayer* playTabLayer;
LcdDDLayer* startBtnLayer;
LcdDDLayer* stopBtnLayer;
LcdDDLayer* amplifyLblLayer;
LedGridDDLayer* amplifyMeterLayer;



// name of recorded WAV file; since only a single name; hence new one will always overwrite old one
const char* SoundName = "recorded_sound";


const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;


#if I2S_SAMPLE_BIT_COUNT == 32
  const int StreamBufferNumBytes = 512;
  const int StreamBufferLen = StreamBufferNumBytes / 4;
  int32_t StreamBuffer[StreamBufferLen];
#else
  #if SOUND_SAMPLE_RATE == 16000
    // for 16 bits ... 16000 sample per second (32000 bytes per second; since 16 bits per sample) ==> 512 bytes = 16 ms per read
    const int StreamBufferNumBytes = 512;
  #else
    // for 16 bits ... 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
    const int StreamBufferNumBytes = 256;
  #endif  
  const int StreamBufferLen = StreamBufferNumBytes / 2;
  int16_t StreamBuffer[StreamBufferLen];
#endif

// sound sample (16 bits) amplification
const int MaxAmplifyFactor = 20;
const int DefAmplifyFactor = 10;


esp_err_t i2s_install();
esp_err_t i2s_setpin();
 

DDConnectVersionTracker cvTracker;  // it is for tracking [new] DD connection established 
int what = 1;  // 1: mic; 2: record; 3: play
bool started = false;
int amplifyFactor = DefAmplifyFactor;//10;
int soundChunkId = -1; // when started sending sound [chunk], the allocated "chunk id"
long streamingMillis = 0;
int streamingTotalSampleCount = 0;


void setup() {

  Serial.begin(115200);

  Serial.println("SETUP MIC ...");

  // set up I2S
  if (i2s_install() != ESP_OK) {
    Serial.println("XXX failed to install I2S");
  }
  if (i2s_setpin() != ESP_OK) {
    Serial.println("XXX failed to set I2S pins");
  }
  if (i2s_zero_dma_buffer(I2S_PORT) != ESP_OK) {
    Serial.println("XXX failed to zero I2S DMA buffer");
  }
  if (i2s_start(I2S_PORT) != ESP_OK) {
    Serial.println("XXX failed to start I2S");
  }

  Serial.println("... DONE SETUP MIC");


  dumbdisplay.recordLayerSetupCommands();  // start recording the layout commands

  plotterLayer = dumbdisplay.createPlotterLayer(1024, 256, SOUND_SAMPLE_RATE / StreamBufferLen);

  // create "MIC/REC/PLAY" lcd layers, as tab
  micTabLayer = dumbdisplay.createLcdLayer(8, 1);
  micTabLayer->writeCenteredLine("MIC");
  micTabLayer->border(1, "gray");
  micTabLayer->enableFeedback("f");
  recTabLayer = dumbdisplay.createLcdLayer(8, 1);
  recTabLayer->writeCenteredLine("REC");
  recTabLayer->border(1, "gray");
  recTabLayer->enableFeedback("f");
  playTabLayer = dumbdisplay.createLcdLayer(8, 1);
  playTabLayer->writeCenteredLine("PLAY");
  playTabLayer->border(1, "gray");
  playTabLayer->enableFeedback("f");

  // create "START/STOP" lcd layer, acting as a button
  startBtnLayer = dumbdisplay.createLcdLayer(12, 3);
  startBtnLayer->pixelColor("darkgreen");
  startBtnLayer->border(2, "darkgreen", "round");
  startBtnLayer->margin(1);
  startBtnLayer->enableFeedback("fl");
  stopBtnLayer = dumbdisplay.createLcdLayer(12, 3);
  stopBtnLayer->pixelColor("darkred");
  stopBtnLayer->border(2, "darkgreen", "round");
  stopBtnLayer->margin(1);
  stopBtnLayer->enableFeedback("fl");

  // create "amplify" label on top the the "amplify" meter layer (to be created next)
  amplifyLblLayer = dumbdisplay.createLcdLayer(12, 1);
  amplifyLblLayer->pixelColor("darkred");
  amplifyLblLayer->noBackgroundColor();

  // create "amplify" meter layer
  amplifyMeterLayer = dumbdisplay.createLedGridLayer(MaxAmplifyFactor, 1, 1, 2);
  amplifyMeterLayer->onColor("darkblue");
  amplifyMeterLayer->offColor("lightgray");
  amplifyMeterLayer->border(0.2, "blue");
  amplifyMeterLayer->enableFeedback("fa:rpt50");  // rep50 means auto repeat every 50 milli-seconds

  DDAutoPinConfig builder('V');  // vertical
  builder
    .addLayer(plotterLayer)
    .beginGroup('H')  // horizontal
      .addLayer(micTabLayer)
      .addLayer(recTabLayer)
      .addLayer(playTabLayer)
    .endGroup()
    .beginGroup('H')  // horizontal
      .addLayer(startBtnLayer)
      .addLayer(stopBtnLayer)
    .endGroup()
    .beginGroup('S')  // stacked, one on top of another
      .addLayer(amplifyLblLayer)  
      .addLayer(amplifyMeterLayer)
    .endGroup();  
  dumbdisplay.configAutoPin(builder.build());

  dumbdisplay.playbackLayerSetupCommands("esp32ddmice");  // playback the stored layout commands, as well as persist the layout to phone, so that can reconnect

  // set when DD idle handler ... here is a lambda expression
  dumbdisplay.setIdleCallback([](long idleForMillis, DDIdleConnectionState connectionState) {
    if (connectionState == DDIdleConnectionState::IDLE_RECONNECTING) {
      started = false;  // if idle, e.g. disconnected, stop whatever
    }
  });

}


void loop() {

  bool updateTab = false;
  bool updateStartStop = false;
  bool updateAmplifyFactor = false;
  if (cvTracker.checkChanged(dumbdisplay)) {
    // if here for the first time, or DD connection changed (e.g. reconnected), update every UI component
    started = false;
    updateTab = true;
    updateStartStop = true;
    updateAmplifyFactor = true;
  } else {
    // check if need to update any UI components
    int oriWhat = what;
    if (micTabLayer->getFeedback()) {
      what = 1;
    } else if (recTabLayer->getFeedback()) {
      what = 2;
    } else if (playTabLayer->getFeedback()) {
      what = 3;
    }
    if (what != oriWhat) {
      started = false;
      updateTab = true;
      updateStartStop = true;
    }
    if (startBtnLayer->getFeedback()) {
      started = true;
      updateStartStop = true;
    } else if (stopBtnLayer->getFeedback()) {
      started = false;
      updateStartStop = true;
    }
    const DDFeedback* feedback = amplifyMeterLayer->getFeedback();
    if (feedback != NULL) {
        amplifyFactor = feedback->x + 1;
        updateAmplifyFactor = true;
    }
  }

  if (updateTab) {
    const char* micColor = what == 1 ? "blue" : "gray";
    const char* micBoarderShape = what == 1 ? "flat" : "hair";
    const char* recColor = what == 2 ? "blue" : "gray";
    const char* recBoarderShape = what == 2 ? "flat" : "hair";
    const char* playColor = what == 3 ? "blue" : "gray";
    const char* playBoarderShape = what == 3 ? "flat" : "hair";
    micTabLayer->border(1, micColor, micBoarderShape);
    micTabLayer->pixelColor(micColor);
    recTabLayer->border(1, recColor, recBoarderShape);
    recTabLayer->pixelColor(recColor);
    playTabLayer->border(1, playColor, playBoarderShape);
    playTabLayer->pixelColor(playColor);
  }
  if (updateStartStop) {
    const char* whatTitle;
    if (what == 1) {
      whatTitle = "MIC";
    } else if (what == 2) {
      whatTitle = "REC";
    } else if (what == 3) {
      whatTitle = "PLAY";
    }
    startBtnLayer->writeCenteredLine(String("Start ") + whatTitle, 1);
    stopBtnLayer->writeCenteredLine(String("Stop ") + whatTitle, 1);
    if (what == 3) {
      startBtnLayer->disabled(false);
      stopBtnLayer->disabled(false);
      amplifyMeterLayer->disabled(true);
    } else {
      if (started) {
        startBtnLayer->disabled(true);
        stopBtnLayer->disabled(false);
      } else {
        startBtnLayer->disabled(false);
        stopBtnLayer->disabled(true);
      }
      micTabLayer->disabled(started);
      recTabLayer->disabled(started);
      playTabLayer->disabled(started);
      amplifyMeterLayer->disabled(false);
    }
  }
  if (updateAmplifyFactor) {
    amplifyMeterLayer->horizontalBar(amplifyFactor);
    amplifyLblLayer->writeLine(String(amplifyFactor), 0, "R");
  }

  // read I2S data and place in data buffer
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);

  // if (false) {
  //   Serial.println(String("* - ") + String(result) + " - " + String(bytesRead));
  // }

  int samplesRead = 0;
#if I2S_SAMPLE_BIT_COUNT == 32
  int16_t sampleStreamBuffer[StreamBufferLen];
#else
  int16_t *sampleStreamBuffer = StreamBuffer;
#endif
  if (result == ESP_OK) {
#if I2S_SAMPLE_BIT_COUNT == 32
      samplesRead = bytesRead / 4;  // 32 bit per sample
#else
      samplesRead = bytesRead / 2;  // 16 bit per sample
#endif    
    if (samplesRead > 0) {
      // find the samples mean ... and amplify the sound sample, by simply multiple it by some "amplify factor"
      float sumVal = 0;
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = StreamBuffer[i];
#if I2S_SAMPLE_BIT_COUNT == 32
        val = val / 0x0000ffff;
#endif
        if (amplifyFactor > 1) {
          val = amplifyFactor * val;
          if (val > 32700) {
            val = 32700;
          } else if (val < -32700) {
            val = -32700;
          }
          //StreamBuffer[i] = val;
        }
        sampleStreamBuffer[i] = val;
        sumVal += val;
      }
      float meanVal = sumVal / samplesRead;
      plotterLayer->set(meanVal);
    }
  }

  if (what == 3) {
    if (updateStartStop) {
      // i.e. click start or stop
      if (started) {
        dumbdisplay.playSound(SoundName);
      } else {
        dumbdisplay.stopSound();
      }   
    }
    return;
  }

  if (started) {
    if (soundChunkId == -1) {
      // while started ... if no allocated "chunk id" (i.e. not yet started sending sound)
      if (what == 1) {
        // start streaming sound, and get the assigned "chunk id"
        soundChunkId = dumbdisplay.streamSound16(SOUND_SAMPLE_RATE, SOUND_CHANNEL_COUNT); // sound is 16 bits per sample
        dumbdisplay.writeComment(String("STARTED mic streaming with chunk id [") + soundChunkId + "]");
      } else if (what == 2) {
        // started saving sound, and get the assigned "chunk id" 
        soundChunkId = dumbdisplay.saveSoundChunked16(SoundName, SOUND_SAMPLE_RATE, SOUND_CHANNEL_COUNT);
        dumbdisplay.writeComment(String("STARTED record streaming with chunk id [") + soundChunkId + "]");
      }
      streamingMillis = millis();
      streamingTotalSampleCount = 0;
    }
  }

  if (result == ESP_OK) {
    if (soundChunkId != -1) {
      // send sound samples read
      bool isFinalChunk = !started;  // it is the final chink if justed turned to stop
      dumbdisplay.sendSoundChunk16(soundChunkId, sampleStreamBuffer, samplesRead, isFinalChunk);
      streamingTotalSampleCount += samplesRead;
      if (isFinalChunk) {
        dumbdisplay.writeComment(String("DONE streaming with chunk id [") + soundChunkId + "]");
        long forMillis = millis() - streamingMillis;
        int totalSampleCount = streamingTotalSampleCount;
        dumbdisplay.writeComment(String(". total streamed samples: ") + totalSampleCount + " in " + String(forMillis / 1000.0) + "s");
        dumbdisplay.writeComment(String(". stream sample rate: ") + String(1000.0 * ((float) totalSampleCount / forMillis)));
        soundChunkId = -1;
      }
    }
  }
}



esp_err_t i2s_install() {
  uint32_t mode = I2S_MODE_MASTER | I2S_MODE_RX;
#if I2S_SCK == I2S_PIN_NO_CHANGE
    mode |= I2S_MODE_PDM;
#endif    
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(mode/*I2S_MODE_MASTER | I2S_MODE_RX*/),
    .sample_rate = SOUND_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BIT_COUNT),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
#if defined(FOR_ESP_SPARKBOT)  // IDF4???
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
#else
    .intr_alloc_flags = 0,
#endif  
#if defined(FOR_ESP_SPARKBOT)  // IDF4???
    .dma_buf_count = 3,
    .dma_buf_len = 1024,
#else
    .dma_buf_count = I2S_DMA_BUF_COUNT/*8*/,
    .dma_buf_len = I2S_DMA_BUF_LEN/*1024*/,
#endif
#if defined(FOR_ESP_SPARKBOT)
    .use_apll = true
#else  
    .use_apll = false
#endif  
  };
  return i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}
 
esp_err_t i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,   
    .data_out_num = I2S_PIN_NO_CHANGE/*-1*/,
    .data_in_num = I2S_SD
  };
  return i2s_set_pin(I2S_PORT, &pin_config);
}

