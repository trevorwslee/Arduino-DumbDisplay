// I2S driver
#include <driver/i2s.h>
 
// INMP441 I2S pin assignment
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
 
// I2S processor
#define I2S_PORT I2S_NUM_0


// ESP32 Bluetooth with name  ESP32
#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));


LcdDDLayer* micTabLayer;
LcdDDLayer* recTabLayer;
LcdDDLayer* playTabLayer;
LcdDDLayer* startBtnLayer;
LcdDDLayer* stopBtnLayer;
LedGridDDLayer* amplifyMeterLayer;
//DDValueRecord<bool> startStopRecord(false, true);


const char* SoundName = "recorded_sound";

const int SoundSampleRate = 8000;  // will be 16-bit per sample
const int SoundNumChannels = 1;

// 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
const int BufferNumBytes = 256;
const int BufferLen = BufferNumBytes / 2;
int16_t Buffer[BufferLen];

// sound sample (16 bits) amplification
const int MaxAmplifyFactor = 20;


void i2s_install();
void i2s_setpin();
 
void setup() {

  Serial.begin(115200);

  Serial.println("SETUP MIC ...");

  // set up I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  Serial.println("... DONE SETUP MIC");

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
  startBtnLayer->border(2, "darkgreen", "round");
  startBtnLayer->enableFeedback("fl");
  stopBtnLayer = dumbdisplay.createLcdLayer(12, 3);
  stopBtnLayer->border(2, "darkgreen", "round");
  stopBtnLayer->enableFeedback("fl");

  amplifyMeterLayer = dumbdisplay.createLedGridLayer(MaxAmplifyFactor, 1, 1, 2);
  amplifyMeterLayer->border(0.2, "blue");
  amplifyMeterLayer->enableFeedback("fa:rpt50");
    // turnSpeedLayer->border(0.2, "darkgray");
    // turnSpeedLayer->offColor("lightgray");
    // turnSpeedLayer->setFeedbackHandler(FeedbackHandler, "fa:rpt50");


  // // set "feedback" handler lambda expression ... i.e. it can be clicked
  // startStopLayer->setFeedbackHandler(
  //   [](DDLayer* layer, DDFeedbackType type, const DDFeedback& feedback) {
  //     startStopRecord = !startStopRecord;  // when clicked, toggle "start/stop record" 
  //   },
  //    "fl");

  DDAutoPinConfigBuilder<1> builder('V');
  builder
    .beginGroup('H')
      .addLayer(micTabLayer)
      .addLayer(recTabLayer)
      .addLayer(playTabLayer)
    .endGroup()
    .beginGroup('H')
      .addLayer(startBtnLayer)
      .addLayer(stopBtnLayer)
    .endGroup()
    .addLayer(amplifyMeterLayer);  
  dumbdisplay.configAutoPin(builder.build());
}



bool initialized = false;
int what = 1;  // 1: mic; 2: record; 3: play
bool started = false;
int amplifyFactor = 10;
int soundChunkId = -1; // when started sending sound [chunk], the allocated "chunk id"

void loop() {

  bool updateTab = false;
  bool updateStartStop = false;
  bool updateAmplifyFactor = false;
  if (initialized) {
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
  } else {
    updateTab = true;
    updateStartStop = true;
    updateAmplifyFactor = true;
    initialized = true;
  }

  if (updateTab) {
    const char* micColor = what == 1 ? "blue" : "gray";
    const char* recColor = what == 2 ? "blue" : "gray";
    const char* playColor = what == 3 ? "blue" : "gray";
    micTabLayer->border(1, micColor);
    micTabLayer->pixelColor(micColor);
    recTabLayer->border(1, recColor);
    recTabLayer->pixelColor(recColor);
    playTabLayer->border(1, playColor);
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

  // bool started = startStopRecord.get();  // get the start/stop value 
  // if (startStopRecord.record()) {  // record and check if the start/stop value changed since it's last record of changed state
  //   const char* what;
  //   if (started) {
  //     // changed to started
  //     what = "STOP";
  //   } else {
  //     // changed to started
  //     what = "START";
  //   }
  //   startStopLayer->writeCenteredLine(what, 1);
  // }

  if (started) {
    if (soundChunkId == -1) {
      // if no allocated "chunk id" (i.e. sending sound not started)
      if (what == 1) {
        // start sending sound, and get the assigned "shunk id"
        soundChunkId = dumbdisplay.streamSound16(SoundSampleRate, SoundNumChannels); // sound is 16 bits per sample
        dumbdisplay.writeComment(String("STARTED mic streaming with chunk id [") + soundChunkId + "]");
      } else if (what == 2) {
        soundChunkId = dumbdisplay.saveSoundChunked16(SoundName, SoundSampleRate, SoundNumChannels);
        dumbdisplay.writeComment(String("STARTED record streaming with chunk id [") + soundChunkId + "]");
      }
    }
  }

  // read I2S data and place in data buffer
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, portMAX_DELAY);
 
  if (result == ESP_OK) {
    int16_t samplesRead = bytesRead / 2;  // 16 bit per sample
    if (amplifyFactor > 1) {
      // amplify the sound sample, by simply multiple it by some "amplify factor"
      for (int i = 0; i < samplesRead; ++i) {
        int32_t val = Buffer[i];
        val = amplifyFactor * val;
        if (val > 32700) {
          val = 32700;
        } else if (val < -32700) {
          val = -32700;
        }
        Buffer[i] = val;
      }
    }
    if (soundChunkId != -1) {
      // send sound samples read
      bool isFinalChunk = !started;  // it is the final chink if justed turned to stop
      dumbdisplay.sendSoundChunk16(soundChunkId, Buffer, samplesRead, isFinalChunk);
      if (isFinalChunk) {
        dumbdisplay.writeComment(String("DONE streaming with chunk id [") + soundChunkId + "]");
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
    .dma_buf_count = 8,
    .dma_buf_len = 1024,//BufferLen,
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

