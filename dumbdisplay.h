#ifndef dumbdisplay_h
#define dumbdisplay_h

#include <SoftwareSerial.h>


class DDInputOutput {
  public:
    DDInputOutput(SoftwareSerial* pSS) {
      this->pSS = pSS;
      pSS->begin(115200);
    }
    inline bool available() {
      return pSS->available();
    }
    inline char read() {
      return pSS->read();
    } 
    inline void print(const char *p) {
      pSS->print(p); 
    }
  private:
    SoftwareSerial* pSS;  
};

// class DDReceivedData {
//   public: 
//     DDReceivedData(/*DDInputOutput* pIO*/) {
//       //this->pIO = pIO;
//     }
//     bool available();
//     String& get();
//     void clear();
//   private:
//     DDInputOutput* pIO;  
//     String data;  
// };


class DDLayer {
  protected:
    DDLayer(/*DDInputOutput* pIO, */const String& layerId) {
      //this->pIO = pIO;
      this->layerId = String(layerId);
    }
  protected:
    //DDInputOutput* pIO;
    String layerId;  
};


class MicroBitLayer: public DDLayer {
  public:
    MicroBitLayer(/*DDInputOutput* pIO, */const String& layerId): DDLayer(/*pIO, */layerId) {
    }
    void showNum(int num);

};

class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO);
    // DumbDisplay(DDInputOutput* pIO) {
    //   this->pIO = pIO;      
    // }
    void connect();
    MicroBitLayer* createMicroBitLayer(int width, int height);
  // private:
  //   void sendCommand3(String& layerId, const char *command, const String& param1, const String& param2, const String& param3);  
  private:
    //DDInputOutput* pIO;  
    //int nextLid = 0;
};



#endif
