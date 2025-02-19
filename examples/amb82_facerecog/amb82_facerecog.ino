// **********
// This sketch is modified from the `Examples/AmebaNN/RTSPFaceRecognition` example of the Arduino IDE board
// Realtek Ameba Boards (32-bit Arm v8M @ 500MHx) -- https://www.amebaiot.com/en/
// The board tested the sketch with is the AMB82 MINI (Ameba RTL8735B) -- https://www.amebaiot.com/en/ameba-arduino-summary/
// Notes:
// . if SD card is installed, you will have the options to persist the registered names to the SD card
// . nevertheless, only the registered names are persisted; the faces are persisted using the board's support
// . you may want to try out the faces @ https://trevorwslee.github.io/Misc/facerecog/
// **********


/**
 * IMPORTANT: set the macros WIFI_SSID and WIFI_PASSWORD below
 *
 * to run and see the result of this sketch, you will need two addition things:
 * . you will need to install Android DumbDisplay app from Play store
 *   https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay
 * . although there are several ways for microcontroller board to establish connection
 *   with DumbDisplay app, here, the simple OTG USB connection is assume;
 *   hence, you will need an OTG adaptor cable for connecting your microcontroller board
 *   to your Android phone
 * . after uploading the sketch to your microcontroller board, plug the USB cable
 *   to the OTG adaptor connected to your Android phone
 * . open the DumbDisplay app and make connection to your microcontroller board via the USB cable;
 *   hopefully, the UI is obvious enough :)
 * . for more details on DumbDisplay Arduino Library, please refer to
 *   https://github.com/trevorwslee/Arduino-DumbDisplay#readme
 * there is a related blog post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */


// *** comment out and set the following lines that define WIFI_SSID and WIFI_PASSWORD .. or define them in _secret.h
//#define WIFI_SSID           "wifi ssid"
//#define WIFI_PASSWORD       "wifi password"
#ifndef WIFI_SSID
  #include "_secret.h"
#endif


// DD (DumbDisplay) UI option macros 
#define AUTO_START_RTSP      true
#define NAME_WIDTH           12
#define REGISTERED_NAME_FILE "registered_names.dat"


// DD (DumbDisplay) global objects
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput());
DDMasterResetPassiveConnectionHelper pdd(dumbdisplay);


// DD global variables
String ip;
bool fsReady;
long lastOSDMillis = 0;

// 2nd batch of DD global variables
SelectionListLayerWrapper nameListSelectionWrapper;
bool rtspStarted;
bool namesChanged;
int selectedNameIndex;
long blinkLastMillis;
int blinkCounter = 0;


// *** original AmebaNN/RTSPFaceRecognition code (modified to work with DD UI)

#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNFaceDetectionRecognition.h"
#include "VideoStreamOverlay.h"
#include "AmebaFatFS.h"

#define CHANNELVID 0   // Channel for RTSP streaming
#define CHANNELNN 3    // RGB format video for NN only available on channel 3

// Customised resolution for NN
#define NNWIDTH 576
#define NNHEIGHT 320

VideoSetting configVID(VIDEO_FHD, CAM_FPS, VIDEO_H264, 0);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
NNFaceDetectionRecognition facerecog;
RTSP rtsp;
StreamIO videoStreamer(1, 1);
StreamIO videoStreamerRGBFD(1, 1);

char ssid[] = WIFI_SSID;       // your network SSID (name)
char pass[] = WIFI_PASSWORD;   // your network password

// File Initialization
AmebaFatFS fs;

void setup() {
  // Attempt to connect to Wifi network:
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(2000);
  }

  // Configure camera video channels with video format information
  Camera.configVideoChannel(CHANNELVID, configVID);
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
  //Camera.channelBegin(CHANNELJPEG);
  Camera.channelBegin(CHANNELNN);

  // Start OSD drawing on RTSP video channel
  OSD.configVideo(CHANNELVID, configVID);
  OSD.begin();

  // Set up for DD UI
  fsReady = fs.begin();
  ip = WiFi.localIP().get_address();
  videoStreamer.pause();  
}

// User callback function for post processing of face recognition results
void FRPostProcess(std::vector<FaceRecognitionResult> results) {
  uint16_t im_h = configVID.height();
  uint16_t im_w = configVID.width();

  bool recognizedAny = false;
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

      if (rtspStarted) {
        if (false) {
          dumbdisplay.writeComment("* detected: [" + String(item.name()) + "]");
        }
        int idx = nameListSelectionWrapper.findSelection(item.name());
        if (idx != -1) {
          nameListSelectionWrapper.select(idx, !recognizedAny);
          nameListSelectionWrapper.scrollToView(idx);
          recognizedAny = true;
        }
      }
      
      // Ensure number of snapshots under MAX_UNKNOWN_COUNT
      uint32_t osd_color;
      if (String(item.name()) == String("unknown")) {
        osd_color = OSD_COLOR_RED;
      } else {
        osd_color = OSD_COLOR_GREEN;
      }

      // Draw boundary box
      OSD.drawRect(CHANNELVID, xmin, ymin, xmax, ymax, 3, osd_color);

      // Print identification text above boundary box
      char text_str[40];
      snprintf(text_str, sizeof(text_str), "Face:%s", item.name());
      OSD.drawText(CHANNELVID, xmin, ymin - OSD.getTextHeight(CHANNELVID), text_str, osd_color);
    }
  }
  OSD.update(CHANNELVID);
  if (rtspStarted && !recognizedAny) {
    nameListSelectionWrapper.deselectAll();
  }
  lastOSDMillis = millis();
}


