// *** experimental ***

#define AUTO_START_RTSP true

#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());
DDMasterResetPassiveConnectionHelper pdd(dumbdisplay);


#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNFaceDetectionRecognition.h"
#include "VideoStreamOverlay.h"
#include "AmebaFatFS.h"

#define CHANNELVID 0   // Channel for RTSP streaming
#define CHANNELJPEG 1  // Channel for taking snapshots
#define CHANNELNN 3    // RGB format video for NN only available on channel 3

// Customised resolution for NN
#define NNWIDTH 576
#define NNHEIGHT 320

// Pin Definition
#define RED_LED 3
#define GREEN_LED 4
#define BUTTON_PIN 5

// Select the maximum number of snapshots to capture
#define MAX_UNKNOWN_COUNT 5

VideoSetting configVID(VIDEO_FHD, CAM_FPS, VIDEO_H264, 0);
VideoSetting configJPEG(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
NNFaceDetectionRecognition facerecog;
RTSP rtsp;
StreamIO videoStreamer(1, 1);
StreamIO videoStreamerFDFR(1, 1);
StreamIO videoStreamerRGBFD(1, 1);

char ssid[] = "TrevorWireless";  // your network SSID (name)
char pass[] = "joysftrevorws";   // your network password
int status = WL_IDLE_STATUS;

bool buttonState = false;
uint32_t img_addr = 0;
uint32_t img_len = 0;
bool regFace = true;
bool unknownDetected = false;
bool roundBegan = false;
int unknownCount = 0;

// File Initialization
AmebaFatFS fs;

String ip;
long lastOSDMillis = 0;


void setup() {
  // GPIO Initialization
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  // Attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(2000);
  }

  ip = WiFi.localIP().get_address();

  // Configure camera video channels with video format information
  Camera.configVideoChannel(CHANNELVID, configVID);
  Camera.configVideoChannel(CHANNELJPEG, configJPEG);
  Camera.configVideoChannel(CHANNELNN, configNN);
  Camera.videoInit();

  // Configure RTSP with corresponding video format information
  rtsp.configVideo(configVID);
  rtsp.begin();

  // Configure Face Recognition model
  facerecog.configVideo(configNN);
  facerecog.modelSelect(FACE_RECOGNITION, NA_MODEL, DEFAULT_SCRFD, DEFAULT_MOBILEFACENET);
  facerecog.begin();
  facerecog.setResultCallback(FRPostProcess);

  // Configure StreamIO object to stream data from video channel to RTSP
  videoStreamer.registerInput(Camera.getStream(CHANNELVID));
  videoStreamer.registerOutput(rtsp);
  if (videoStreamer.begin() != 0) {
    Serial.println("StreamIO link start failed");
  }

  // Configure StreamIO object to stream data from RGB video channel to face detection
  videoStreamerRGBFD.registerInput(Camera.getStream(CHANNELNN));
  videoStreamerRGBFD.setStackSize();
  videoStreamerRGBFD.setTaskPriority();
  videoStreamerRGBFD.registerOutput(facerecog);
  if (videoStreamerRGBFD.begin() != 0) {
    Serial.println("StreamIO link start failed");
  }

  // Start data stream from video channel
  Camera.channelBegin(CHANNELVID);
  Camera.channelBegin(CHANNELJPEG);
  Camera.channelBegin(CHANNELNN);

  // Start OSD drawing on RTSP video channel
  OSD.configVideo(CHANNELVID, configVID);
#if SUPPORT_DD
  OSD.configTextSize(CHANNELVID, 32, 64);
#endif  
  OSD.begin();

  videoStreamer.pause();  
}

void loop() {
  pdd.loop([]() {
    initializeDD();
  },
           []() {
             updateDD(!pdd.firstUpdated());
           },
           []() {
             deinitializeDD();
           });
}

