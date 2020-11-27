#include "Arduino.h"
#include "dumbdisplay.h"


// class DDInputOutput {
//   public:
//     DDInputOutput(SoftwareSerial* pSS) {
//       this->pSS = pSS;
//       pSS->begin(115200);
//     }
//     inline bool available() {
//       return pSS->available();
//     }
//     inline char read() {
//       return pSS->read();
//     } 
//     inline void print(const char *p) {
//       pSS->print(p); 
//     }
//   private:
//     SoftwareSerial* pSS;  
// };

class DDReceivedData {
  public: 
    DDReceivedData(/*DDInputOutput* pIO*/) {
      //this->pIO = pIO;
    }
    bool available();
    String& get();
    void clear();
  private:
    //DDInputOutput* pIO;  
    String data;  
};



int _NextLid = 0;
DDInputOutput* _IO = NULL;  



// DDReceivedData::DDReceivedData(DDInputOutput* pIO) {
//   this->pIO = pIO;
// }
bool DDReceivedData::available() {
  bool done = false;
  if (_IO->available()) {
    char c =  _IO->read();
    if (c == '\n') {
      done = true;
    } else {
      data = data + c;
    }
  }
  return done;
}
String& DDReceivedData::get() {
  return data;
}
void DDReceivedData::clear() {
  data = "";
}





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
  long nextTime = 0;
  DDReceivedData receivedData/*(pIO)*/;
  while (true) {
    long now = millis();
    if (now > nextTime) {
      _IO->print("ddhello\n");
      nextTime = now + 1000;
    }
    if (receivedData.available()) {
      String data = receivedData.get();
//Serial.println(data);
      if (data == "ddhello")
        break;
      receivedData.clear();  
    }
  }

  nextTime = 0;
  receivedData.clear();
  while (true) {
    long now = millis();
    if (now > nextTime) {
      _IO->print(">init>:Arduino\n");
      nextTime = now + 1000;
    }
    if (receivedData.available()) {
      String data = receivedData.get();
//Serial.println(data);
      if (data == "<init<")
        break;
      receivedData.clear();  
    }
  }
}

void MicroBitLayer::showNum(int num) {
  _sendCommand1(/*pIO, */layerId, "shn", String(num));
}

MicroBitLayer* DumbDisplay::createMicroBitLayer(int width, int height) {
  int lid = _NextLid++;
  String layerId = String(lid);
  _sendCommand3(/*pIO, */layerId, "SU", String("mb"), String(width), String(height));
  return new MicroBitLayer(/*pIO, */layerId);
}