// *** DD UI code

void loop() {
  // Standard "passive master reset" DD UI loop
  pdd.loop(
    []() { initializeDD(); },
    []() { updateDD(!pdd.firstUpdated()); },
    []() { deinitializeDD(); }
  );
}

// DD global layer variables that will be initialized in initializeDD()
RtspClientDDLayer *rtspClient;
SelectionDDLayer *startStopSelection;
LcdDDLayer *registerButton;
LcdDDLayer *saveButton;
LedGridDDLayer *led;
SelectionListDDLayer *nameListSelection;
LcdDDLayer *scrollUpButton;
LcdDDLayer *scrollDownButton;

void syncSaveButtonUI() {
  if (namesChanged && fsReady) {
    saveButton->disabled(false);
  } else {
    saveButton->disabled(true);
  }
}

void syncRegisterButtonUI() {
  if (rtspStarted) {
    registerButton->disabled(false);
    registerButton->border(0.8, DD_COLOR_darkgreen);
    registerButton->pixelColor(DD_COLOR_darkgreen);
    registerButton->bgPixelColor(DD_COLOR_lightgreen);
    registerButton->writeCenteredLine("Register");
    registerButton->enableFeedback("f:keys");
    nameListSelectionWrapper.deselectAll();
    selectedNameIndex = -1;
  } else {
    registerButton->disabled(true);
    registerButton->border(0.8, DD_COLOR_darkred);
    registerButton->pixelColor(DD_COLOR_darkred);
    registerButton->bgPixelColor(DD_COLOR_orange);
    registerButton->writeCenteredLine("Deregister");
    registerButton->enableFeedback();
    nameListSelectionWrapper.deselectAll();
    selectedNameIndex = -1;
  }
}

bool saveRegisteredNames() {
  if (!fsReady) {
    return false;
  }
  char nameCount = nameListSelectionWrapper.getSelectionCount();
  String filePath = String(fs.getRootPath()) + REGISTERED_NAME_FILE;
  File file = fs.open(filePath);
  dumbdisplay.writeComment("* save to " + filePath + " ...");
  file.write(&nameCount, 1);
  dumbdisplay.writeComment("* saving " + String((uint8_t) nameCount) + " names:");
  for (int i = 0; i < nameCount; i++) {
    String selectionText = nameListSelectionWrapper.getSelectionText(i);
    char name[NAME_WIDTH + 1] = {0};
    selectionText.toCharArray(name, NAME_WIDTH + 1);
    file.write(name, NAME_WIDTH + 1);
    dumbdisplay.writeComment("  . saved [" + String(name) + "]");
  }
  file.close();
  namesChanged = false;
  syncSaveButtonUI();
  return true;
}

int readBackRegisteredNames() {
  if (!fsReady) {
    return 0;
  }
  nameListSelectionWrapper.removeAllSelections();
  String filePath = String(fs.getRootPath()) + REGISTERED_NAME_FILE;
  char nameCount = 0;
  if (fs.exists(filePath)) {
    dumbdisplay.writeComment("* reading from " + filePath + " ...");
    File file = fs.open(filePath);
    file.read(&nameCount, 1);
    dumbdisplay.writeComment("* reading " + String((uint8_t) nameCount) + " names:");
    for (int i = 0; i < nameCount; i++) {
      char name[NAME_WIDTH + 1] = {0};
      file.read(name, NAME_WIDTH + 1);
      nameListSelectionWrapper.addSelection(-1, name);
      dumbdisplay.writeComment("  . added [" + String(name) + "]");
    }
    file.close();
  }
  namesChanged = false;
  syncSaveButtonUI();
  return nameCount;
}

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
  syncRegisterButtonUI();
}

void onNameListStateChanged() {
  int selectionOffset = nameListSelectionWrapper.getOffset();
  int selectionCount = nameListSelectionWrapper.getSelectionCount();
  int visibleSelectionCount = 4;
  bool canScrollUp = selectionOffset > 0;
  bool canScrollDown = selectionOffset < (selectionCount - visibleSelectionCount);
  scrollUpButton->disabled(!canScrollUp);
  scrollDownButton->disabled(!canScrollDown);
}


