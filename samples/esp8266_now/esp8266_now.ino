
//#define ESP_NOW_SERVER_FOR_MAC { 0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15 }
#define ESP_NOW_CLIENT


#include <ESP8266WiFi.h>
#include <espnow.h>

struct ESPNowPacket
{
  long tPlusMillis;
};

#if defined(ESP_NOW_SERVER_FOR_MAC)
uint8_t ClientMACAddress[] = ESP_NOW_SERVER_FOR_MAC;
#endif

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



#if defined(ESP_NOW_CLIENT)
ESPNowPacket LastReceivedPacket;
long LastReceivedPacketMillis = 0;
bool LastReceivedPacketValid = false;
void OnReceivedData(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  // //memcpy(&myData, incomingData, sizeof(myData));
  // struct_message myData;
  // memcpy(&myData, incomingData, len);
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  // Serial.print("Char: ");
  // Serial.println(myData.a);
  // Serial.print("Int: ");
  // Serial.println(myData.b);
  // Serial.print("Float: ");
  // Serial.println(myData.c);
  // Serial.print("String: ");
  // Serial.println(myData.d);
  // Serial.print("Bool: ");
  // Serial.println(myData.e);
  // Serial.println();
}
#endif

int espNowStatus;

void setup()
{

  WiFi.mode(WIFI_STA);

  Serial.begin(115200);


  espNowStatus = esp_now_init();

  if (espNowStatus == 0)
  {
#if defined(ESP_NOW_SERVER_FOR_MAC)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnSentData);
    esp_now_add_peer(ClientMACAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0); // 1 is the channel
#endif
#if defined(ESP_NOW_CLIENT)
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnReceivedData);
#endif
  }

  Serial.println();
  Serial.println("OK!");
}

unsigned long lastActivityMillis = 0;

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

#if defined(ESP_NOW_CLIENT)
    if (LastReceivedPacketValid) {
      LastReceivedPacketValid = false;
      int tPlusSecond = LastReceivedPacket.tPlusMillis / 1000;
      Serial.print("received ... T+");
      Serial.print(tPlusSecond);
      Serial.println("s");
      lastActivityMillis = nowMillis;
      return;
    }
#endif

  if ((nowMillis - lastActivityMillis) >= 2000)
  {
#if defined(ESP_NOW_SERVER_FOR_MAC)
    ESPNowPacket packet;
    packet.tPlusMillis = nowMillis;
    esp_now_send(ClientMACAddress, (uint8_t *) &packet, sizeof(packet));
#endif
#if defined(ESP_NOW_CLIENT)
      Serial.print("my MAC is ");
      Serial.println(WiFi.macAddress());
#endif
    lastActivityMillis = nowMillis;
  }
}
