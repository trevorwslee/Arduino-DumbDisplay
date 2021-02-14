#include "Arduino.h"
#include "dumbdisplay.h"


#define HAND_SHAKE_GAP 1000
#define ENABLE_FEEDBACK
#define STORE_LAYERS

#define TO_BOOL(val) (val ? "1" : "0")

//#define DD_DEBUG_HS
//#define DD_DEBUG_SEND_COMMAND
//#define DEBUG_ECHO_COMMAND
#define SERIAL_ECHO_FEEDBACK


#define DEBUG_WITH_LED


// not flush seems to be a bit better for Serial (lost data)
#define FLUSH_AFTER_SENT_COMMAND false



namespace DDImpl {


class IOProxy {
  public: 
    IOProxy(DDInputOutput *pIO) {
      this->pIO = pIO;
    }
    bool available();
    String& get();
    void clear();
    void print(const String &s);
    void print(const char *p);
  private:
    DDInputOutput *pIO;
    bool fromSerial;
    String data;  
};

bool IOProxy::available() {
  bool done = false;
  if (true) {
    while (!done && pIO->available()) {
      char c =  pIO->read();
      if (c == '\n') {
        done = true;
      } else {
        data = data + c;
      }
    }
  } else {
    if (pIO->available()) {
      char c =  pIO->read();
      if (c == '\n') {
        done = true;
      } else {
        data = data + c;
      }
    }
  }
  return done;
}
String& IOProxy::get() {
  return data;
}
void IOProxy::clear() {
  data = "";
}
void IOProxy::print(const String &s) {
  pIO->print(s);
}
void IOProxy::print(const char *p) {
  pIO->print(p);
}



volatile bool _Connected = false;
volatile int _DDCompatibility = 0;
volatile int _NextLid = 0;
DDLayer** _DDLayerArray = NULL;
DDInputOutput* volatile _IO = NULL;
IOProxy* volatile _ConnectedIOProxy = NULL;
volatile bool _ConnectedFromSerial = false; 

#ifdef DEBUG_WITH_LED
volatile int _DebugLedPin = -1;
#endif
#ifdef SERIAL_ECHO_FEEDBACK 
volatile bool _DebugEnableSerialEchoFeedback = false;
#endif

volatile bool _SendingCommand = false;
volatile bool _HandlingFeedback = false;

void _Connect() {
  if (_Connected)
    return;
  _IO->preConnect();
#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;  
  bool debugLedOn;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
    debugLedOn = true;
  }
#endif
  {
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    IOProxy* pSerialIOProxy = NULL;
    DDInputOutput *pSIO = NULL;
    if (_IO->isBackupBySerial()) {
      pSIO = new DDInputOutput();
      pSerialIOProxy = new IOProxy(pSIO);
    }
    while (true) {
      long now = millis();
      if (now > nextTime) {
#ifdef DEBUG_WITH_LED
        if (debugLedPin != -1) {
          debugLedOn = !debugLedOn;
          digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
        }
#endif
        ioProxy.print("ddhello\n");
        if (pSerialIOProxy != NULL) 
          pSerialIOProxy->print("ddhello\n");
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:ddhello");
#endif        
        nextTime = now + HAND_SHAKE_GAP;
      }
      bool fromSerial = false;
      bool available = ioProxy.available();
      if (!available && pSerialIOProxy != NULL) {
        if (pSerialIOProxy->available()) {
          available = true;
          fromSerial = true;
        }
      }
      if (available) {
        String& data = fromSerial ? pSerialIOProxy->get() : ioProxy.get();
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:data-" + data);
#endif        
        if (data == "ddhello") {
          if (fromSerial) {
            _IO = pSIO;
            pSIO = NULL;
          }
          _ConnectedIOProxy = new IOProxy(_IO);
          _ConnectedFromSerial = fromSerial;
          break;
        }
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:DONE");
#endif        
        if (fromSerial) 
          pSerialIOProxy->clear();
        else
          ioProxy.clear();  
      }
    }
    if (pSerialIOProxy != NULL)
      delete pSerialIOProxy;
    if (pSIO != NULL)
      delete pSIO;
  }
  int compatibility = 0;
  { 
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    while (true) {
      long now = millis();
      if (now > nextTime) {
#ifdef DEBUG_WITH_LED
        if (debugLedPin != -1) {
          debugLedOn = !debugLedOn;
          digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
        }
#endif
        ioProxy.print(">init>:Arduino-c1\n");
        nextTime = now + HAND_SHAKE_GAP;
      }
      if (ioProxy.available()) {
        String& data = ioProxy.get();
        if (data == "<init<")
          break;
        if (data.startsWith("<init<:")) {
            compatibility = data.substring(7).toInt();
            break;
        }  
        ioProxy.clear();  
      }
    }
  }
  _Connected = true;
//  _ConnectedIOProxy = new IOProxy(_IO);
  _DDCompatibility = compatibility;
  if (false) {
    // ignore any input in 1000ms window
    delay(1000);
    while (_IO->available()) {
      _IO->read();
    }
  }
  if (true) {       
    _IO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
    //_IO->flush();
    if (false) {
      // *** debug code
      for (int i = 0; i < 10; i++) {
        delay(500);
        _IO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
      }
    }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** CONNECTED");
#endif        
  }
#ifdef DEBUG_WITH_LED
    if (debugLedPin != -1) {
      digitalWrite(debugLedPin, LOW);
    }
#endif
    // if (false) {
    //   // *** debug code
    //   _IO->print("// connection to DD made\n");
    //    _sendCommand0("", "// *** connection made ***");
    // }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** DONE MAKE CONNECTION");
#endif        
}

