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
const String NewsApiEndpoint = String("https://newsapi.org/v2/top-headlines?apiKey=") + NEWS_API__API_KEY;



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


void SynthesizeVoice(const String& text) {
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


void ResetDisplaying() {
    langsButton->disabled(true);
    newsButton->disabled(true);
    textLayer->clear();
    textLayer->setCursor(0, 0);
    imageLayer->clear();
}
void EnableButtons() {
    langsButton->disabled(false);
    newsButton->disabled(false);
}



const char* ImageFileName = "tempimage.png";

bool isIdle = false;
DDPendingValue<bool> englishOnly(true);
DDPendingValue<bool> requestNews;
DDPendingValue<String> adhocText;



void FeedbackHandler(DDLayer* layer, DDFeedbackType type, const DDFeedback& feedback) {
    if (layer == langsButton) {
        englishOnly = !englishOnly;
    } else if (layer == newsButton) {
        requestNews = true;
    } else if (layer == textLayer) {
        adhocText = feedback.text;
    }
}


void setup() {
    synthesizer.begin(115200);

    langsButton = dumbdisplay.createLcdLayer(12, 1);
    langsButton->backgroundColor("indigo");
    langsButton->border(1, "gray", "round");
    langsButton->setFeedbackHandler(FeedbackHandler, "f");
    langsButton->pixelColor("white");

    newsButton = dumbdisplay.createLcdLayer(12, 1);
    newsButton->backgroundColor("navy");
    newsButton->border(1, "gray", "round");
    newsButton->setFeedbackHandler(FeedbackHandler, "f");
    newsButton->pixelColor("white");
    newsButton->writeCenteredLine("News");

    textLayer = dumbdisplay.createGraphicalLayer(200, 80);
    textLayer->border(2, "darkblue");
    textLayer->padding(5);
    textLayer->setTextColor("blue");
    textLayer->backgroundColor("ivory");
    textLayer->setTextWrap(true);
    textLayer->setFeedbackHandler(FeedbackHandler, "f:keys");  // "feedback" is input text (with keyboard)

    newsTunnel = dumbdisplay.createFilteredJsonTunnel("", "title,urlToImage");  
    imageTunnel = dumbdisplay.createImageDownloadTunnel("", ImageFileName);

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

    EnableButtons();
}



void HandleGetAnotherNews() {
    ResetDisplaying();
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
    String endpoint = NewsApiEndpoint + ("&pageSize=1&category=" + category) + ("&country=" + country);
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
    ResetDisplaying();
    if (title.length() > 0) {
        textLayer->println(title);
        isIdle = false;
        String text = "[v1][h0]" + title;
        if (englishOnly) {
            text = "[g2]" + text;
        } else {
            text = "[g1]" + text;
        }
        SynthesizeVoice(text);
    }
    if (imageUrl.length() > 0) {
        imageTunnel->reconnectTo(imageUrl);
        while (true) {
            int result = imageTunnel->checkResult();
            if (result == 1) {
                imageLayer->drawImageFileFit(ImageFileName);
            }
            if (result != 0) {
                break;
            }
        }
    }
    EnableButtons();
}

void HandleAdhocText(const String& text) {
    ResetDisplaying();
    if (text.length() > 0) {
        textLayer->print(text);
        if (isIdle) {
            isIdle = false;
            String text = "[v1][h0]" + text;
            if (englishOnly) {
                text = "[g2]" + text;
            } else {
                text = "[g1]" + text;
            }
            dumbdisplay.writeComment(text);
            SynthesizeVoice(text);
        } else {
            dumbdisplay.writeComment("BUSY!");
        }
    }
    EnableButtons();
}


long checkMillis = 0;

void loop() {
    // check if englishOnly has "pending value" [since last check]
    if (englishOnly.acknowledge()) {
        if (englishOnly) {
            langsButton->writeCenteredLine("English");
        } else {
            langsButton->writeCenteredLine("Eng & 中文");
        }
    }

    // check if adhocText has "pending value" [since last check]
    if (adhocText.acknowledge()) {
        HandleAdhocText(adhocText);
    }

    // check if requestNews has "pending value" [since last check]
    if (requestNews.acknowledge()) {
        HandleGetAnotherNews();
    }

    if ((millis() - checkMillis) > 2000) {
        // check if synthesizer is idle
        synthesizer.write((byte) 0xFD);
        synthesizer.write((byte) 0x00);
        synthesizer.write((byte) 0x01);
        synthesizer.write((byte) 0x21);
        checkMillis = millis();
    } else {
        if (synthesizer.available()) {
            int status = synthesizer.read();
            isIdle = status == 0x4F;
        }
    }

    DDYield();  // yield to DD so that it can do its work
}
