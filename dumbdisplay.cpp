#include "Arduino.h"
#include "dumbdisplay.h"

const bool DEBUG_SERIAL = false;

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
if (DEBUG_SERIAL) Serial.print("-- ");    
if (DEBUG_SERIAL) Serial.print(p);    
  pIO->print(p);
}



int _NextLid = 0;
DDInputOutput* _IO = NULL;  


void _sendCommand1(String& layerId, const char *command, const String& param1) {
  _IO->print(layerId.c_str());
  _IO->print(".");
  _IO->print(command);
  _IO->print(":");
  _IO->print(param1.c_str());
  _IO->print("\n");
}  
void _sendCommand3(String& layerId, const char *command, const String& param1, const String& param2, const String& param3) {
  _IO->print(layerId.c_str());
  _IO->print(".");
  _IO->print(command);
  _IO->print(":");
  _IO->print(param1.c_str());
  _IO->print(",");
  _IO->print(param2.c_str());
  _IO->print(",");
  _IO->print(param3.c_str());
  _IO->print("\n");
}  


DumbDisplay::DumbDisplay(DDInputOutput* pIO) {
  _IO = pIO;
}
void DumbDisplay::connect() {
if (DEBUG_SERIAL) Serial.println("connect ...");   
  {
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    IOProxy* pSerialIOProxy = NULL;
    DDInputOutput *pSIO = NULL;
    if (_IO->allowSerial()) {
      pSIO = new DDInputOutput(true);
      pSerialIOProxy = new IOProxy(pSIO);
    }
if (DEBUG_SERIAL) Serial.println("connected");    
    while (true) {
      long now = millis();
      if (now > nextTime) {
//_IO->print("XXX\n");
        ioProxy.print("ddhello\n");
        if (pSerialIOProxy != NULL) 
          pSerialIOProxy->print("ddhello\n");
        nextTime = now + 1000;
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
        String data = fromSerial ? pSerialIOProxy->get() : ioProxy.get();
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
        nextTime = now + 1000;
      }
      if (ioProxy.available()) {
        String data = ioProxy.get();
        if (data == "<init<")
          break;
        ioProxy.clear();  
      }
    }
  }
}

void MicroBitLayer::showNum(int num) {
  _sendCommand1(layerId, "shn", String(num));
}

MicroBitLayer* DumbDisplay::createMicroBitLayer(int width, int height) {
  int lid = _NextLid++;
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("mb"), String(width), String(height));
  return new MicroBitLayer(layerId);
}




