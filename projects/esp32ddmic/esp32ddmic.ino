// I2S driver
#include <driver/i2s.h>
 
// INMP441 I2S pin assignment
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
 
// I2S processor
#define I2S_PORT I2S_NUM_0




#define USE_BLUETOOTH
#if defined(USE_BLUETOOTH)
  // ESP32 Bluetooth with name  ESP32
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));
#else
  // ESP32 WiFi
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#endif


PlotterDDLayer* plotterLayer;
LcdDDLayer* micTabLayer;
LcdDDLayer* recTabLayer;
LcdDDLayer* playTabLayer;
LcdDDLayer* startBtnLayer;
LcdDDLayer* stopBtnLayer;
LcdDDLayer* amplifyLblLayer;
LedGridDDLayer* amplifyMeterLayer;




const char* SoundName = "recorded_sound";


const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;

const int SoundSampleRate = 8000;  // will be 16-bit per sample
const int SoundNumChannels = 1;


// 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
const int StreamBufferNumBytes = 256;
const int StreamBufferLen = StreamBufferNumBytes / 2;
int16_t StreamBuffer[StreamBufferLen];

// sound sample (16 bits) amplification
const int MaxAmplifyFactor = 20;
const int DefAmplifyFactor = 10;


void i2s_install();
void i2s_setpin();
 

DDConnectVersionTracker cvTracker(-1);
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
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  Serial.println("... DONE SETUP MIC");


  dumbdisplay.recordLayerSetupCommands();  // start recording the layout commands

  plotterLayer = dumbdisplay.createPlotterLayer(1024, 256, SoundSampleRate / StreamBufferLen);

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

  amplifyLblLayer = dumbdisplay.createLcdLayer(12, 1);
  amplifyLblLayer->pixelColor("darkred");
  amplifyLblLayer->noBackgroundColor();

  amplifyMeterLayer = dumbdisplay.createLedGridLayer(MaxAmplifyFactor, 1, 1, 2);
  amplifyMeterLayer->onColor("darkblue");
  amplifyMeterLayer->offColor("lightgray");
  amplifyMeterLayer->border(0.2, "blue");
  amplifyMeterLayer->enableFeedback("fa:rpt50");

  DDAutoPinConfigBuilder<1> builder('V');
  builder
    .addLayer(plotterLayer)
    .beginGroup('H')
      .addLayer(micTabLayer)
      .addLayer(recTabLayer)
      .addLayer(playTabLayer)
    .endGroup()
    .beginGroup('H')
      .addLayer(startBtnLayer)
      .addLayer(stopBtnLayer)
    .endGroup()
    .beginGroup('S')
      .addLayer(amplifyLblLayer)  
      .addLayer(amplifyMeterLayer)
    .endGroup();  
  dumbdisplay.configAutoPin(builder.build());

  dumbdisplay.playbackLayerSetupCommands("esp32ddmice");  // playback the stored layout commands, as well as persist the layout to phone, so that can reconnect

  // set when idel handler ... here is a lambda expression
  dumbdisplay.setIdleCalback([](long idleForMillis) {
    started = false;  // if idle, e.g. disconnected, stop whatever
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
 
  int16_t samplesRead = 0;
  if (result == ESP_OK) {
    samplesRead = bytesRead / 2;  // 16 bit per sample
    if (samplesRead > 0) {
      // find the samples mean ... and amplify the sound sample, by simply multiple it by some "amplify factor"
      float sumVal = 0;
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = StreamBuffer[i];
        if (amplifyFactor > 1) {
          val = amplifyFactor * val;
          if (val > 32700) {
            val = 32700;
          } else if (val < -32700) {
            val = -32700;
          }
          StreamBuffer[i] = val;
        }
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
        // start sending sound, and get the assigned "shunk id"
        soundChunkId = dumbdisplay.streamSound16(SoundSampleRate, SoundNumChannels); // sound is 16 bits per sample
        dumbdisplay.writeComment(String("STARTED mic streaming with chunk id [") + soundChunkId + "]");
      } else if (what == 2) {
        soundChunkId = dumbdisplay.saveSoundChunked16(SoundName, SoundSampleRate, SoundNumChannels);
        dumbdisplay.writeComment(String("STARTED record streaming with chunk id [") + soundChunkId + "]");
      }
      streamingMillis = millis();
      streamingTotalSampleCount = 0;
    }
  }

  if (result == ESP_OK) {
    //int16_t samplesRead = bytesRead / 2;  // 16 bit per sample
    // if (amplifyFactor > 1) {
    //   // amplify the sound sample, by simply multiple it by some "amplify factor"
    //   for (int i = 0; i < samplesRead; ++i) {
    //     int32_t val = Buffer[i];
    //     val = amplifyFactor * val;
    //     if (val > 32700) {
    //       val = 32700;
    //     } else if (val < -32700) {
    //       val = -32700;
    //     }
    //     Buffer[i] = val;
    //   }
    // }
    if (soundChunkId != -1) {
      // send sound samples read
      bool isFinalChunk = !started;  // it is the final chink if justed turned to stop
      dumbdisplay.sendSoundChunk16(soundChunkId, StreamBuffer, samplesRead, isFinalChunk);
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