void initializeDD() {
  rtspClient = dumbdisplay.createRtspClient(160, 90);  // 160x90 actually spells out the aspect-ratio
  rtspClient->border(2, "blue", "round");
  rtspClient->padding(2);

  startStopSelection = dumbdisplay.createSelectionLayer(2, 1);
  startStopSelection->highlightBorder(true, DD_COLOR_blue);
  startStopSelection->highlightBorder(false, DD_COLOR_gray);
  startStopSelection->text("📺");

  registerButton = dumbdisplay.createLcdLayer(10, 1);

  saveButton = dumbdisplay.createLcdLayer(2, 1);  
  saveButton->border(1, DD_COLOR_red);
  saveButton->writeCenteredLine("💾");
  saveButton->enableFeedback("fl");

  nameListSelection = nameListSelectionWrapper.initializeLayer(dumbdisplay, NAME_WIDTH, 1, 2, 2);
  nameListSelectionWrapper.setListStateChangedCallback(onNameListStateChanged);
  nameListSelection->border(1, DD_COLOR_black);

  scrollUpButton = dumbdisplay.createLcdLayer(2, 3);
  scrollUpButton->border(1, DD_COLOR_navy);
  scrollUpButton->writeCenteredLine("⏪", 1);
  scrollUpButton->enableFeedback("f");
 
  scrollDownButton = dumbdisplay.createLcdLayer(2, 3);
  scrollDownButton->border(1, DD_COLOR_navy);
  scrollDownButton->writeCenteredLine("⏩", 1);
  scrollDownButton->enableFeedback("f");

  led = dumbdisplay.createLedGridLayer();
  const char *color = "darkblue";
  switch (blinkCounter++ % 3) {
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
      .addLayer(saveButton)
      .addLayer(led)
    .endGroup()
    .beginGroup('H')
      .addLayer(scrollUpButton)
      .addLayer(nameListSelection)
      .addLayer(scrollDownButton)
    .endGroup()
    .build());

  startStopRtsp(AUTO_START_RTSP);

  int nameCount = 0;
  if (fsReady) {
    nameCount = readBackRegisteredNames();
    dumbdisplay.writeComment("* read back " + String(nameCount) + " names");
    if (nameCount > 0) {
      for (int i = 0; i < nameCount; i++) {
        String name = nameListSelectionWrapper.getSelectionText(i);
        dumbdisplay.writeComment(" . [" + name + "]");
      }
    }
  } else {
    dumbdisplay.writeComment("!!! FS not ready !!!");
    saveButton->visible(false);
  }

  if (nameCount == 0) {
    facerecog.resetRegisteredFace();
    dumbdisplay.writeComment("* reset faces");
  } else {
    facerecog.restoreRegisteredFace();
    dumbdisplay.writeComment("* restored faces");
  }

  blinkLastMillis = 0;
}

void updateDD(bool isFirstUpdate) {
  bool osdIdle = false;
  if (lastOSDMillis != 0 && (millis() - lastOSDMillis) >= 1000) {
    osdIdle = true;
    lastOSDMillis = 0;
  }
  if (osdIdle) {
    if (rtspStarted) {
      nameListSelectionWrapper.deselectAll();
    }
    OSD.createBitmap(CHANNELVID);
    OSD.update(CHANNELVID);
  }

  if (scrollUpButton->getFeedback() != NULL) {
    nameListSelectionWrapper.decrementOffset();
  }
  if (scrollDownButton->getFeedback() != NULL) {
    nameListSelectionWrapper.incrementOffset();
  }

  if (startStopSelection->getFeedback() != NULL) {
    startStopRtsp(!rtspStarted);
  }

  if (saveButton->getFeedback() != NULL) {
    if (saveRegisteredNames()) {
      facerecog.backupRegisteredFace();
      dumbdisplay.writeComment("* backed up faces");
    }
  }

  const DDFeedback *regFB = registerButton->getFeedback();
  if (regFB != NULL) {
    if (rtspStarted) {
      if (regFB->text.length() > 0) {
        String name = regFB->text;
        if (name.length() > NAME_WIDTH) {
          name = name.substring(0, NAME_WIDTH);
        }
        if (true) {
          dumbdisplay.writeComment("+ register: [" + name + "]");
        }
        facerecog.registerFace(name);
        int index = nameListSelectionWrapper.addSelection(-1, name);
        nameListSelectionWrapper.scrollToView(index);
      }
    } else {
      if (selectedNameIndex != -1) {
        String name = nameListSelectionWrapper.getSelectionText(selectedNameIndex);
        if (true) {
          dumbdisplay.writeComment("- deregister: [" + name + "]");
        }
        facerecog.removeFace(name);
        nameListSelectionWrapper.removeSelection(selectedNameIndex);
        registerButton->disabled(true);
      }
    }  
    namesChanged = true;
    syncSaveButtonUI();
  }

  if (rtspStarted) {
  } else {
    const DDFeedback *nlFB = nameListSelection->getFeedback();
    if (nlFB != NULL) {
      int x = nlFB->x;
      int y = nlFB->y;
      nameListSelection->flashArea(x, y);
      selectedNameIndex = nameListSelectionWrapper.getSelectionIndexFromView(x, y);
      nameListSelectionWrapper.select(selectedNameIndex);
      registerButton->disabled(false);
    }
  }

  bool blink = false;
  if ((millis() - blinkLastMillis) >= 1000) {
    blink = true;
    blinkLastMillis = millis();
  }
  if (blink) {
    if (led != NULL) {
      led->toggle();
    }
  }
}

void deinitializeDD() {
}