int _AllocLid() {
  _Connect();
  int lid = _NextLid++;
#ifdef STORE_LAYERS  
  DDLayer** oriLayerArray = _DDLayerArray;
  DDLayer** layerArray = (DDLayer**) malloc((lid + 1) * sizeof(DDLayer*));
  if (oriLayerArray != NULL) {
    memcpy(layerArray, oriLayerArray, lid * sizeof(DDLayer*));
    delete oriLayerArray;
  }
  _DDLayerArray = layerArray;
#endif
  return lid;
}
int _LayerIdToLid(const String& layerId) {
  return atoi(layerId.c_str());
}
void _PostCreateLayer(DDLayer* pLayer) {
#ifdef STORE_LAYERS  
  int lid = _LayerIdToLid(pLayer->getLayerId());
  _DDLayerArray[lid] = pLayer;
#endif
}
void _PreDeleteLayer(DDLayer* pLayer) {
#ifdef STORE_LAYERS  
  int lid = _LayerIdToLid(pLayer->getLayerId());
  _DDLayerArray[lid] = NULL;
#endif
}
String* _ReadFeedback() {
  if (_ConnectedIOProxy == NULL || !_ConnectedIOProxy->available()) {
    return NULL;
  }
  String data = _ConnectedIOProxy->get();
  String* pResData = new String(data);
  _ConnectedIOProxy->clear();
  return pResData;
  // const char* dataStr = data.c_str();
  // int dataLen = strlen(dataStr);
  // char* resDataStr = (char*) malloc(dataLen + 1);
  // strcpy(resDataStr, dataStr);
  // _ConnectedIOProxy->clear()
  // return resDataStr;
}



void __SendCommand(const String& layerId, const char* command, const String* pParam1, const String* pParam2, const String* pParam3, const String* pParam4, const String* pParam5, const String* pParam6, const String* pParam7, const String* pParam8) {
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print("// *** sent");
#endif        
  if (layerId != "") {
    _IO->print(layerId/*.c_str()*/);
    _IO->print(".");
  }
  _IO->print(command);
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" ...");
#endif        
  if (pParam1 != NULL) {
    _IO->print(":");
    _IO->print(*pParam1/*pParam1->c_str()*/);
    if (pParam2 != NULL) {
      _IO->print(",");
      _IO->print(*pParam2/*pParam2->c_str()*/);
      if (pParam3 != NULL) {
        _IO->print(",");
        _IO->print(*pParam3/*pParam3->c_str()*/);
        if (pParam4 != NULL) {
          _IO->print(",");
          _IO->print(*pParam4/*pParam4->c_str()*/);
          if (pParam5 != NULL) {
            _IO->print(",");
            _IO->print(*pParam5/*pParam5->c_str()*/);
            if (pParam6 != NULL) {
              _IO->print(",");
              _IO->print(*pParam6/*pParam6->c_str()*/);
              if (pParam7 != NULL) {
                _IO->print(",");
                _IO->print(*pParam7/*pParam7->c_str()*/);
                if (pParam8 != NULL) {
                  _IO->print(",");
                  _IO->print(*pParam8/*pParam8->c_str()*/);
                }
              }
            }
          }
        }
      }
    }
  }
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" COMMAND ");
#endif        
  _IO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _IO->flush();
  }
