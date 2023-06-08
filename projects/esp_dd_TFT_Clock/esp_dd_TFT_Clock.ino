
// if want to use Bluetooth for connection, uncomment the following like whic also define the Bluetooth device name
// note that if not using Bluetooth, WIFI is assumed
//#define BLUETOOTH "ESP32"

#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true, DD_SERIAL_BAUD));
#else
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#endif


// *****
// * say, for TTGO T-Display
// * - modify User_Setup_Select.h
// * . comment out #include <User_Setup.h>
// * . uncomment #include <User_Setups/Setup25_TTGO_T_Display.h>
// *****

// for T-DISPLAY -- LEFT: 0 ... RIGHT: 35  ... 0 seems doesn't work

// if not for TTGO T-Display, comment out the following lines
// #define WAKE_BUTTON_PIN       35
// #define WAKE_BUTTON_PIN_NUM   GPIO_NUM_35
#define WAKE_BUTTON_PIN       0
#define WAKE_BUTTON_PIN_NUM   GPIO_NUM_0



LcdDDLayer* syncButton = NULL;
BasicDDTunnel* datetimeTunnel;
JoystickDDLayer* blueSlider;
SevenSegmentRowDDLayer* left7Seg;
SevenSegmentRowDDLayer* mid7Seg;
SevenSegmentRowDDLayer* right7Seg;
LedGridDDLayer* sepLed1;
LedGridDDLayer* sepLed2;
LcdDDLayer* sleepIdleLabel;
LcdDDLayer* sleepNoneButton;
LcdDDLayer* sleep2MinsButton;


int shownHH, shownMM, shownSS = -1;
//RTC_DATA_ATTR int sleepHH, sleepMM, sleepSS = -1;


const int DefaultMaxIdleMillis = 2 * 60 * 1000;
RTC_DATA_ATTR int maxIdleMillis = -1;//DefaultMaxIdleMillis;
long idleStartMillis;




/*
 An example analogue clock using a TFT LCD screen to show the time
 use of some of the drawing commands with the ST7735 library.

 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo. 

 Uses compile time to set the time so a reset will start with the compile time again
 
 Gilchrist 6/2/2014 1.0
 Updated by Bodmer
 */

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define TFT_GREY 0xBDF7


float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
uint16_t osx=64, osy=64, omx=64, omy=64, ohx=64, ohy=64;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint32_t targetTime = 0;                    // for next 1 second timeout

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

RTC_DATA_ATTR uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6);  // Get H, M, S from compile time
RTC_DATA_ATTR uint32_t ClockBGC = 0;

bool initial = 1;


void setup(void) {  
  Serial.begin(115200);
  idleStartMillis = millis();

#ifdef WAKE_BUTTON_PIN  
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
#endif  

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_GREY);
  tft.setTextColor(TFT_GREEN, TFT_GREY);  // Adding a black background colour erases previous text automatically
  
  // Draw clock face
  tft.fillCircle(64, 64, 61, TFT_BLUE);
  tft.fillCircle(64, 64, 57, ClockBGC/*TFT_BLACK*/);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*57+64;
    yy0 = sy*57+64;
    x1 = sx*50+64;
    yy1 = sy*50+64;

    tft.drawLine(x0, yy0, x1, yy1, TFT_BLUE);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*53+64;
    yy0 = sy*53+64;
    
    tft.drawPixel(x0, yy0, TFT_BLUE);
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==0 || i==180) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
  }

  tft.fillCircle(65, 65, 3, TFT_RED);

  // Draw text at position 64,125 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
  tft.drawCentreString("Time flies",64,130,4);

  targetTime = millis() + 1000; 

  Serial.println("*** READY!!!");
}


