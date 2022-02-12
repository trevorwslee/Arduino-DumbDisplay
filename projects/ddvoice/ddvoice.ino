

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"



#define NEWS_API_ENDPOINT "https://newsapi.org/v2/top-headlines?apiKey=cf8b2b54b5a7499dafd18938294204d9"
#define IMAGE_FILE_NAME   "tempimage.png"


#define PIN_SPEAKER_ENABLE 23
#define synthesizer Serial2




#include "esp32dumbdisplay.h"
DumbDisplay dumbdisplay(new DDBluetoothSerialIO("ESP32"));




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
         utf16Buffer[idx++] = 0x00FF & value;
         utf16Buffer[idx++] = (0xFF00 & value) >> 8;
    }
    return idx;
}


void synthesizeVoice(const String& text) {
    synthesizer.write((byte) 0xFD);
    int text_len = text.length();
    uint8_t buffer[2 * text_len];
    int len = UTF8ToUnicode(text.c_str(), buffer);
    int out_len = 2 + len;
    synthesizer.write((byte) ((out_len & 0xFF00) > 8));
    synthesizer.write((byte) (out_len & 0x00FF));
    synthesizer.write((byte) 0x01);
    synthesizer.write((byte) 0x03);
    synthesizer.write(buffer, len);
}



LcdDDLayer* langsButton;
LcdDDLayer* newsButton;
GraphicalDDLayer* textLayer;
GraphicalDDLayer* imageLayer;

#ifdef NEWS_API_ENDPOINT
JsonDDTunnel* newsTunnel;
SimpleToolDDTunnel* imageTunnel;
#endif


void resetDisplaying() {
    langsButton->pixelColor("gray");
    newsButton->pixelColor("gray");
    textLayer->clear();
    textLayer->setCursor(0, 0);
    imageLayer->clear();
}
void enableButtons() {
    langsButton->pixelColor("white");
    newsButton->pixelColor("white");
}




void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);     // turn off the 'brownout detector'


    synthesizer.begin(115200);

    pinMode(PIN_SPEAKER_ENABLE, OUTPUT);
    digitalWrite(PIN_SPEAKER_ENABLE, LOW);

    delay(1000);

    langsButton = dumbdisplay.createLcdLayer(12, 1);
    langsButton->backgroundColor("indigo");
    langsButton->border(1, "gray", "round");
    langsButton->enableFeedback("f");

    newsButton = dumbdisplay.createLcdLayer(12, 1);
    newsButton->backgroundColor("navy");
    newsButton->border(1, "gray", "round");
    newsButton->enableFeedback("f");
    newsButton->writeCenteredLine("News");

    textLayer = dumbdisplay.createGraphicalLayer(200, 100);
    textLayer->border(2, "darkblue");
    textLayer->padding(5);
    textLayer->setTextColor("blue");
    textLayer->backgroundColor("ivory");
    textLayer->setTextWrap(true);
    textLayer->enableFeedback("f:keys");

#ifdef NEWS_API_ENDPOINT
    newsTunnel = dumbdisplay.createFilteredJsonTunnel("", "title,description,urlToImage");  
    imageTunnel = dumbdisplay.createImageDownloadTunnel("", IMAGE_FILE_NAME);
#endif

    imageLayer = dumbdisplay.createGraphicalLayer(300, 150);
    imageLayer->border(10, "azure", "round");  
    imageLayer->noBackgroundColor();
    imageLayer->penColor("navy");

    dumbdisplay.configAutoPin(DD_AP_VERT_3(
        DD_AP_HORI_2(langsButton->getLayerId(), newsButton->getLayerId()),
        textLayer->getLayerId(),
        imageLayer->getLayerId()
    ));

    enableButtons();
    digitalWrite(PIN_SPEAKER_ENABLE, HIGH);
}



long checkMillis = 0;
bool isIdle = false;

DDValueRecord<bool> englishOnly(true, false);


void loop() {
    if (englishOnly.record()) {
        if (englishOnly) {
            langsButton->writeCenteredLine("English");
        } else {
            langsButton->writeCenteredLine("Eng & 中文");
        }
    }

    const DDFeedback* feedback = textLayer->getFeedback();
    if (feedback != NULL) {
        resetDisplaying();
        if (feedback->text.length() > 0) {
            textLayer->print(feedback->text);
            if (isIdle) {
                isIdle = false;
                String text = "[v1][h0]" + feedback->text;
                if (englishOnly) {
                    text = "[g2]" + text;
                } else {
                    text = "[g1]" + text;
                }
                dumbdisplay.writeComment(text);
                synthesizeVoice(text);
            } else {
                dumbdisplay.writeComment("BUSY!");
            }
        }
        enableButtons();
    }

#ifdef NEWS_API_ENDPOINT
    if (newsButton->getFeedback()) {
        resetDisplaying();
        textLayer->print("... ");
        String category;
        switch (rand() % 5) {
            case 0:
                category = "health";
                break;
            case 1:
                category = "science";
                break;
            case 2:
                category = "sports";
                break;
            case 3:
                category = "technology";
                break;
            default:
                category = "business";
        }
        String country = "us";
        if (!englishOnly) {
            if (rand() % 2 == 0) {
                country="hk";
            }
        }
        String endpoint = NEWS_API_ENDPOINT + ("&pageSize=1&category=" + category) + ("&country=" + country);
        newsTunnel->reconnectTo(endpoint);
        String title = "";
        String description = "";
        String imageUrl = "";
        while (!newsTunnel->eof()) {
            if (newsTunnel->count() > 0) {
                textLayer->print(".");
                String fieldId;
                String fieldValue;
                newsTunnel->read(fieldId, fieldValue);
                if (fieldId == "articles.0.title") {
                    title = fieldValue;
                    if (true) {
                        dumbdisplay.writeComment(fieldValue);
                    }
                }
                else if (fieldId == "articles.0.description") {
                    description = fieldValue;
                }
                else if (fieldId == "articles.0.urlToImage") {
                    imageUrl = fieldValue;
                }
            }
        }
        resetDisplaying();
        if (title.length() > 0 || description.length() > 0) {
            textLayer->println(title + ":");
            textLayer->println(description);
            isIdle = false;
            String text = "[v1][h0]" + title;
            if (englishOnly) {
                text = "[g2]" + text;
            } else {
                text = "[g1]" + text;
            }
            synthesizeVoice(text);
        }
        if (imageUrl.length() > 0) {
            imageTunnel->reconnectTo(imageUrl);
            while (true) {
                int result = imageTunnel->checkResult();
                if (result == 1) {
                    imageLayer->unloadImageFile(IMAGE_FILE_NAME);
                    imageLayer->drawImageFileFit(IMAGE_FILE_NAME);
                }
                if (result != 0) {
                    break;
                }
            }
        }
        enableButtons();
    }
#endif

    if (langsButton->getFeedback()) {
        englishOnly = !englishOnly;
    }

    if ((millis() - checkMillis) > 2000) {
        checkMillis = millis();
        if (false) dumbdisplay.writeComment("checking status ...");
        synthesizer.write((byte) 0xFD);
        synthesizer.write((byte) 0x00);
        synthesizer.write((byte) 0x01);
        synthesizer.write((byte) 0x21);
    } else {
        if (synthesizer.available()) {
            int status = synthesizer.read();
            isIdle = status == 0x4F;
        }
    }
}
