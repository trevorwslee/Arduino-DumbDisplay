//*******
//** ADAPTED FROM ESP32 SAMPLE BLE_server ***
//*******


//#define INDICATE
#define NOTIFY


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


BLECharacteristic *pCharacteristic;

void setup() {
  BLEDevice::init("ESP32Ble");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ
#if defined(INDICATE)                                        
      | BLECharacteristic::PROPERTY_INDICATE
#elif defined (NOTIFY)                                        
      | BLECharacteristic::PROPERTY_NOTIFY
#endif                                        
  );
#if defined(INDICATE) || defined (NOTIFY)                                        
  pCharacteristic->addDescriptor(new BLE2902());
#endif

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

long startMillis = millis();
void loop() {
  long up = (millis() - startMillis) / 1000;
#if defined(INDICATE)                                        
  pCharacteristic->setValue(("I-Up: " + String(up) + "s").c_str());
  pCharacteristic->indicate();
#elif defined (NOTIFY)                                        
  pCharacteristic->setValue(("N-Up: " + String(up) + "s").c_str());
  pCharacteristic->notify();
#else  
  pCharacteristic->setValue(("Up: " + String(up) + "s").c_str());
#endif
  delay(1000);
}