#ifdef DEBUG_ECHO_COMMAND
  _IO->print("// ");
  _IO->print(command);
  _IO->print("\n");
  _IO->flush();
#endif  
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.println(command);
#endif        
}  
void _HandleFeedback();
void _SendCommand(const String& layerId, const char* command, const String* pParam1 = NULL, const String* pParam2 = NULL, const String* pParam3 = NULL, const String* pParam4 = NULL, const String* pParam5 = NULL, const String* pParam6 = NULL, const String* pParam7 = NULL, const String* pParam8 = NULL) {
  bool alreadySendingCommand = _SendingCommand;  // not very accurate
  _SendingCommand = true;

#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
  }
#endif   

  if (command != NULL) {
    __SendCommand(layerId, command, pParam1, pParam2, pParam3, pParam4, pParam5, pParam6, pParam7, pParam8);
  }

#ifdef ENABLE_FEEDBACK
  if (!alreadySendingCommand) {
    _HandleFeedback();
  }
#endif

#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, LOW);
  }  
#endif

  _SendingCommand = false;
}

void _HandleFeedback() {
  bool alreadyHandlingFeedback = _HandlingFeedback;  // not very accurate
  _HandlingFeedback = true;
  if (!alreadyHandlingFeedback) {
    String* pFeedback = _ReadFeedback();
    if (pFeedback != NULL) {
#ifdef SERIAL_ECHO_FEEDBACK
    if (_DebugEnableSerialEchoFeedback && !_ConnectedFromSerial) {
      Serial.print("// FB -- ");
      Serial.print(*pFeedback);
      Serial.print("\n");
      Serial.flush();
    }
#endif  
      //_SendCommand("", ("// feedback -- " + *pFeedback).c_str());
#ifdef STORE_LAYERS
      int bufLen = pFeedback->length() + 1;
      char buf[bufLen];
      pFeedback->toCharArray(buf, bufLen);
      char* pos = strchr(buf, '.');
      if (pos != NULL) { 
        *pos = 0;
        int lid = _LayerIdToLid(buf);
        DDLayer* pLayer = _DDLayerArray[lid];
        if (pLayer != NULL) {
          DDFeedbackHandler handler = pLayer->getFeedbackHandler();
          if (handler != NULL) {
            handler(pLayer, CLICK, 0, 0);
            //_SendCommand("", ("// feedback (" + String(lid) + ") -- " + *pFeedback).c_str());
          }
        }
      }
#endif  
      delete pFeedback;
    }
    _HandlingFeedback = false;
  }
}

void _Delay(unsigned long ms) {
  unsigned long delayMicros = ms * 1000;
	unsigned long start = micros();
  while (true) {
    _HandleFeedback();
    unsigned long remain = delayMicros - (micros() - start);
    if (remain > 50000) {
      delay(50);
    } else if (remain > 0) {
      delay(remain / 1000);
      break;
    } else {
      break;
    }
  }
}



inline void _sendCommand0(const String& layerId, const char *command) {
  _SendCommand(layerId, command);
}  
inline void _sendCommand1(const String& layerId, const char *command, const String& param1) {
  _SendCommand(layerId, command, &param1);
}  
inline void _sendCommand2(const String& layerId, const char *command, const String& param1, const String& param2) {
  _SendCommand(layerId, command, &param1, &param2);
}  
inline void _sendCommand3(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3) {
  _SendCommand(layerId, command, &param1, &param2, &param3);
}  
inline void _sendCommand4(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4);
}
inline void _sendCommand5(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5);
}
inline void _sendCommand6(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6);
}
inline void _sendCommand7(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6, const String& param7) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6, &param7);
}
inline void _sendCommand8(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6, const String& param7, const String& param8) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6, &param7, &param8);
}





}



