#include "Arduino.h"
#include "dumbdisplay.h"


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



int _NextLid = 0;
DDInputOutput* _IO = NULL;  


void _sendCommand(String& layerId, const char *command, const String* pParam1, const String* pParam2, const String* pParam3) {
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
      }
    }
  }
  _IO->print("\n");
}  
void _sendCommand0(String& layerId, const char *command) {
  _sendCommand(layerId, command, NULL, NULL, NULL);
}  
void _sendCommand1(String& layerId, const char *command, const String& param1) {
  _sendCommand(layerId, command, &param1, NULL, NULL);
}  
void _sendCommand2(String& layerId, const char *command, const String& param1, const String& param2) {
  _sendCommand(layerId, command, &param1, &param2, NULL);
}  
void _sendCommand3(String& layerId, const char *command, const String& param1, const String& param2, const String& param3) {
  _sendCommand(layerId, command, &param1, &param2, &param3);
}  


DumbDisplay::DumbDisplay(DDInputOutput* pIO) {
  _IO = pIO;
}
void DumbDisplay::connect() {
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
}

void DDLayer::visibility(bool visible) {
  _sendCommand1(layerId, "visible", TO_BOOL(visible));
}
void DDLayer::opacity(int opacity) {
  _sendCommand1(layerId, "opacity", String(opacity));
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



void MicroBitLayer::showIcon(MBIcon icon) {
  _sendCommand1(layerId, "shi", String(icon));
}
void MicroBitLayer::showArrow(MBArrow arrow) {
  _sendCommand1(layerId, "sha", String(arrow));
}
void MicroBitLayer::showNumber(int num) {
  _sendCommand1(layerId, "shn", String(num));
}
void MicroBitLayer::showString(const String& str) {
  _sendCommand1(layerId, "shs", str);
}
void MicroBitLayer::plot(int x, int y) {
  _sendCommand2(layerId, "pl", String(x), String(y));
}
void MicroBitLayer::unplot(int x, int y) {
  _sendCommand2(layerId, "upl", String(x), String(y));
}
void MicroBitLayer::toggle(int x, int y) {
  _sendCommand2(layerId, "tggl", String(x), String(y));
}
void MicroBitLayer::showLeds(const String& ledPattern) {
  _sendCommand1(layerId, "shledpat", ledPattern);
}
void MicroBitLayer::clearScreen() {
  _sendCommand0(layerId, "cs");
}
void MicroBitLayer::ledColor(long color) {
  _sendCommand1(layerId, "ledc", HEX_COLOR(color));
}
void MicroBitLayer::ledColor(const String& color) {
  _sendCommand1(layerId, "ledc", color);
}

MicroBitLayer* DumbDisplay::createMicroBitLayer(int width, int height) {
  int lid = _NextLid++;
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("mb"), String(width), String(height));
  return new MicroBitLayer(layerId);
}




