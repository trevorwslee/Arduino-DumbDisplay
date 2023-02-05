


#include <esp_now.h>
#include <WiFi.h>



#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)

  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //dumbdisplay.writeComment(String("received command (") + String(len) + ")");
  if (!receivedNewCommand) {
    memcpy(&ReceivedPacket, incomingData, sizeof(ReceivedPacket));
    const char* commandTarget = ReceivedPacket.commandTarget;
    const char* commandAction = ReceivedPacket.commandAction;
    statusLayer->clear();
    statusLayer->setCursor(0, 0);
    statusLayer->penColor("black");
    statusLayer->println(String("* Received command for [") + commandTarget + "] to [" + commandAction + "]");
    receivedNewCommand = true;
  } else {
    statusLayer->println("* BUSY ... will ignore newly received command");

  }
}

void setup() {

  dumbdisplay.connect();

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
  //statusLayer->border(5, "darkgreen");
  statusLayer->backgroundColor("white");
  statusLayer->penColor("darkblue");
  //statusLayer->setTextFont("DL::Roboto");

  // pin to virtual print frame, which is by default 100x100
  dumbdisplay.pinLayer(statusLayer, 0, 0, 100, 35);
  dumbdisplay.pinAutoPinLayers(DD_AP_HORI, 0, 35, 100, 65);

  // DDAutoPinConfigBuilder<1> builder('V');
  // builder.addLayer(statusLayer);
  // builder.addRemainingGroup('H');
  // dumbdisplay.configAutoPin(builder.build());

  lockedTunnel = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-locked.png", LockImageFileName);
  unlockedTunnel = dumbdisplay.createImageDownloadTunnel("https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png", UnlockImageFileName);

  dumbdisplay.writeComment("... initialized");
}

struct KnownCommandLayer {
  String commandTarget;
  DDLayer* layer;
};

const int MaxKnownCommandLayers = 4;
KnownCommandLayer knownCommandLayers[MaxKnownCommandLayers];
int knownCommandLayerCount = 0;

DDLayer* findCommandLayer(const String& commandTarget, const String& commandAction) {
  //String key = commandTarget + ":" + commandAction;
  for (int i = 0; i < knownCommandLayerCount; i++) {
    if (knownCommandLayers[i].commandTarget == commandTarget) {
      return knownCommandLayers[i].layer;
    }
  }
  if (knownCommandLayerCount >= MaxKnownCommandLayers) {
    return NULL;
  }
  DDLayer* layer = NULL;
  if ((commandTarget == "kitchen" || commandTarget == "living room") &&
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
    layer = ledLayer;
  } else if ((commandTarget == "bedroom" || commandTarget == "balcony") &&
             (commandAction == "lock" || commandAction == "unlock")) {
    LcdDDLayer* label = dumbdisplay.createLcdLayer(12, 1);
    label->border(2, "green");
    label->backgroundColor("blue");
    label->pixelColor("white");
    label->writeCenteredLine(commandTarget);
    GraphicalDDLayer* graphicallayer = dumbdisplay.createGraphicalLayer(200, 200);
    graphicallayer->backgroundColor("lightgray");
    graphicallayer->setTextSize(12);
    graphicallayer->setTextColor("red");
    //graphicallayer->print(commandTarget);
    dumbdisplay.addRemainingAutoPinConfig(DD_AP_VERT_2(label->getLayerId(), graphicallayer->getLayerId()));
    layer = graphicallayer;
  }
  if (layer != NULL) {
    knownCommandLayers[knownCommandLayerCount].commandTarget = commandTarget;
    knownCommandLayers[knownCommandLayerCount].layer = layer;
    knownCommandLayerCount += 1;
  }
  return layer;
}

bool handleCommand(const String& commandTarget, const String& commandAction) {
  DDLayer* layer = findCommandLayer(commandTarget, commandAction);
  if (layer != NULL) {
    if (commandAction == "on") {
      LedGridDDLayer* ledLayer = (LedGridDDLayer*) layer;
      ledLayer->turnOn();
      return true;
    }
    if (commandAction == "off") {
      LedGridDDLayer* ledLayer = (LedGridDDLayer*) layer;
      ledLayer->turnOff();
      return true;
    }
    if (commandAction == "lock") {
      GraphicalDDLayer* graphicalLayer = (GraphicalDDLayer*) layer;
      graphicalLayer->drawImageFileFit(LockImageFileName);
      return true;
    }
    if (commandAction == "unlock") {
      GraphicalDDLayer* graphicalLayer = (GraphicalDDLayer*) layer;
      graphicalLayer->drawImageFileFit(UnlockImageFileName);
      return true;
    }
  } 
  return false;
}

long lastShowIdleMillis = 0;

void loop() {
  if (receivedNewCommand) {
    const char* commandTarget = ReceivedPacket.commandTarget;
    const char* commandAction = ReceivedPacket.commandAction;
    if (handleCommand(commandTarget, commandAction)) {
      statusLayer->penColor("darkgreen");
      statusLayer->println(String("- Handled command for [") + commandTarget + "] to [" + commandAction + "]");
    } else {
      statusLayer->penColor("red");
      statusLayer->println(String("- Not handled command for [") + commandTarget + "] to [" + commandAction + "]");
    }
    receivedNewCommand = false;
} else  {
    long nowMillis = millis();
    if ((nowMillis - lastShowIdleMillis) >= 5000) {
#if defined(DD_USING_WIFI)
      Serial.println(String("MAC is ") + WiFi.macAddress());
#endif
      dumbdisplay.writeComment(String("Idle ... client MAC is ") + WiFi.macAddress());
      lastShowIdleMillis = nowMillis;
    }
  }
}


