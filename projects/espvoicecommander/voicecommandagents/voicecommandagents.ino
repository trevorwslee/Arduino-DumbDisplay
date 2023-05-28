


#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)

  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

#endif


#if defined(ESP32)

  #include <WiFi.h>
  #include <esp_now.h>

#else

  #include <ESP8266WiFi.h>
  #include <espnow.h>
  #define ESP_OK 0

#endif





const char* LockImageFileName = "lock-locked.png";
const char* UnlockImageFileName = "lock-unlocked.png";

GraphicalDDLayer* statusLayer;
SimpleToolDDTunnel *lockedTunnel;
SimpleToolDDTunnel *unlockedTunnel;


// define a structure as ESP Now packet (the same as server)
struct ESPNowCommandPacket {
  char commandTarget[32];
  char commandAction[32];
};


ESPNowCommandPacket ReceivedPacket;
volatile bool receivedNewCommand = false;

#if defined(ESP32)
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#else
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
#endif  
  //dumbdisplay.writeComment(String("received command (") + String(len) + ")");
  if (!receivedNewCommand) {
    memcpy(&ReceivedPacket, incomingData, sizeof(ReceivedPacket));
    const char* commandTarget = ReceivedPacket.commandTarget;
    const char* commandAction = ReceivedPacket.commandAction;
    statusLayer->clear();
    statusLayer->setCursor(0, 0);
    statusLayer->penColor(DD_COLOR_black);
    statusLayer->println(String("* Received command for [") + commandTarget + "] to [" + commandAction + "]");
    receivedNewCommand = true;
  } else {
    statusLayer->println("* BUSY ... will ignore newly received command");

  }
}

void setup() {

  Serial.println("*****");
  Serial.println(String("* agent MAC is ") + WiFi.macAddress());
  Serial.println("*****");

  dumbdisplay.connect();  // explicitly connection, so that can write comments to DD

  dumbdisplay.writeComment("initializing ...");
  dumbdisplay.writeComment(String("... ") + WiFi.macAddress() + " ...");

#if defined(DD_USING_WIFI)
  // Set device as a Wi-Fi Station and also a Access Point
  WiFi.mode(WIFI_AP_STA);
#else
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
#endif

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    dumbdisplay.writeComment("Error initializing ESP-NOW");
  }  

  // Register "receive callback"
  if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
    dumbdisplay.writeComment("Error registering receive callback");
  }

  statusLayer = dumbdisplay.createGraphicalLayer(300, 80);
  statusLayer->margin(5);
  statusLayer->border(5, "blue");
  statusLayer->backgroundColor("white");
  statusLayer->penColor("darkblue");
  
  // pin to virtual print frame, which is by default 100x100
  dumbdisplay.pinLayer(statusLayer, 0, 0, 100, 35);
  dumbdisplay.pinAutoPinLayers(DD_AP_HORI, 0, 35, 100, 65);

  lockedTunnel = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", LockImageFileName);
  unlockedTunnel = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", UnlockImageFileName);

  dumbdisplay.writeComment("... initialized");
}

struct KnownCommandLayer {
  String commandTarget;
  String commandType;  // onoff / lockunlock
  DDLayer* layer;
  LedGridDDLayer* getOnOffLayer() {
    if (commandType == "onoff") {
      return (LedGridDDLayer*) layer;
    } else {
      return NULL;
    }
  }
  GraphicalDDLayer* getLockUnlockLayer() {
    if (commandType == "lockunlock") {
      return (GraphicalDDLayer*) layer;
    } else {
      return NULL;
    }
  }
};

const int MaxKnownCommandLayers = 5;
KnownCommandLayer KnownCommandLayers[MaxKnownCommandLayers];
int KnownCommandLayerCount = 0;

