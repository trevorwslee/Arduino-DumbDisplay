#define ENABLE_DUMBDISPLAY
//#define TEST_VOICE



#ifdef ENABLE_DUMBDISPLAY
#include "dumbdisplay.h"
DumbDisplay dumbdisplay(new DDInputOutput(115200));
// #ifdef ESP32
// #include "esp32dumbdisplay.h"
// DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32", true, 115200));
// #else
// //#include "ssdumbdisplay.h"
// //DumbDisplay dumbdisplay(new DDSoftwareSerialIO(new SoftwareSerial(2, 3), 115200, true, 115200));
// #include "dumbdisplay.h"
// DumbDisplay dumbdisplay(new DDInputOutput(115200));
// #endif
#endif


// #ifdef ESP32
// #define DISABLE_BROWNOUT
// #endif
// #ifdef DISABLE_BROWNOUT
// // for disable brownout detection
// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"
// #endif




#ifdef ESP32
#define synthesizer Serial2
#else
#include <SoftwareSerial.h>
SoftwareSerial synthesizer(9, 8);  // pin 9 connect to TX of XFS5152CE board; 8 connect to RX of XFS5152CE board
#endif




int UTF8ToUnicode(const char* utf8, uint8_t* utf16Buffer) {
    int idx = 0;
    int i = 0;
    while (true) {
         uint32_t value = utf8[i];
         if (value == 0) {
             break;
         }
         if ((value & 0b10000000) == 0) {
             value = utf8[i++];
         } else if ((value & 0b11100000) == 0b11000000) {
             value   = 0b00011111 & utf8[i++];
             value <<= 6;
             value  += 0b00111111 & utf8[i++];
         } else if ((value & 0b11110000) == 0b11100000) {
             value   = 0b00001111 & utf8[i++];
             value <<= 6;
             value  += 0b00111111 & utf8[i++];
             value <<= 6;
             value += 0b00111111 & utf8[i++];
         } else {
             value   = 0b00000111 & utf8[i++];
             value <<= 6;
             value  += 0b00111111 & utf8[i++];
             value <<= 6;
             value  += 0b00111111 & utf8[i++];
             value <<= 6;
             value  += 0b00111111 & utf8[i++];
         }
//Serial.println(value, HEX);
         utf16Buffer[idx++] = 0x00FF & value;
         utf16Buffer[idx++] = (0xFF00 & value) >> 8;
//Serial.println(utf16Buffer[idx - 2], HEX);
//Serial.println(utf16Buffer[idx - 1], HEX);
    }
    return idx;
}

void synthesizeSpeech(const String& text) {
    synthesizer.write((byte)0xFD);
    int text_len = text.length();
    uint8_t buffer[2 * text_len];
    int len = UTF8ToUnicode(text.c_str(), buffer);
    int out_len = 2 + len;
    synthesizer.write((byte)((out_len&0xFF00)>8));
    synthesizer.write((byte)(out_len&0x00FF));
    synthesizer.write((byte)0x01);
    synthesizer.write((byte)0x03);
#ifndef ENABLE_DUMBDISPLAY
    Serial.print("{");
    Serial.print(text);
    Serial.print("}=");
    Serial.println(len);
#endif           
    synthesizer.write(buffer, len);
}


#ifdef ENABLE_DUMBDISPLAY
LcdDDLayer* lcd;
#endif

void setup() {
// #ifdef ESP32
// #ifdef DISABLE_BROWNOUT
//   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);     // Turn-off the 'brownout detector'
// #endif
// #endif

#ifndef ENABLE_DUMBDISPLAY
    Serial.begin(115200);
#endif
    synthesizer.begin(115200);

#ifndef ENABLE_DUMBDISPLAY
    Serial.println("Hello! Let's get started!");
    if (true) {
        String text = "科大讯飞";
        //String text = "ABCD";
        uint8_t buffer[64];
        int len = UTF8ToUnicode(text.c_str(), buffer);
        for (int i = 0; i < len; i++) {
            uint8_t b = buffer[i];
            Serial.print(b, HEX);
            Serial.print(".");
        }
        Serial.println();
    }
#endif

#ifdef ENABLE_DUMBDISPLAY
    lcd = dumbdisplay.createLcdLayer(32, 3);
    lcd->enableFeedback("f:keys");
#endif
}




long checkMillis = 0;
bool isIdle = false;
void loop() {
#ifdef ENABLE_DUMBDISPLAY
    const DDFeedback* feedback = lcd->getFeedback();
    if (feedback != NULL) {
        if (feedback->text.length() > 0) {
            lcd->writeLine(feedback->text, 1);
            if (isIdle) {
                isIdle = false;
                synthesizeSpeech("[v1][h0]" + feedback->text);
            } else {
                dumbdisplay.writeComment("BUSY!");
            }
        }
    }
#endif

#ifdef TEST_VOICE
    if (isIdle) {
        isIdle = false;
        synthesizeSpeech("[v1][h0]港大生功課「回收USB線」 吸引兩上市公司合作 設40回收點");
    }
#endif

    if ((millis() - checkMillis) > 2000) {
        checkMillis = millis();
#ifdef ENABLE_DUMBDISPLAY
        dumbdisplay.writeComment("checking status ...");
#else        
        Serial.println("checking status ...");
#endif        
        synthesizer.write((byte)0xFD);
        synthesizer.write((byte)0x00);
        synthesizer.write((byte)0x01);
        synthesizer.write((byte)0x21);
    } else {
        if (synthesizer.available()) {
            int status = synthesizer.read();
            isIdle = status == 0x4F;
#ifdef ENABLE_DUMBDISPLAY
            if (isIdle) {
                dumbdisplay.writeComment("... status=IDLE");
            } else {
                dumbdisplay.writeComment(String("... status=" + String(status)));
            }       
#else
            Serial.print("... status=");
            Serial.print(status, HEX);
            if (isIdle) {
                Serial.print(" (IDLE)");
            }
            Serial.println();
#endif            
        }
    }
}
