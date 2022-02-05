
#ifdef ESP32

#define synthesizer Serial2

#else

#include <SoftwareSerial.h>
SoftwareSerial synthesizer(9, 8);  // pin 9 connect to TX of XFS5152CE board; 8 connect to RX of XFS5152CE board

#endif


#define TEST_VOICE


int utf8ToUtf16(const char* utf8, uint8_t* utf16Buffer) {
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


void setup() {
    Serial.begin(115200);
    synthesizer.begin(115200);

    Serial.println("Hello! Let's get started!");

    if (true) {
        String text = "A科B大C讯D飞";
        //String text = "ABCD";
        uint8_t buffer[64];
        int len = utf8ToUtf16(text.c_str(), buffer);
        for (int i = 0; i < len; i++) {
            uint8_t b = buffer[i];
            Serial.print(b, HEX);
            Serial.print(".");
        }
        Serial.println();
    }
}




long checkMillis = 0;
bool isIdle = false;
void loop() {

#ifdef TEST_VOICE
    if (isIdle) {
        isIdle = false;
        synthesizer.write((byte)0xFD);
        if (true) {
            //String text = "[v1][h0]科大讯飞! 科大訊飛! Apple, orange and banana!";
            String text = "[v1][h0]巴黎世家人氣老爹鞋低至6折!粉紅Triple S波鞋只需$5,880!";
            int text_len = text.length();
            uint8_t buffer[2 * text_len];
            int len = utf8ToUtf16(text.c_str(), buffer);
            int out_len = 2 + len;
            synthesizer.write((byte)((out_len&0xFF00)>8));
            synthesizer.write((byte)(out_len&0x00FF));
            synthesizer.write((byte)0x01);
            synthesizer.write((byte)0x03);
            Serial.print("{");
            Serial.print(text);
            Serial.print("}=");
            Serial.println(len);
            synthesizer.write(buffer, len);
            //synthesizer.print(text);
            // for (int i = 0; i < len; i++) {
            //     uint8_t b = buffer[i];
            //     synthesizer.write(b);
            // }
        } else if (true) {
            String text = "[v1][h0][m54]apple and orange";//"快";
            //String text = "快";
            if (true) {
                int len = 2 + text.length();
                synthesizer.write((byte)((len&0xFF00)>8));
                synthesizer.write((byte)(len&0x00FF));
            } else {    
                synthesizer.write((byte)0x00);
                synthesizer.write((byte)(2 + text.length())/*0x07*/);
            }
            synthesizer.write((byte)0x01);
            synthesizer.write((byte)0x00);
            if (false) {
                for (int i = 0; i < text.length(); i++) {
                    char c = text.charAt(i);
                    Serial.print("[");
                    Serial.print(c);
                    Serial.print("]");
                    synthesizer.write((byte) c);
                }
                Serial.println(text.length());
            } else {
                Serial.print("{");
                Serial.print(text);
                Serial.print("}=");
                Serial.println(text.length());
                synthesizer.print(text);
            }
        } else {
            if (false) {
                // UNICODE
                synthesizer.write((byte)0x00);
                synthesizer.write((byte)0x06);
                synthesizer.write((byte)0x01);
                synthesizer.write((byte)0x03);
                // 5feb 4e50
                synthesizer.write((byte)0xeb);
                synthesizer.write((byte)0x5f);
                synthesizer.write((byte)0x50);
                synthesizer.write((byte)0x4e);
            } else {
                synthesizer.write((byte)0x00);
                synthesizer.write((byte)0x0A);
                synthesizer.write((byte)0x01);
                synthesizer.write((byte)0x00);
                synthesizer.write((byte)0xBF);
                synthesizer.write((byte)0xC6);
                synthesizer.write((byte)0xB4);
                synthesizer.write((byte)0xF3);
                synthesizer.write((byte)0xD1);
                synthesizer.write((byte)0xB6);
                synthesizer.write((byte)0xB7);
                synthesizer.write((byte)0xC9);
            }
        }
        delay(5000);
    }
#endif

    if ((millis() - checkMillis) > 2000) {
        checkMillis = millis();
        Serial.println("checking status ...");
        synthesizer.write((byte)0xFD);
        synthesizer.write((byte)0x00);
        synthesizer.write((byte)0x01);
        synthesizer.write((byte)0x21);
    } else {
        if (synthesizer.available()) {
            int status = synthesizer.read();
            isIdle = status == 0x4F;
            Serial.print("... status=");
            Serial.print(status, HEX);
            if (isIdle) {
                Serial.print(" (IDLE)");
            }
            Serial.println();
        }
    }
}