KnownCommandLayer* toKnownCommandLayer(const String& commandTarget, const String& commandAction) {
  for (int i = 0; i < KnownCommandLayerCount; i++) {
    if (KnownCommandLayers[i].commandTarget == commandTarget) {
      return KnownCommandLayers + i;
    }
  }
  if (KnownCommandLayerCount >= MaxKnownCommandLayers) {
    return NULL;
  }
  const char* commandType = NULL;
  DDLayer* layer = NULL;
  if ((commandTarget == "kitchen" || commandTarget == "living room" || commandTarget == "fan") &&
      (commandAction == "on" || commandAction == "off")) {
    LcdDDLayer* label = dumbdisplay.createLcdLayer(12, 1);
    label->border(2, "green");
    label->backgroundColor("blue");
    label->pixelColor("white");
    label->writeCenteredLine(commandTarget);
    LedGridDDLayer* ledLayer = dumbdisplay.createLedGridLayer();
    ledLayer->onColor("darkgreen");
    ledLayer->offColor("lightgray");
    dumbdisplay.addRemainingAutoPinConfig(DD_AP_VERT_2(label->getLayerId(), ledLayer->getLayerId()));
    commandType = "onoff";
    layer = ledLayer;
  } else if ((commandTarget == "bedroom" || commandTarget == "balcony") &&
             (commandAction == "lock" || commandAction == "unlock")) {
    LcdDDLayer* label = dumbdisplay.createLcdLayer(12, 1);
    label->border(2, "green");
    label->backgroundColor("blue");
    label->pixelColor("white");
    label->writeCenteredLine(commandTarget);
    GraphicalDDLayer* graphicallayer = dumbdisplay.createGraphicalLayer(200, 200);
    graphicallayer->backgroundColor("white");
    graphicallayer->setTextSize(12);
    graphicallayer->setTextColor("red");
    dumbdisplay.addRemainingAutoPinConfig(DD_AP_VERT_2(label->getLayerId(), graphicallayer->getLayerId()));
    commandType = "lockunlock";
    layer = graphicallayer;
  }
  if (layer != NULL) {
    KnownCommandLayers[KnownCommandLayerCount].commandTarget = commandTarget;
    KnownCommandLayers[KnownCommandLayerCount].commandType = commandType;;
    KnownCommandLayers[KnownCommandLayerCount].layer = layer;
    KnownCommandLayerCount += 1;
    return KnownCommandLayers + KnownCommandLayerCount - 1;
  } else {
    return NULL;
  }
}

bool handleCommand(const String& commandTarget, const String& commandAction) {
  KnownCommandLayer* knownCommandLayer = toKnownCommandLayer(commandTarget, commandAction);
  if (knownCommandLayer != NULL) {
    if (commandAction == "on") {
      LedGridDDLayer* ledLayer = knownCommandLayer->getOnOffLayer();
      if (ledLayer != NULL) {
        ledLayer->turnOn();
        return true;
      }
    }
    if (commandAction == "off") {
      LedGridDDLayer* ledLayer = knownCommandLayer->getOnOffLayer();
      if (ledLayer != NULL) {
        ledLayer->turnOff();
        return true;
      }
    }
    if (commandAction == "lock") {
      GraphicalDDLayer* graphicalLayer = knownCommandLayer->getLockUnlockLayer();
      if (graphicalLayer != NULL) {
        graphicalLayer->drawImageFileFit(LockImageFileName);
        return true;
      }
    }
    if (commandAction == "unlock") {
      GraphicalDDLayer* graphicalLayer = knownCommandLayer->getLockUnlockLayer();
      if (graphicalLayer != NULL) {
        graphicalLayer->drawImageFileFit(UnlockImageFileName);
        return true;
      }
    }
  } 
  return false;
}

long lastShowIdleMillis = 0;

void loop() {
  if (receivedNewCommand) {
      lastShowIdleMillis = -1;  // prevent showing of MAC
    const char* commandTarget = ReceivedPacket.commandTarget;
    const char* commandAction = ReceivedPacket.commandAction;
    bool handled = handleCommand(commandTarget, commandAction);
    String status = String("handled command for [") + commandTarget + "] to [" + commandAction + "]";
    if (handled) {
      statusLayer->penColor("darkgreen");
    } else {
      dumbdisplay.tone(3000, 100);
      statusLayer->penColor("red");
      status = "not " + status;
    }
    statusLayer->println(status);
    dumbdisplay.writeComment(String("- ") + status);
    receivedNewCommand = false;
} else if (lastShowIdleMillis != -1)  {
    long nowMillis = millis();
    if ((nowMillis - lastShowIdleMillis) >= 5000) {
#if defined(DD_USING_WIFI)
      Serial.println(String("agent MAC is ") + WiFi.macAddress());
#endif
      dumbdisplay.writeComment(String("agent MAC is ") + WiFi.macAddress());
      lastShowIdleMillis = nowMillis;
    }
  }
}


