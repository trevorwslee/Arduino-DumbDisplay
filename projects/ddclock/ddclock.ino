// for a desciption of the experiment
// -- ESP32 Digital Clock experiment with DHT11 and DumbDisplay, and the Internet
// -- https://www.youtube.com/watch?v=BVceJHvigBQ 


//****
//* - may want to setup proper "timezone" for WORLD_TIME_API_END_POINT
//* - get openweathermap.org APPID, and setup OPEN_WEATHER_API_END_POINT
//* - comment out DHTTYPE if DHT11 not connected; note that DHT11 data pin is 5 (DHTPin below)
//* - comment out ESP32_BT_NAME if not using ESP32
//****
#define WORLD_TIME_API_END_POINT "http://worldtimeapi.org/api/timezone/Asia/Hong_Kong"
//#define OPEN_WEATHER_API_END_POINT "http://api.openweathermap.org/data/2.5/weather?q=Hong Kong&appid=<your-APPID>"
//#define DHTTYPE DHT11
//#define ESP32_BT_NAME "ESP32"


const char TemperatureUnit = 'C';  // can be F
const int DHTPin = 5;

#ifdef ESP32_BT_NAME
  const int LEDPIN = 2;
#else
  const int LEDPIN = LED_BUILTIN;
#endif


#ifdef DHTTYPE
#include "DHT.h"
DHT dht(DHTPin, DHTTYPE);
#endif


#ifdef ESP32_BT_NAME
  #define DD_4_ESP32
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(ESP32_BT_NAME));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput(57600));
#endif


SevenSegmentRowDDLayer *pLeft7Seg;
SevenSegmentRowDDLayer *pMid7Seg;
SevenSegmentRowDDLayer *pRight7Seg;
LedGridDDLayer *pSepLed1;
LedGridDDLayer *pSepLed2;
LcdDDLayer *pLcd;
JsonDDTunnelMultiplexer *pJsonData;


void setup() {
  pinMode(LEDPIN, OUTPUT);

#ifdef DHTTYPE
  dht.begin();
#endif 

  pLeft7Seg = dumbdisplay.create7SegmentRowLayer(2);   // 2 digits
  pMid7Seg = dumbdisplay.create7SegmentRowLayer(2);    // 2 digits
  pRight7Seg = dumbdisplay.create7SegmentRowLayer(2);  // 2 digits
  pSepLed1 = dumbdisplay.createLedGridLayer(3, 7);     // 3x7 leds
  pSepLed2 = dumbdisplay.createLedGridLayer(3, 7);     // 3x7 leds
  pLcd = dumbdisplay.createLcdLayer(20, 3);

  pLeft7Seg->segmentColor("navy");
  pLeft7Seg->backgroundColor("ivory");
  pMid7Seg->segmentColor("navy");
  pMid7Seg->backgroundColor("ivory");
  pRight7Seg->segmentColor("navy");
  pRight7Seg->backgroundColor("ivory");

  pSepLed1->onColor("navy");
  pSepLed1->backgroundColor("ivory");
  pSepLed2->onColor("navy");
  pSepLed2->backgroundColor("ivory");

  pLcd->pixelColor("midnightblue");
  pLcd->bgPixelColor("ivory");
  pLcd->backgroundColor("lightgray");
  pLcd->border(2, "aliceblue");

  dumbdisplay.configAutoPin(DD_AP_VERT_2(
    DD_AP_HORI_5(pLeft7Seg->getLayerId(),
                   pSepLed1->getLayerId(),
                 pMid7Seg->getLayerId(),
                   pSepLed2->getLayerId(),
                 pRight7Seg->getLayerId()),
    pLcd->getLayerId()
  ));


  dumbdisplay.connect();  // explicitly make connection, so the the following "write comment" will work
  dumbdisplay.writeComment("*** DDCLOCK READY ***");
  dumbdisplay.writeComment("***  LET'S BEGIN  ***");

  JsonDDTunnel *pTimeTunnel = dumbdisplay.createJsonTunnel(WORLD_TIME_API_END_POINT, false);  
#ifdef OPEN_WEATHER_API_END_POINT  
  JsonDDTunnel *pWeatherTunnel = dumbdisplay.createJsonTunnel(OPEN_WEATHER_API_END_POINT, false);  
#else
  JsonDDTunnel *pWeatherTunnel = NULL;  
#endif

  JsonDDTunnel* tunnels[] = { pTimeTunnel, pWeatherTunnel };
  pJsonData = new JsonDDTunnelMultiplexer(tunnels, 2);
}



long lastUpdateTempMillis;
long lastGetJsonDataMillis;

String date;
int hour;
int minute;
int second;

long nextMillis = -1;