//*************/
/** EXPORTED **/
//*************/


using namespace DDImpl;


DDLayer::DDLayer(int layerId) {
  this->layerId = String(layerId);
}
void DDLayer::visibility(bool visible) {
  _sendCommand1(layerId, "visible", TO_BOOL(visible));
}
void DDLayer::opacity(int opacity) {
  _sendCommand1(layerId, "opacity", String(opacity));
}
void DDLayer::padding(int left, int top, int right, int bottom) {
  _sendCommand4(layerId, "padding", String(left), String(top), String(right), String(bottom));
}
void DDLayer::noPadding() {
  _sendCommand0(layerId, "padding");
}
void DDLayer::clear() {
  _sendCommand0(layerId, "clear");
}
// void DDLayer::backgroundColor(long color) {
//   _sendCommand1(layerId, "bgcolor", HEX_COLOR(color));
// }
void DDLayer::backgroundColor(const String& color) {
  _sendCommand1(layerId, "bgcolor", color);
}
void DDLayer::noBackgroundColor() {
  _sendCommand0(layerId, "nobgcolor");
}
void DDLayer::writeComment(const String& comment) {
  _sendCommand0("", ("// " + layerId + ": " + comment).c_str());
}
//void DDLayer::setFeedbackHandler(void (*handler)(DDFeedbackType, int, int)) {
void DDLayer::setFeedbackHandler(DDFeedbackHandler handler) {
  bool enable = handler != NULL;
  _sendCommand1(layerId, "feedback", TO_BOOL(enable));
  feedbackHandler = handler;
}



void MbDDLayer::showIcon(MbIcon icon) {
  _sendCommand1(layerId, "shi", String(icon));
}
void MbDDLayer::showArrow(MbArrow arrow) {
  _sendCommand1(layerId, "sha", String(arrow));
}
void MbDDLayer::showNumber(int num) {
  _sendCommand1(layerId, "shn", String(num));
}
void MbDDLayer::showString(const String& str) {
  _sendCommand1(layerId, "shs", str);
}
void MbDDLayer::plot(int x, int y) {
  _sendCommand2(layerId, "pl", String(x), String(y));
}
void MbDDLayer::unplot(int x, int y) {
  _sendCommand2(layerId, "upl", String(x), String(y));
}
void MbDDLayer::toggle(int x, int y) {
  _sendCommand2(layerId, "tggl", String(x), String(y));
}
void MbDDLayer::showLeds(const String& ledPattern) {
  _sendCommand1(layerId, "shledpat", ledPattern);
}
// void MicroBitLayer::clearScreen() {
//   _sendCommand0(layerId, "cs");
// }
// void MbDDLayer::ledColor(long color) {
//   _sendCommand1(layerId, "ledc", HEX_COLOR(color));
// }
void MbDDLayer::ledColor(const String& color) {
  _sendCommand1(layerId, "ledc", color);
}

