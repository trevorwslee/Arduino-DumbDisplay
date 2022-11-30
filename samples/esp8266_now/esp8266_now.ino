
// *
// * if for CLIENT, define ESP_NOW_CLIENT, and may want to define it first, since you will need to find out CLIENT's MAC
// * if for SERVER, define ESP_NOW_SERVER_FOR_MAC with client's MAC, like shown below
// *

//#define ESP_NOW_CLIENT
#define ESP_NOW_SERVER_FOR_MAC 0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15

#include <ESP8266WiFi.h>
#include <espnow.h>

// define a structure as an ESP Now packet
struct ESPNowPacket
{
  long tPlusMillis;
};

// if acting as SERVER, define a variable for storing CLIENT's MAC
#if defined(ESP_NOW_SERVER_FOR_MAC)
uint8_t ClientMACAddress[] = { ESP_NOW_SERVER_FOR_MAC };
#endif

// if acting as SERVER, also define a callback function to be called after sending a packet
#if defined(ESP_NOW_SERVER_FOR_MAC)
void OnSentData(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("... sent ");
  if (sendStatus == 0)
  {
    Serial.print("SUCCESSFUL");
  }
  else
  {
    Serial.print("FAILED");
  }
  Serial.println(" ...");
}
#endif

// if acting as CLIENT, define a callback once a package is received
#if defined(ESP_NOW_CLIENT)
ESPNowPacket LastReceivedPacket;
volatile bool LastReceivedPacketValid = false;
void OnReceivedData(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&LastReceivedPacket, incomingData, len);
  LastReceivedPacketValid = true;
}
#endif

int espNowStatus;

void setup()
{
  // put ESP8266's WIFI to station mode
  WiFi.mode(WIFI_STA);

  Serial.begin(115200);

  // 1. initial ESP Now
  espNowStatus = esp_now_init();

  if (espNowStatus == 0)
  {
    // if acting as SERVER, set ESP role to controller, set "sent data" callback, and add peer (i.e. CLIENT) to send package to
#if defined(ESP_NOW_SERVER_FOR_MAC)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnSentData);
    esp_now_add_peer(ClientMACAddress, ESP_NOW_ROLE_SLAVE, 0, NULL, 0); // 0 is the channel
#endif

    // if acting as CLIENT, set ESP role to slave, and set "received data" callback
#if defined(ESP_NOW_CLIENT)
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnReceivedData);
#endif
  }

  Serial.println();
  Serial.println("OK!");
}

#if defined(ESP_NOW_SERVER_FOR_MAC)
const long MaxIdleMillis = 2000;
#endif
#if defined(ESP_NOW_CLIENT)
const long MaxIdleMillis = 3000;
#endif

long lastActivityMillis = 0;

void loop()
{
  if (espNowStatus != 0)
  {
    Serial.println("ESP Now not properly initialized!");
    Serial.print("status is ");
    Serial.println(espNowStatus);
    delay(2000);
    return;
  }

  long nowMillis = millis();

// if acting as CLIENT, check if received packet ("current time millis of the SERVER"); if so, show it
#if defined(ESP_NOW_CLIENT)
  if (LastReceivedPacketValid)
  {
    LastReceivedPacketValid = false;
    int tPlusSecond = LastReceivedPacket.tPlusMillis / 1000;
    Serial.print("received ... T+");
    Serial.print(tPlusSecond);
    Serial.println("s");
    lastActivityMillis = nowMillis;
    return;
  }
#endif

  if ((nowMillis - lastActivityMillis) >= MaxIdleMillis)
  {
    // if acting as SERVER, setup package to send to CLIENT ... the packet is just the current millis of the SERVER
#if defined(ESP_NOW_SERVER_FOR_MAC)
    ESPNowPacket packet;
    packet.tPlusMillis = nowMillis;
    esp_now_send(ClientMACAddress, (uint8_t *)&packet, sizeof(packet));
#endif

    // if acting as CLIENT, print out the CLIENT's MAC
#if defined(ESP_NOW_CLIENT)
    Serial.print("my MAC is ");
    Serial.println(WiFi.macAddress());
#endif
    lastActivityMillis = nowMillis;
  }
}
