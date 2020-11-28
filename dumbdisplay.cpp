#include "Arduino.h"
#include "dumbdisplay.h"


//#define DD_DEBUG

#define HAND_SHAKE_GAP 500

#define HEX_COLOR(color) ("#" + String(color, 16))
#define TO_BOOL(val) (val ? "1" : "0")

class IOProxy {
  public: 
    IOProxy(DDInputOutput *pIO) {
      this->pIO = pIO;
    }
    bool available();
    String& get();
    void clear();
    void print(const char *p);
  private:
    DDInputOutput *pIO;
    String data;  
};

bool IOProxy::available() {
  bool done = false;
  if (pIO->available()) {
    char c =  pIO->read();
    if (c == '\n') {
      done = true;
    } else {
      data = data + c;
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
void IOProxy::print(const char *p) {
  pIO->print(p);
}



bool _Connected = false;
int _NextLid = 0;
DDInputOutput* _IO = NULL;  


void _sendCommand(String& layerId, const char *command, const String* pParam1, const String* pParam2, const String* pParam3, const String* pParam4, const String* pParam5) {
  _IO->print(layerId.c_str());
  _IO->print(".");
  _IO->print(command);
  if (pParam1 != NULL) {
    _IO->print(":");
    _IO->print(pParam1->c_str());
    if (pParam2 != NULL) {
      _IO->print(",");
      _IO->print(pParam2->c_str());
      if (pParam3 != NULL) {
        _IO->print(",");
        _IO->print(pParam3->c_str());
        if (pParam4 != NULL) {
          _IO->print(",");
          _IO->print(pParam4->c_str());
          if (pParam5 != NULL) {
            _IO->print(",");
            _IO->print(pParam5->c_str());
          }
        }
      }
    }
  }
  _IO->print("\n");
}  
void _sendCommand0(String& layerId, const char *command) {
  _sendCommand(layerId, command, NULL, NULL, NULL, NULL, NULL);
}  
void _sendCommand1(String& layerId, const char *command, const String& param1) {
  _sendCommand(layerId, command, &param1, NULL, NULL, NULL, NULL);
}  
void _sendCommand2(String& layerId, const char *command, const String& param1, const String& param2) {
  _sendCommand(layerId, command, &param1, &param2, NULL, NULL, NULL);
}  
void _sendCommand3(String& layerId, const char *command, const String& param1, const String& param2, const String& param3) {
  _sendCommand(layerId, command, &param1, &param2, &param3, NULL, NULL);
}  
void _sendCommand4(String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4) {
  _sendCommand(layerId, command, &param1, &param2, &param3, &param4, NULL);
}
void _sendCommand5(String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5) {
  _sendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5);
}



void _Connect() {
  if (_Connected)
    return;
  {
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    IOProxy* pSerialIOProxy = NULL;
    DDInputOutput *pSIO = NULL;
    if (_IO->allowSerial()) {
      pSIO = new DDInputOutput();
      pSerialIOProxy = new IOProxy(pSIO);
    }
    while (true) {
      long now = millis();
      if (now > nextTime) {
        ioProxy.print("ddhello\n");
        if (pSerialIOProxy != NULL) 
          pSerialIOProxy->print("ddhello\n");
#ifdef DD_DEBUG          
        if (!_IO->allowSerial()) {
          Serial.println("handshake:ddhello");
        }  
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
        if (data == "ddhello") {
          if (fromSerial) {
            _IO = pSIO;
            pSIO = NULL;
          }
          break;
        }
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
  { 
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    while (true) {
      long now = millis();
      if (now > nextTime) {
        ioProxy.print(">init>:Arduino\n");
        nextTime = now + HAND_SHAKE_GAP;
      }
      if (ioProxy.available()) {
        String& data = ioProxy.get();
        if (data == "<init<")
          break;
        ioProxy.clear();  
      }
    }
  }
  _Connected = true;
}

int _AllocLayerId() {
  _Connect();
  return _NextLid++;
}

DumbDisplay::DumbDisplay(DDInputOutput* pIO) {
  _IO = pIO;
}

MbDDLayer* DumbDisplay::createMicrobitLayer(int width, int height) {
#ifdef DD_DEBUG          
  if (!_IO->allowSerial()) Serial.println("createMicrobitLayer"); 
#endif
  int lid = _AllocLayerId();
#ifdef DD_DEBUG          
  if (!_IO->allowSerial()) Serial.println("allocated layer id"); 
#endif
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("mb"), String(width), String(height));
  return new MbDDLayer(lid);
}
TurtleDDLayer* DumbDisplay::createTurtleLayer(int width, int height) {
  int lid = _AllocLayerId();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("turtle"), String(width), String(height));
  return new TurtleDDLayer(lid);
}
LedGridDDLayer* DumbDisplay::createLedGridLayer(int colCount, int rowCount, int subColCount, int subRowCount) {
  int lid = _AllocLayerId();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("ledgrid"), String(colCount), String(rowCount));
  return new LedGridDDLayer(lid);
}
void DumbDisplay::deleteLayer(DDLayer *pLayer) {
  _sendCommand0(pLayer->getLayerId(), "DEL");
  delete pLayer;
}


DDLayer::DDLayer(int layerId) {
  this->layerId = String(layerId);
}
void DDLayer::visibility(bool visible) {
  _sendCommand1(layerId, "visible", TO_BOOL(visible));
}
void DDLayer::opacity(int opacity) {
  _sendCommand1(layerId, "opacity", String(opacity));
}
void DDLayer::clear() {
  _sendCommand0(layerId, "clear");
}
void DDLayer::backgroundColor(long color) {
  _sendCommand1(layerId, "bgcolor", HEX_COLOR(color));
}
void DDLayer::backgroundColor(const String& color) {
  _sendCommand1(layerId, "bgcolor", color);
}
void DDLayer::noBackgroundColor() {
  _sendCommand0(layerId, "nobgcolor");
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
void MbDDLayer::ledColor(long color) {
  _sendCommand1(layerId, "ledc", HEX_COLOR(color));
}
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
  _sendCommand2(layerId, withPen ? "goto" : "hto", String(x), String(y));
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
void TurtleDDLayer::penColor(long color) {
  _sendCommand1(layerId, "pencolor", HEX_COLOR(color));
}
void TurtleDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, "pencolor", color);
}
void TurtleDDLayer::fillColor(long color) {
  _sendCommand1(layerId, "fillcolor", HEX_COLOR(color));
}
void TurtleDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, "fillcolor", color);
}
void TurtleDDLayer::noFillColor() {
  _sendCommand0(layerId, "nofillcolor");
}
void TurtleDDLayer::penFilled(bool filled) {
  _sendCommand1(layerId, "pfilled", TO_BOOL(filled));
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
void LedGridDDLayer::horizontalBar(int count) {
  _sendCommand1(layerId, "ledhoribar", String(count));
}
void LedGridDDLayer::verticalBar(int count) {
  _sendCommand1(layerId, "ledvertbar", String(count));
}