void TurtleDDLayer::forward(int distance, bool withPen) {
  _sendCommand1(layerId, withPen ? "fd" : "dlfd", String(distance));
}
void TurtleDDLayer::backward(int distance, bool withPen) {
  _sendCommand1(layerId, withPen ? "bk" : "dlbk", String(distance));
}
void TurtleDDLayer::leftTurn(int angle) {
  _sendCommand1(layerId, "lt", String(angle));
}
void TurtleDDLayer::rightTurn(int angle) {
  _sendCommand1(layerId, "rt", String(angle));
}
void TurtleDDLayer::home(bool withPen) {
  _sendCommand0(layerId, withPen ? "home" : "jhome");
}
void TurtleDDLayer::goTo(int x, int y, bool withPen) {
  _sendCommand2(layerId, withPen ? "goto" : "jto", String(x), String(y));
}
void TurtleDDLayer::goBy(int byX, int byY, bool withPen) {
  _sendCommand2(layerId, withPen ? "goby" : "jby", String(byX), String(byY));
}
void TurtleDDLayer::setHeading(int angle) {
  _sendCommand1(layerId, "seth", String(angle));
}
void TurtleDDLayer::penUp() {
  _sendCommand0(layerId, "pu");
}
void TurtleDDLayer::penDown() {
  _sendCommand0(layerId, "pd");
}
void TurtleDDLayer::penSize(int size) {
  _sendCommand1(layerId, "pensize", String(size));
}
// void TurtleDDLayer::penColor(long color) {
//   _sendCommand1(layerId, "pencolor", HEX_COLOR(color));
// }
void TurtleDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, "pencolor", color);
}
// void TurtleDDLayer::fillColor(long color) {
//   _sendCommand1(layerId, "fillcolor", HEX_COLOR(color));
// }
void TurtleDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, "fillcolor", color);
}
void TurtleDDLayer::noFillColor() {
  _sendCommand0(layerId, "nofillcolor");
}
void TurtleDDLayer::penFilled(bool filled) {
  _sendCommand1(layerId, "pfilled", TO_BOOL(filled));
}
void TurtleDDLayer::setTextSize(int size) {
  _sendCommand1(layerId, "ptextsize", String(size));
}
void TurtleDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, "ptextfont", fontName, String(size));
}
void TurtleDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? "ccircle" : "circle", String(radius));
}
void TurtleDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "coval" : "oval", String(width), String(height));
}
void TurtleDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "crect" : "rect", String(width), String(height));
}
void TurtleDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, "trisas", String(side1), String(angle), String(side2));
}
void TurtleDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, "trisas", String(side), String(angle));
}
void TurtleDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, "poly", String(side), String(vertexCount));
}
void TurtleDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? "cpolyin" : "cpoly", String(radius), String(vertexCount));
}
void TurtleDDLayer::write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? "drawtext" : "write", text);
}
void LedGridDDLayer::turnOn(int x, int y) {
  _sendCommand2(layerId, "ledon", String(x), String(y));
}
void LedGridDDLayer::turnOff(int x, int y) {
  _sendCommand2(layerId, "ledoff", String(x), String(y));
}
void LedGridDDLayer::toggle(int x, int y) {
  _sendCommand2(layerId, "ledtoggle", String(x), String(y));
}
void LedGridDDLayer::horizontalBar(int count, bool rightToLeft) {
  _sendCommand2(layerId, "ledhoribar", String(count), TO_BOOL(rightToLeft));
}
void LedGridDDLayer::verticalBar(int count, bool bottomToTop) {
  _sendCommand2(layerId, "ledvertbar", String(count), TO_BOOL(bottomToTop));
}
// void LedGridDDLayer::onColor(long color) {
//   _sendCommand1(layerId, "ledoncolor", HEX_COLOR(color));
// }
void LedGridDDLayer::onColor(const String& color) {
  _sendCommand1(layerId, "ledoncolor", color);
}
// void LedGridDDLayer::offColor(long color) {
//   _sendCommand1(layerId, "ledoffcolor", HEX_COLOR(color));
// }
void LedGridDDLayer::offColor(const String& color) {
  _sendCommand1(layerId, "ledoffcolor", color);
}
void LedGridDDLayer::noOffColor() {
  _sendCommand0(layerId, "ledoffcolor");
}


void LcdDDLayer::print(const String& text) {
  _sendCommand1(layerId, "print", text);
}
void LcdDDLayer::home() {
  _sendCommand0(layerId, "home");
}
void LcdDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, "setcursor", String(x), String(y));
}
void LcdDDLayer::cursor() {
  _sendCommand1(layerId, "cursor", TO_BOOL(true));
}
void LcdDDLayer::noCursor() {
  _sendCommand1(layerId, "cursor", TO_BOOL(false));
}
void LcdDDLayer::autoscroll() {
  _sendCommand1(layerId, "autoscroll", TO_BOOL(true));
}
void LcdDDLayer::noAutoscroll() {
  _sendCommand1(layerId, "autoscroll", TO_BOOL(false));
}
void LcdDDLayer::display() {
  _sendCommand1(layerId, "display", TO_BOOL(true));
}
void LcdDDLayer::noDisplay() {
  _sendCommand1(layerId, "display", TO_BOOL(false));
}
void LcdDDLayer::scrollDisplayLeft() {
  _sendCommand0(layerId, "scrollleft");
}
void LcdDDLayer::scrollDisplayRight() {
  _sendCommand0(layerId, "scrollright");
}
void LcdDDLayer::writeLine(const String& text, int y, const String& align) {
  _sendCommand3(layerId, "writeline", String(y), align, text);
}
void LcdDDLayer::pixelColor(const String &color) {
  _sendCommand1(layerId, "pixelcolor", color);
}
void LcdDDLayer::bgPixelColor(const String &color) {
  _sendCommand1(layerId, "bgpixelcolor", color);
}
void LcdDDLayer::noBgPixelColor() {
  _sendCommand0(layerId, "bgpixelcolor");
}


void GraphicalDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, "setcursor", String(x), String(y));
}
void GraphicalDDLayer::moveCursorBy(int byX, int byY) {
  _sendCommand2(layerId, "movecursorby", String(byX), String(byY));
}
// void GraphicalDDLayer::setTextColor(const String& color) {
//   _sendCommand1(layerId, "textcolor", color);
// }
void GraphicalDDLayer::setTextColor(const String& color, const String& bgColor) {
  _sendCommand2(layerId, "textcolor", color, bgColor);
}
void GraphicalDDLayer::setTextSize(int size) {
  _sendCommand1(layerId, "textsize", String(size));
}
void GraphicalDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, "textfont", fontName, String(size));
}
void GraphicalDDLayer::setTextWrap(bool wrapOn) {
  _sendCommand1(layerId, "settextwrap", TO_BOOL(wrapOn));
}
void GraphicalDDLayer::fillScreen(const String& color) {
  _sendCommand1(layerId, "fillscreen", color);
}
void GraphicalDDLayer::print(const String& text) {
  _sendCommand1(layerId, "print", text);
}
void GraphicalDDLayer::println(const String& text) {
  _sendCommand1(layerId, "println", text);
}
void GraphicalDDLayer::drawChar(int x, int y, char c, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, "drawchar", String(x), String(y), color, bgColor, String(size), String(c));
}
void GraphicalDDLayer::drawPixel(int x, int y, const String& color) {
  _sendCommand3(layerId, "drawpixel", String(x), String(y), color);
}
void GraphicalDDLayer::drawLine(int x1, int y1, int x2, int y2, const String& color) {
  _sendCommand5(layerId, "drawline", String(x1), String(y1), String(x2), String(y2), color);
}
void GraphicalDDLayer::drawRect(int x, int y, int w, int h, const String& color) {
  _sendCommand5(layerId, "drawrect", String(x), String(y), String(w), String(h), color);
}
void GraphicalDDLayer::fillRect(int x, int y, int w, int h, const String& color) {
  _sendCommand6(layerId, "drawrect", String(x), String(y), String(w), String(h), color, TO_BOOL(true));
}
void GraphicalDDLayer::drawCircle(int x, int y, int r, const String& color) {
  _sendCommand4(layerId, "drawcircle", String(x), String(y), String(r), color);
}
void GraphicalDDLayer::fillCircle(int x, int y, int r, const String& color) {
  _sendCommand5(layerId, "drawcircle", String(x), String(y), String(r), color, TO_BOOL(true));
}
void GraphicalDDLayer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color) {
  _sendCommand7(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color);
}
void GraphicalDDLayer::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color) {
  _sendCommand8(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(true));
}
void GraphicalDDLayer::drawRoundRect(int x, int y, int w, int h, int r, const String& color) {
  _sendCommand6(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color);
}
void GraphicalDDLayer::fillRoundRect(int x, int y, int w, int h, int r, const String& color) {
  _sendCommand7(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color, TO_BOOL(true));
}
void GraphicalDDLayer::forward(int distance) {
  _sendCommand1(layerId, "fd", String(distance));
}
void GraphicalDDLayer::leftTurn(int angle) {
  _sendCommand1(layerId, "lt", String(angle));
}
void GraphicalDDLayer::rightTurn(int angle) {
  _sendCommand1(layerId, "rt", String(angle));
}
void GraphicalDDLayer::setHeading(int angle) {
  _sendCommand1(layerId, "seth", String(angle));
}
void GraphicalDDLayer::penSize(int size) {
  _sendCommand1(layerId, "pensize", String(size));
}
void GraphicalDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, "pencolor", color);
}
void GraphicalDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, "fillcolor", color);
}
void GraphicalDDLayer::noFillColor() {
  _sendCommand0(layerId, "nofillcolor");
}
void GraphicalDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? "ccircle" : "circle", String(radius));
}
void GraphicalDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "coval" : "oval", String(width), String(height));
}
void GraphicalDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "crect" : "rect", String(width), String(height));
}
void GraphicalDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, "trisas", String(side1), String(angle), String(side2));
}
void GraphicalDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, "trisas", String(side), String(angle));
}
void GraphicalDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, "poly", String(side), String(vertexCount));
}
void GraphicalDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? "cpolyin" : "cpoly", String(radius), String(vertexCount));
}
void GraphicalDDLayer::write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? "drawtext" : "write", text);
}

