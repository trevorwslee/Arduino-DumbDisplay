// ***
// * assume inclusion TFT_eSPI.h
// ***

//#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

#ifndef _dd_tft_util_h
#define _dd_tft_util_h

#include "_dd_misc.h"

class TftDDDebugInterface: public DrawTextDDDebugInterface {
  public:
    TftDDDebugInterface(TFT_eSPI& tft, int x = 0, int y = 0, uint8_t fontSize = 2, uint8_t font = 1, bool indicateSendCommand = false): tft(tft) {
      this->x = x;
      this->y = y;
      this->fontSize = fontSize;
      this->font = font;
      this->indicateSendCommand = indicateSendCommand;
    }
  public:
    virtual void logSendCommand(int state) {
      if (indicateSendCommand) {
        if (true) {
          tft.fillRect(x, y, x + 10, y + 10, TFT_WHITE);
          if (state == 1) {
            tft.fillCircle(x + 5, y + 5, 5, TFT_RED);
          }
        } else {
          tft.fillRect(x + 2, y + 2, x + 12, y + 12, TFT_WHITE);
          if (state == 1) {
            tft.fillCircle(x + 7, y + 7, 5, TFT_RED);
          }
        }
      }
    }
  protected:  
    virtual void drawText(const char* text, bool isError) {
      if (true) {
        if (isError) {
          showMsg(text, 80);
        } else {
          showMsg(text, 10);
        }
      } else {
        if (isError) {
          showMsg(text, 80);
        } else {
          showMsg(text, 16);
        }
      }
    }
  private:  
    void showMsg(const char* msg, int xOff) {
      uint32_t textcolor = tft.textcolor;
      uint32_t textbgcolor = tft.textbgcolor;
      uint8_t textsize = tft.textsize;
      tft.setTextColor(TFT_RED, TFT_WHITE);
      tft.setTextSize(fontSize);
      tft.drawString(msg, x + xOff, y, font);
      tft.setTextColor(textcolor, textbgcolor);
      tft.setTextSize(textsize);
    }
  protected:
    TFT_eSPI& tft;  
    int x;
    int y;
    uint8_t fontSize;
    uint8_t font;
    bool indicateSendCommand;
};


// class TftDDDebugInterface: public DDDebugInterface {
//   public:
//     TftDDDebugInterface(TFT_eSPI& tft, int x = 0, int y = 0, uint8_t fontSize = 2, uint8_t font = 1, bool indicateSendCommand = false): tft(tft) {
//       this->x = x;
//       this->y = y;
//       this->fontSize = fontSize;
//       this->font = font;
//       this->indicateSendCommand = indicateSendCommand;
//     }
//   public:
//     virtual void logConnectionState(DDDebugConnectionState connectionState) {
//       const char* state = NULL;
//       switch (connectionState) {
//         case DDDebugConnectionState::DEBUG_NOT_CONNECTED:
//           state = "NCed";
//           break;
//         case DDDebugConnectionState::DEBUG_CONNECTING:
//           state = "Cing ";
//           break;
//         case DDDebugConnectionState::DEBUG_CONNECTED:
//           state = "Ced  ";
//           break;
//         case DDDebugConnectionState::DEBUG_RECONNECTING:
//           state = "RCing";
//           break;
//         case DDDebugConnectionState::DEBUG_RECONNECTED:
//           state = "RCed ";
//           break;
//       }
//       if (state != NULL) {
//         showMsg(state, 16);
//       }
//     }
//     virtual void logSendCommand(int state) {
//       if (indicateSendCommand) {
//         tft.fillRect(x + 2, y + 2, x + 12, y + 12, TFT_WHITE);
//         if (state == 1) {
//           tft.fillCircle(x + 7, y + 7, 5, TFT_RED);
//         }
//       }
//     }
//     virtual void logError(const String& errMsg) {
//         showMsg("Err", 80);
//     }
//   protected:  
//     void showMsg(const char* msg, int xOff) {
//       uint32_t textcolor = tft.textcolor;
//       uint32_t textbgcolor = tft.textbgcolor;
//       uint8_t textsize = tft.textsize;
//       tft.setTextColor(TFT_RED, TFT_WHITE);
//       tft.setTextSize(fontSize);
//       tft.drawString(msg, x + xOff, y, font);
//       tft.setTextColor(textcolor, textbgcolor);
//       tft.setTextSize(textsize);
//     }
//   private:
//     TFT_eSPI& tft;  
//     int x;
//     int y;
//     uint8_t fontSize;
//     uint8_t font;
//     bool indicateSendCommand;
// };

#endif
