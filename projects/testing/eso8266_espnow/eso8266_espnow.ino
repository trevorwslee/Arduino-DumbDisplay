
//#define CONTROLLER

#if defined(ESP8266)
  #define ESP_OK 0
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#else
  #include <WiFi.h>
  #include <esp_now.h>
#endif



typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

#if defined(CONTROLLER)
uint8_t broadcastAddress[] = {0x48, 0x3F, 0xDA, 0x51, 0x22, 0x15};
#endif

#if !defined(ESP8266)
esp_now_peer_info_t peerInfo;
#endif



#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
#else
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#endif



#if defined(ESP8266)
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len)
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
#endif
{
  //memcpy(&myData, incomingData, sizeof(myData));
  struct_message myData;
  memcpy(&myData, incomingData, len);
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("String: ");
  Serial.println(myData.d);
  Serial.print("Bool: ");
  Serial.println(myData.e);
  Serial.println();
}
 





int espnow_status;
int add_peer_res;
int last_send_res;





void setup(){

  WiFi.mode(WIFI_STA);


  Serial.begin(115200);
  espnow_status = esp_now_init();

  if (espnow_status == ESP_OK) {

#if defined(ESP8266)    
  #if defined(CONTROLLER)
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    add_peer_res = esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);  // 1 is the channel
  #else
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
    add_peer_res = -999;
  #endif
#else
  #if defined(CONTROLLER)
    esp_now_register_send_cb(OnDataSent);
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    add_peer_res = esp_now_add_peer(&peerInfo);
  #else
    esp_now_register_recv_cb(OnDataRecv);
  #endif
#endif  

  } else {
    add_peer_res = -999;
  }

}


unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer




void loop(){

  if ((millis() - lastTime) > timerDelay) {

    Serial.print("status: ");
    Serial.print(espnow_status);
    Serial.print(" ... add peer res: ");
    Serial.print(add_peer_res);
    Serial.print(" ... last send res: ");
    Serial.print(last_send_res);
    Serial.print(" ... ESP Board MAC Address: ");
    Serial.println(WiFi.macAddress());

    if (espnow_status == ESP_OK) {
#if defined(CONTROLLER)
      struct_message myData;
      strcpy(myData.a, "THIS IS A CHAR");
      myData.b = random(1,20);
      myData.c = 1.2;
      myData.d = "Hello";
      myData.e = false;
      last_send_res = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
#else
      last_send_res = -999;
#endif
    }

    lastTime = millis();
  }

}