// for a desciption of the experiment
// -- Arduino Speech Synthesizer Experiment with XFS5152CE
// -- https://www.youtube.com/watch?v=3aVNp6xhqVM 



// ***
// *** ESP32 DEV Kit
// ***
// *   Serial2 -- GPIO16 (RXD 2) => TX of XFS5152CE board; GPIO17 (TXD 2) => RX of XFS5152CE
#define synthesizer Serial2


// ***
// *** newsapi.org 
// ***
// *   reference: https://newsapi.org/docs/endpoints/top-headlines
#define NEWS_API_ENDPOINT "https://newsapi.org/v2/top-headlines?apiKey=<APIKEY>"


#define IMAGE_FILE_NAME   "tempimage.png"




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

JsonDDTunnel* newsTunnel;
SimpleToolDDTunnel* imageTunnel;


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
    synthesizer.begin(115200);

    langsButton = dumbdisplay.createLcdLayer(12, 1);
    langsButton->backgroundColor("indigo");
    langsButton->border(1, "gray", "round");
    langsButton->enableFeedback("f");

    newsButton = dumbdisplay.createLcdLayer(12, 1);
    newsButton->backgroundColor("navy");
    newsButton->border(1, "gray", "round");
    newsButton->enableFeedback("f");
    newsButton->writeCenteredLine("News");

    textLayer = dumbdisplay.createGraphicalLayer(200, 80);
    textLayer->border(2, "darkblue");
    textLayer->padding(5);
    textLayer->setTextColor("blue");
    textLayer->backgroundColor("ivory");
    textLayer->setTextWrap(true);
    textLayer->enableFeedback("f:keys");

    newsTunnel = dumbdisplay.createFilteredJsonTunnel("", "title,urlToImage");  
    imageTunnel = dumbdisplay.createImageDownloadTunnel("", IMAGE_FILE_NAME);

    imageLayer = dumbdisplay.createGraphicalLayer(300, 150);
    imageLayer->padding(5);
    imageLayer->border(10, "gray", "round");  
    imageLayer->noBackgroundColor();
    imageLayer->penColor("navy");

    dumbdisplay.configAutoPin(DD_AP_VERT_3(
        DD_AP_HORI_2(langsButton->getLayerId(), newsButton->getLayerId()),
        textLayer->getLayerId(),
        imageLayer->getLayerId()
    ));

    enableButtons();
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
                country = "hk";
            }
        }
        String endpoint = NEWS_API_ENDPOINT + ("&pageSize=1&category=" + category) + ("&country=" + country);
        newsTunnel->reconnectTo(endpoint);
        String title = "";
        String imageUrl = "";
        while (!newsTunnel->eof()) {
            if (newsTunnel->count() > 0) {
                textLayer->print(".");
                String fieldId;
                String fieldValue;
                newsTunnel->read(fieldId, fieldValue);
                if (fieldId == "articles.0.title") {
                    title = fieldValue;
                } else if (fieldId == "articles.0.urlToImage") {
                    imageUrl = fieldValue;
                }
            }
        }
        resetDisplaying();
        if (title.length() > 0) {
            textLayer->println(title);
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

    if (langsButton->getFeedback()) {
        englishOnly = !englishOnly;
    }

    if ((millis() - checkMillis) > 2000) {
        checkMillis = millis();
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
