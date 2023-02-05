


#include <esp_now.h>
#include <WiFi.h>



#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)

  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));

#else

  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());

#endif


GraphicalDDLayer* statusLayer;


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
  statusLayer->border(5, "darkgreen", "round", 2);
  //statusLayer->border(5, "darkgreen");
  statusLayer->backgroundColor("lightgreen");
  statusLayer->penColor("darkblue");
  //statusLayer->setTextFont("DL::Roboto");

  DDAutoPinConfigBuilder<1> builder('V');
  builder.addLayer(statusLayer);
  builder.addRemainingGroup('H');
  dumbdisplay.configAutoPin(builder.build());

  dumbdisplay.writeComment("... initialized");
}

struct KnownCommandLayer {
  String key;
  DDLayer* layer;
};

KnownCommandLayer knownCommandLayers[4];  // maximum 4 layers
int knownCommandLayerCount = 0;

DDLayer* findCommandLayer(const String& commandTarget, const String& commandAction) {
  String key = commandTarget + ":" + commandAction;
  for (int i = 0; i < knownCommandLayerCount; i++) {
    if (knownCommandLayers[i].key == key) {
      return knownCommandLayers[i].layer;
    }
  }
  DDLayer* layer = NULL;
  if (commandTarget == "kitchen" || commandTarget == "living room") {
    LcdDDLayer* lcdLayer = dumbdisplay.createLcdLayer(12, 3);
    //lcdLayer->border(1, "black");
    lcdLayer->writeCenteredLine(commandTarget, 1);
    layer = lcdLayer;
  } else if (commandTarget == "bedroom" || commandTarget == "balcony") {
    GraphicalDDLayer* graphicallayer = dumbdisplay.createGraphicalLayer(200, 200);
    //graphicallayer->border(5, "white");
    graphicallayer->setTextSize(12);
    graphicallayer->setTextColor("red");
    graphicallayer->print(commandTarget);
    layer = graphicallayer;
  }
  if (layer != NULL) {
    knownCommandLayers[knownCommandLayerCount].key = key;
    knownCommandLayers[knownCommandLayerCount].layer = layer;
    knownCommandLayerCount += 1;
  }
  return layer;
}

bool handleCommand(const String& commandTarget, const String& commandAction) {
  DDLayer* layer = findCommandLayer(commandTarget, commandAction);
  if (layer != NULL) {
    return true;
  } else {
    return false;
  }
}

long lastShowIdleMillis = 0;

void loop() {
  if (receivedNewCommand) {
    const char* commandTarget = ReceivedPacket.commandTarget;
    const char* commandAction = ReceivedPacket.commandAction;
    if (handleCommand(commandTarget, commandAction)) {
      statusLayer->println(String("- Handled command for [") + commandTarget + "] to [" + commandAction + "]");
    } else {
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