void loop() {
#ifdef DHTTYPE
  // Check and make sure not getting DHT11 data too frequent.
  if (lastUpdateTempMillis == 0 || (millis() - lastUpdateTempMillis) > 5000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    float t = dht.readTemperature(TemperatureUnit != 'C');
    // Check if any reads failed (will try again).
    if (isnan(h) || isnan(t)) {
      dumbdisplay.writeComment("Failed to read from DHT sensor!");
    } else {
      int humi = (int) (h + 0.5);
      int temp = (int) (t + 0.5);
      dumbdisplay.writeComment("Temp: " + String(t) + "°" +  TemperatureUnit + "; Humi: " + String(h) + "%");
      pLcd->writeLine("🌡 : " + String(temp) + "°" + TemperatureUnit + "   💧 : " + String(humi) + "%", 2);
      lastUpdateTempMillis = millis();
    }
  }
#endif

  if (pJsonData->eof()) {
    // Need to get data from Internet again. Check to make sure not too frequent.
    if (lastGetJsonDataMillis == 0 || (millis() - lastGetJsonDataMillis) > 30000) {
      pJsonData->reconnect();
      lastGetJsonDataMillis = millis();
    }
  }

  // Loop and get all the data got from Internet.
  while (pJsonData->count() > 0) {
      String fieldId;
      String fieldValue;
      int tunnelIdx = pJsonData->read(fieldId, fieldValue); 
      // Write out what data got as comments to DD.
      dumbdisplay.writeComment(String(tunnelIdx) + "@" + fieldId + "=" + fieldValue);
      if (tunnelIdx == 0 && fieldId == "datetime") {
        // It is the 'datetime' field. Process it.
        int idx1 = fieldValue.indexOf('T');
        if (idx1 != -1) {
          String dateField = fieldValue.substring(0, idx1);
          int idx2 = fieldValue.lastIndexOf('+');
          if (idx2 != -1) {
            String time = fieldValue.substring(idx1 + 1, idx2);
            idx1 = time.indexOf(':');
            idx2 = time.indexOf(':', idx1 + 1);
            int idx3 = time.indexOf('.', idx2 + 1);
            if (idx3 != -1) {
              String hh = time.substring(0, idx1);
              String mm = time.substring(idx1 + 1, idx2);
              String ss = time.substring(idx2 + 1, idx3);
              String sub = time.substring(idx3);
              hour = hh.toInt();
              minute = mm.toInt();
              second = ss.toInt();
              float subSecond = sub.toFloat();
              long diffMillis = 1000.0 * (1.0 -  subSecond);
              nextMillis = millis() + diffMillis;
            }
            date = dateField;
          }
        }
      }
      if (tunnelIdx == 0 && fieldId == "day_of_week") {
        // Got 'day_of_week' field. Process it and show date.
        int dayField = fieldValue.toInt();
        String day = "";
        switch (dayField) {
          case 1: day = "Mon"; break;
          case 2: day = "Tue"; break;
          case 3: day = "Wed"; break;
          case 4: day = "Thu"; break;
          case 5: day = "Fri"; break;
          case 6: day = "Sat"; break;
          case 0: day = "Sun"; break;
        }
        pLcd->writeLine("📆 : " + date + " " + day, 0);
      }
      if (tunnelIdx == 1 && fieldId == "weather.0.main") {
        // Got 'weather'. Show it.
        pLcd->writeLine("☁ : " + fieldValue, 1);
      }
  }

  if (nextMillis == -1) {
      // Don't know time yet.
      digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  } else {
    // Check to see if it is time to show time, at one second interval.
    long diffMillis = millis() - nextMillis;
    if (diffMillis >= 0) {
      // It is time to show time.
      if (digitalRead(LEDPIN)) {
        digitalWrite(LEDPIN, LOW);
        pSepLed1->turnOff(1, 2);
        pSepLed1->turnOff(1, 4);
        pSepLed2->turnOff(1, 2);
        pSepLed2->turnOff(1, 4);
      } else {
        digitalWrite(LEDPIN, HIGH);
        pSepLed1->turnOn(1, 2);
        pSepLed1->turnOn(1, 4);
        pSepLed2->turnOn(1, 2);
        pSepLed2->turnOn(1, 4);
      }
      second++;
      if (second > 59) {
        second = 0;
        minute++;
      }
      if (minute > 59) {
        minute = 0;
        hour++;
      }
      if (hour > 23) {
        hour = 0;
      }
      pLeft7Seg->showNumber(hour, "0");
      pMid7Seg->showNumber(minute, "0");
      pRight7Seg->showNumber(second, "0");
      nextMillis =  millis() + (1000.0 - diffMillis);
    }
  }
}