void loop() {

#ifdef WAKE_BUTTON_PIN  
  int v = digitalRead(WAKE_BUTTON_PIN);
  if (v == LOW) {
    Serial.println("$");
  }
#endif  


  DDConnectPassiveStatus connectStatus;
  dumbdisplay.connectPassive(&connectStatus);
  if (connectStatus.connected) {
    idleStartMillis = millis();

    if (syncButton != NULL && connectStatus.reconnecting) {
      dumbdisplay.masterReset();
      syncButton = NULL;
      datetimeTunnel = NULL;
      tft.setTextColor(TFT_GREY, TFT_GREY);
      tft.drawCentreString("Connected",66,160,4);
      return;
    }

    bool updateUI = false;
    if (syncButton == NULL) {
      dumbdisplay.recordLayerSetupCommands();

      syncButton = dumbdisplay.createLcdLayer(14, 1);
      syncButton->border(2, DD_COLOR_darkgreen, "raised");
      syncButton->writeLine(" 🔄  Sync Time");
      syncButton->enableFeedback("fl");

      blueSlider = dumbdisplay.createJoystickLayer(127, "hori");
      blueSlider->moveToPos(ClockBGC & 127, 0);
      blueSlider->border(5, DD_COLOR_darkgray, "round", 3);

      left7Seg = dumbdisplay.create7SegmentRowLayer(2);   // 2 digits
      mid7Seg = dumbdisplay.create7SegmentRowLayer(2);    // 2 digits
      right7Seg = dumbdisplay.create7SegmentRowLayer(2);  // 2 digits
      sepLed1 = dumbdisplay.createLedGridLayer(3, 7);     // 3x7 leds
      sepLed2 = dumbdisplay.createLedGridLayer(3, 7);     // 3x7 leds
      left7Seg->segmentColor(DD_COLOR_navy);
      left7Seg->backgroundColor(DD_COLOR_ivory);
      mid7Seg->segmentColor(DD_COLOR_navy);
      mid7Seg->backgroundColor(DD_COLOR_ivory);
      right7Seg->segmentColor(DD_COLOR_navy);
      right7Seg->backgroundColor(DD_COLOR_ivory);
      sepLed1->onColor(DD_COLOR_navy);
      sepLed1->backgroundColor(DD_COLOR_ivory);
      sepLed2->onColor(DD_COLOR_navy);
      sepLed2->backgroundColor(DD_COLOR_ivory);

      sleepIdleLabel = dumbdisplay.createLcdLayer(7, 2);
      sleepIdleLabel->pixelColor(DD_COLOR_darkblue);
      sleepIdleLabel->border(1, DD_COLOR_gray, "flat");
      sleepIdleLabel->writeLine("😴 Idle", 0);
      sleepIdleLabel->writeLine(" Sleep", 1);
      
      sleepNoneButton = dumbdisplay.createLcdLayer(6, 1);
      sleepNoneButton->writeLine(" None ");
      sleepNoneButton->enableFeedback("f");
      
      sleep2MinsButton = dumbdisplay.createLcdLayer(6, 1);
      sleep2MinsButton->writeLine("2 Mins");
      sleep2MinsButton->enableFeedback("f");

      dumbdisplay.configAutoPin(DDAutoPinConfig('V')
        .addLayer(syncButton)
        .beginGroup('H')
          .addLayer(left7Seg)
          .addLayer(sepLed1)
          .addLayer(mid7Seg)
          .addLayer(sepLed2)
          .addLayer(right7Seg)
        .endGroup()
        .addLayer(blueSlider)
        .beginGroup('H')
          .addLayer(sleepIdleLabel)
          .beginGroup('V')
            .addLayer(sleepNoneButton)
            .addLayer(sleep2MinsButton)
          .endGroup()
        .endGroup()
        .build()
      );
      dumbdisplay.playbackLayerSetupCommands( __FILE__);
      dumbdisplay.backgroundColor(DD_INT_COLOR(ClockBGC));
      tft.setTextColor(TFT_RED, TFT_GREY);
      tft.drawCentreString("Connected",66,160,4);

      updateUI = true;
      initial = 1;
    }

    if (initial || hh != shownHH) {
      left7Seg->showNumber(hh, "0");
      shownHH = hh;
    }
    if (initial || mm != shownMM) {
      mid7Seg->showNumber(mm, "0");
      shownMM = mm;
    }
    if (initial || ss != shownSS) {
      right7Seg->showNumber(ss, "0");
      shownSS = ss;
      if ((ss % 2) == 1) {
        sepLed1->turnOn(1, 2);
        sepLed1->turnOn(1, 4);
        sepLed2->turnOn(1, 2);
        sepLed2->turnOn(1, 4);
      } else {
        sepLed1->turnOff(1, 2);
        sepLed1->turnOff(1, 4);
        sepLed2->turnOff(1, 2);
        sepLed2->turnOff(1, 4);
      }
    }

    if (syncButton->getFeedback()) {
      // "sync" button clicked ==> create a "tunnel" to get current date time
      if (datetimeTunnel != NULL) {
        dumbdisplay.deleteTunnel(datetimeTunnel);
      }
      dumbdisplay.logToSerial("getting time for sync ...");
      datetimeTunnel = dumbdisplay.createDateTimeServiceTunnel();
      datetimeTunnel->reconnectTo("now:hhmmss");
    }

    const DDFeedback* fb = blueSlider->getFeedback();
    if (fb != NULL) {
      ClockBGC = fb->x; 
      dumbdisplay.backgroundColor(DD_INT_COLOR(ClockBGC));
      tft.fillCircle(64, 64, 48, ClockBGC);
      initial = 1;
    }
    if (datetimeTunnel != NULL) {
      String nowStr;
      if (datetimeTunnel->readLine(nowStr)) {
        dumbdisplay.logToSerial("... got sync time " + nowStr);
        int now = nowStr.toInt();
        hh = now / 10000;
        mm = (now / 100) % 100;
        ss = now % 100;
        dumbdisplay.deleteTunnel(datetimeTunnel);
        datetimeTunnel = NULL;
        dumbdisplay.tone(2000, 100);
        initial = 1;
      }
    }

    if (sleepNoneButton->getFeedback()) {
      maxIdleMillis = -1;
      updateUI = true;
    }
    if (sleep2MinsButton->getFeedback()) {
      maxIdleMillis = DefaultMaxIdleMillis;
      updateUI = true;
    }

    if (updateUI) {
      sleepNoneButton->border(1, DD_COLOR_gray, maxIdleMillis == -1 ? "flat" : "hair");
      sleepNoneButton->pixelColor(maxIdleMillis == -1 ? DD_COLOR_darkblue : DD_COLOR_gray);
      sleep2MinsButton->border(1, DD_COLOR_gray, maxIdleMillis != -1 ? "flat" : "hair");
      sleep2MinsButton->pixelColor(maxIdleMillis != -1 ? DD_COLOR_darkblue : DD_COLOR_gray);
    }
  } else {
    if (maxIdleMillis != -1) {
      long diffMillis = millis() - idleStartMillis;
      if (diffMillis >= maxIdleMillis) {
        Serial.print("*** idle going to sleep ... ");
        Serial.print("HH=");
        Serial.print(hh);
        Serial.print("; MM=");
        Serial.print(mm);
        Serial.print("; SS=");
        Serial.print(ss);
        Serial.print(" ... ");
        //dumbdisplay->savePassiveConnectState(savedConnectState);
#ifdef WAKE_BUTTON_PIN     
        // Serial.println("wake in delay ...");
        // esp_sleep_enable_timer_wakeup(10 * 1000 * 1000);  // wake in 10 later
        Serial.println("wake by button (TTGO right button) ...");
        esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN_NUM, 0);
#endif
        esp_deep_sleep_start();    
      }
    }
  }

  if (targetTime < millis()) {
    targetTime = millis()+1000;
    ss++;              // Advance second
    if (ss==60) {
      ss=0;
      mm++;            // Advance minute
      if(mm>59) {
        mm=0;
        hh++;          // Advance hour
        if (hh>23) {
          hh=0;
        }
      }
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss*6;                  // 0-59 -> 0-354
    mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
    hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg-90)*0.0174532925);    
    hy = sin((hdeg-90)*0.0174532925);
    mx = cos((mdeg-90)*0.0174532925);    
    my = sin((mdeg-90)*0.0174532925);
    sx = cos((sdeg-90)*0.0174532925);    
    sy = sin((sdeg-90)*0.0174532925);

    if (ss==0 || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 65, 65, ClockBGC/*TFT_BLACK*/);
      ohx = hx*33+65;    
      ohy = hy*33+65;
      tft.drawLine(omx, omy, 65, 65, ClockBGC/*TFT_BLACK*/);
      omx = mx*44+65;    
      omy = my*44+65;
    }

      // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
      tft.drawLine(osx, osy, 65, 65, ClockBGC/*TFT_BLACK*/);
      tft.drawLine(ohx, ohy, 65, 65, TFT_WHITE);
      tft.drawLine(omx, omy, 65, 65, TFT_WHITE);
      osx = sx*47+65;    
      osy = sy*47+65;
      tft.drawLine(osx, osy, 65, 65, TFT_RED);

    tft.fillCircle(65, 65, 3, TFT_RED);
  }
}


