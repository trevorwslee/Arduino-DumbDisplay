
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#define SERVICE_UUID              "d4b76bfb-2cc6-426c-adcb-905e8bc1a907"
#define ANGLE_CHARACTERISTIC_UUID "284b7fd5-cea3-4426-9499-918350b2ea68"
#define SIN_CHARACTERISTIC_UUID   "a18d7b8d-005d-4ada-b50d-796d50f71f92"
#define COS_CHARACTERISTIC_UUID   "42a06347-c605-473e-8af4-956923d39ba5"
#define TAN_CHARACTERISTIC_UUID   "3347662f-c816-464e-aa76-cdbf34fd09e3"


BLECharacteristic *pAngleCharacteristic;
BLECharacteristic *pSinCharacteristic;
BLECharacteristic *pCosCharacteristic;
BLECharacteristic *pTanCharacteristic;

void setup() {
  BLEDevice::init("ESP32Ble");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pAngleCharacteristic = pService->createCharacteristic(
      ANGLE_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY); 
  pSinCharacteristic = pService->createCharacteristic(
      SIN_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY); 
  pCosCharacteristic = pService->createCharacteristic(
      COS_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY); 
  pTanCharacteristic = pService->createCharacteristic(
      TAN_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY); 

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
}

int angle = 0;
void loop() {
  float r = angle * 0.01745329252; 
  float s = sin(r);
  float c = cos(r);
  float t = tan(r);

  pAngleCharacteristic->setValue(("Ang: " + String(angle)).c_str());
  pAngleCharacteristic->notify();
  pSinCharacteristic->setValue(("Sin: " + String(s, 2)).c_str());
  pSinCharacteristic->notify();
  pCosCharacteristic->setValue(("Cos: " + String(c, 2)).c_str());
  pCosCharacteristic->notify();
  pTanCharacteristic->setValue(("Tan: " + String(t, 2)).c_str());
  pTanCharacteristic->notify();

  angle = angle + 5;
  if (angle >= 360) {
    angle = 0;
  }
  delay(1000);
}