// User callback function for post processing of face recognition results
void FRPostProcess(std::vector<FaceRecognitionResult> results) {
  uint16_t im_h = configVID.height();
  uint16_t im_w = configVID.width();

  //printf("Total number of faces detected = %d\r\n", facerecog.getResultCount());
  OSD.createBitmap(CHANNELVID);

  if (facerecog.getResultCount() > 0) {
    for (int i = 0; i < facerecog.getResultCount(); i++) {
      FaceRecognitionResult item = results[i];
      // Result coordinates are floats ranging from 0.00 to 1.00
      // Multiply with RTSP resolution to get coordinates in pixels
      int xmin = (int)(item.xMin() * im_w);
      int xmax = (int)(item.xMax() * im_w);
      int ymin = (int)(item.yMin() * im_h);
      int ymax = (int)(item.yMax() * im_h);

      uint32_t osd_color;
      if (String(item.name()) == String("unknown")) {
        osd_color = OSD_COLOR_RED;
        if (regFace == false) {
          unknownDetected = true;
          unknownCount++;
          if (unknownCount < (MAX_UNKNOWN_COUNT + 1)) {                 // Ensure number of snapshots under MAX_UNKNOWN_COUNT
            facerecog.registerFace("Stranger" + String(unknownCount));  // Register under named Stranger <No.> to prevent recapture of same unrecognised person twice
            fs.begin();
            File file = fs.open(String(fs.getRootPath()) + "Stranger" + String(unknownCount) + ".jpg");  // Capture snapshot of stranger under name Stranger <No.>
            delay(1000);
            Camera.getImage(CHANNELJPEG, &img_addr, &img_len);
            file.write((uint8_t *)img_addr, img_len);
            file.close();
            fs.end();
          }
        }
      } else {
        osd_color = OSD_COLOR_GREEN;
      }

      // Draw boundary box
      //printf("Face %d name %s:\t%d %d %d %d\n\r", i, item.name(), xmin, xmax, ymin, ymax);
      OSD.drawRect(CHANNELVID, xmin, ymin, xmax, ymax, 3, osd_color);

      // Print identification text above boundary box
      char text_str[40];
      snprintf(text_str, sizeof(text_str), "Face:%s", item.name());
      OSD.drawText(CHANNELVID, xmin, ymin - OSD.getTextHeight(CHANNELVID), text_str, osd_color);
    }
    if ((regFace == false) && (unknownDetected == true)) {
      // RED LED remain lit up when unknown faces detected
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else if ((regFace == false) && (unknownDetected == false)) {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }
    unknownDetected = false;
  }
  OSD.update(CHANNELVID);
  lastOSDMillis = millis();
}

RtspClientDDLayer *rtspClient;
SelectionDDLayer *startStopSelection;
LcdDDLayer *registerButton;
LedGridDDLayer *led;
bool rtspStarted;
long lastMillis;
int counter = 0;

void startStopRtsp(bool start) {
  if (start) {
    String url = "rtsp://" + ip;
    videoStreamer.resume();  
    rtspClient->start(url);
    startStopSelection->select();
    rtspStarted = true;
  } else {
    rtspClient->stop();
    videoStreamer.pause();  
    startStopSelection->deselect();
    rtspStarted = false;
  }
}

void initializeDD() {
  rtspClient = dumbdisplay.createRtspClient(160, 90);
  rtspClient->border(3, "blue", "round");
  rtspClient->padding(3);

  startStopSelection = dumbdisplay.createSelectionLayer(2, 1);
  startStopSelection->highlightBorder(true, DD_COLOR_blue);
  startStopSelection->highlightBorder(false, DD_COLOR_gray);
  startStopSelection->text("📺");

  registerButton = dumbdisplay.createLcdLayer(8, 1);
  registerButton->border(2, DD_COLOR_green);
  registerButton->writeCenteredLine("Register");
  registerButton->enableFeedback("f:keys");


  led = dumbdisplay.createLedGridLayer();
  const char *color = "darkblue";
  switch (counter++ % 3) {
    case 0:
      color = "darkred";
      break;
    case 1:
      color = "darkgreen";
      break;
  }
  led->onColor(color);
  led->offColor("gray");
  led->enableFeedback("fl");

  dumbdisplay.configAutoPin(DDAutoPinConfig('V')
                              .addLayer(rtspClient)
                              .beginGroup('H')
                              .addLayer(startStopSelection)
                              .addLayer(registerButton)
                              .addLayer(led)
                              .endGroup()
                              .build());

  // String url = "rtsp://" + ip;
  // dumbdisplay.log("* RTSP: " + url);
  // rtspClient->start(url);

  startStopRtsp(AUTO_START_RTSP);
  lastMillis = 0;
}

void updateDD(bool isFirstUpdate) {
  bool osdIdle = false;
  if (lastOSDMillis != 0 && (millis() - lastOSDMillis) >= 1000) {
    osdIdle = true;
    lastOSDMillis = 0;
  }
  if (osdIdle) {
    OSD.createBitmap(CHANNELVID);
    OSD.update(CHANNELVID);
  }

  if (startStopSelection->getFeedback() != NULL) {
    startStopRtsp(!rtspStarted);
  }

  const DDFeedback *regFB = registerButton->getFeedback();
  if (regFB != NULL) {
    if (regFB->text.length() > 0) {
      String name = regFB->text;
      facerecog.registerFace(name);
    }
  }


  bool blink = false;
  if ((millis() - lastMillis) >= 1000) {
    blink = true;
    lastMillis = millis();
  }
  if (blink) {
    if (led != NULL) {
      led->toggle();
    }
  }
}

void deinitializeDD() {
}