void SevenSegmentRowDDLayer::segmentColor(const String& color) {
  _sendCommand1(layerId, "segcolor", color);
}
void SevenSegmentRowDDLayer::turnOn(const String& segments, int digitIdx) {
  _sendCommand2(layerId, "segon", segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::turnOff(const String& segments, int digitIdx) {
  _sendCommand2(layerId, "segoff", segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::showNumber(float number) {
  _sendCommand1(layerId, "shownumber", String(number, 5));
}
void SevenSegmentRowDDLayer::showHexNumber(int number) {
  _sendCommand1(layerId, "showhex", String(number));
}
void SevenSegmentRowDDLayer::showFormatted(const String& formatted) {
  _sendCommand1(layerId, "showformatted", formatted);
}




DumbDisplay::DumbDisplay(DDInputOutput* pIO) {
  _IO = pIO;
}
void DumbDisplay::connect() {
  _Connect();
}
void DumbDisplay::configPinFrame(int xUnitCount, int yUnitCount) {
  _Connect();
  if (xUnitCount != 100 || yUnitCount != 100) {
    _sendCommand2("", "CFGPF", String(xUnitCount), String(yUnitCount));
  }
}
void DumbDisplay::configAutoPin(const String& layoutSpec) {
  _Connect();
  _sendCommand1("", "CFGAP", layoutSpec);
}
MbDDLayer* DumbDisplay::createMicrobitLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("mb"), String(width), String(height));
  MbDDLayer* pLayer = new MbDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
TurtleDDLayer* DumbDisplay::createTurtleLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("turtle"), String(width), String(height));
  TurtleDDLayer* pLayer = new TurtleDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
LedGridDDLayer* DumbDisplay::createLedGridLayer(int colCount, int rowCount, int subColCount, int subRowCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("ledgrid"), String(colCount), String(rowCount), String(subColCount), String(subRowCount));
  LedGridDDLayer* pLayer = new LedGridDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
LcdDDLayer* DumbDisplay::createLcdLayer(int colCount, int rowCount, int charHeight, const String& fontName) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("lcd"), String(colCount), String(rowCount), String(charHeight), fontName);
  LcdDDLayer* pLayer = new LcdDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
GraphicalDDLayer* DumbDisplay::createGraphicalLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("graphical"), String(width), String(height));
  GraphicalDDLayer* pLayer = new GraphicalDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
SevenSegmentRowDDLayer* DumbDisplay::create7SegmentRowLayer(int digitCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand2(layerId, "SU", String("7segrow"), String(digitCount));
  SevenSegmentRowDDLayer* pLayer = new SevenSegmentRowDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
void DumbDisplay::pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand5(pLayer->getLayerId(), "PIN", String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::deleteLayer(DDLayer *pLayer) {
  _sendCommand0(pLayer->getLayerId(), "DEL");
  _PreDeleteLayer(pLayer);
  delete pLayer;
}
void DumbDisplay::writeComment(const String& comment) {
  _sendCommand0("", ("// " + comment).c_str());
}


void DumbDisplay::debugSetup(int debugLedPin, bool enableSerialEchoFeedback) {
#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
     pinMode(debugLedPin, OUTPUT);
   }
  _DebugLedPin = debugLedPin;
#endif  
#ifdef SERIAL_ECHO_FEEDBACK
  _DebugEnableSerialEchoFeedback = enableSerialEchoFeedback;
#endif
}
// void DumbDisplay::delay(unsigned long ms) {
//   _Delay(ms);
// }


void DDDelay(unsigned long ms) {
  _Delay(ms);
}
void DDYield() {
    _HandleFeedback();
}



