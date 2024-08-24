// ***
// * assume inclusion Adafruit_SSD1306.h
// ***
//#include <Adafruit_SSD1306.h>

#ifndef _dd_oled_util_h
#define _dd_oled_util_h


class OledDDDebugInterface: public DrawTextDDDebugInterface {
  public:
    OledDDDebugInterface(Adafruit_SSD1306& display, int x = 0, int y = 0/*, uint8_t fontSize = 2, uint8_t font = 1, */, bool indicateSendCommand = false): display(display) {
      this->x = x;
      this->y = y;
      this->indicateSendCommand = indicateSendCommand;
    }
  public:
    virtual void logSendCommand(int state) {
      if (indicateSendCommand) {
        display.fillRect(x, y, x + 8, y + 8, WHITE);
        if (state == 1) {
          display.fillCircle(x + 4, y + 4, 4, BLACK);
        }
        display.display();
      }
    }
  protected:  
    virtual void drawText(const char* text, bool isError) {
      if (true) {
        if (isError) {
          showMsg(text, 70);
        } else {
          showMsg(text, 8);
        }
      } else {
        if (isError) {
          showMsg(text, 70);
        } else {
          showMsg(text, 12);
        }
      }
    }
  private:  
    void showMsg(const char* msg, int xOff) {
      const int charWidth = 6;
      for (int i = 0;; i++) {
        char c = msg[i];
        if (c == 0) {
          break;
        }
        display.drawChar(x + xOff + charWidth * i, y, c, WHITE, BLACK, 1);
      }
      display.display();
    }
  private:
    Adafruit_SSD1306& display;  
    int x;
    int y;
    bool indicateSendCommand;
};


// class OledDDDebugInterface: public DDDebugInterface {
//   public:
//     OledDDDebugInterface(Adafruit_SSD1306& display, int x = 0, int y = 0/*, uint8_t fontSize = 2, uint8_t font = 1, */, bool indicateSendCommand = false): display(display) {
//       this->x = x;
//       this->y = y;
//       //this->fontSize = fontSize;
//       //this->font = font;
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
//         showMsg(state, 12/*16*/);
//       }
//     }
//     virtual void logSendCommand(int state) {
//       if (indicateSendCommand) {
//         display.fillRect(x, y, x + 8, y + 8, WHITE);
//         if (state == 1) {
//           display.fillCircle(x + 4, y + 4, 4, BLACK);
//         }
//         display.display();
//         // display.fillRect(x + 2, y + 2, x + 12, y + 12, WHITE);
//         // if (state == 1) {
//         //   display.fillCircle(x + 7, y + 7, 5, BLACK);
//         // }
//         // display.display();
//       }
//     }
//     virtual void logError(const String& errMsg) {
//         showMsg("Err", 70/*80*/);
//     }
//   private:  
//     void showMsg(const char* msg, int xOff) {
//       const int charWidth = 6;
//       for (int i = 0;; i++) {
//         char c = msg[i];
//         if (c == 0) {
//           break;
//         }
//         display.drawChar(x + xOff + charWidth * i, y, c, WHITE, BLACK, 1);
//       }
//       display.display();
//     }
//   private:
//     Adafruit_SSD1306& display;  
//     int x;
//     int y;
//     bool indicateSendCommand;
// };

#endif
