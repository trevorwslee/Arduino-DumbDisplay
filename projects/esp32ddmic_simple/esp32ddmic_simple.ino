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


LcdDDLayer* startStopLayer;
DDValueRecord<bool> startStopRecord(false, true);


const int SoundSampleRate = 8000;  // will be 16-bit per sample
const int SoundNumChannels = 1;

// 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
const int BufferNumBytes = 256;
const int BufferLen = BufferNumBytes / 2;
int16_t Buffer[BufferLen];

// sound sample (16 bits) amplification
const int AmplifyFactor = 5;  // <= 1 if no amplification of the sound sample


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


  // create "START/STOP" lcd layer, acting as a button
  startStopLayer = dumbdisplay.createLcdLayer(16, 3);

  // set "feedback" handler lambda expression ... i.e. it can be clicked
  startStopLayer->setFeedbackHandler(
    [](DDLayer* layer, DDFeedbackType type, const DDFeedback& feedback) {
      startStopRecord = !startStopRecord;  // when clicked, toggle "start/stop record" 
    },
     "fl");
}



int soundChunkId = -1; // when started sending sound [chunk], the allocated "chunk id"

void loop() {
  DDYield();  // give DumbDisplay has a chance to do it's work (e.g. check for "feedback")

  bool started = startStopRecord.get();  // get the start/stop value 
  if (startStopRecord.record()) {  // record and check if the start/stop value changed since it's last record of changed state
    const char* what;
    if (started) {
      // changed to started
      what = "STOP";
    } else {
      // changed to started
      what = "START";
    }
    startStopLayer->writeCenteredLine(what, 1);
  }

  if (started) {
    if (soundChunkId == -1) {
      // if no allocated "chunk id" (i.e. sending sound not started), start sending sound, and get the assigned "shunk id"
      soundChunkId = dumbdisplay.streamSound16(SoundSampleRate, SoundNumChannels); // sound is 16 bits per sample
      dumbdisplay.writeComment(String("started stream with chunk id [") + soundChunkId + "]");
    }
  }

  // read I2S data and place in data buffer
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, portMAX_DELAY);
 
  if (result == ESP_OK) {
    if (started) {
      int16_t samplesRead = bytesRead / 2;  // 16 bit per sample
      if (AmplifyFactor > 1) {
        // amplify the sound sample, by simply multiple it by some "amplify factor"
        for (int i = 0; i < samplesRead; ++i) {
          int32_t val = Buffer[i];
          val = AmplifyFactor * val;
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
          soundChunkId = -1;
        }
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
    .dma_buf_len = BufferLen,
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

