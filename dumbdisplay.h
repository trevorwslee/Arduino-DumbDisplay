#ifndef dumbdisplay_h
#define dumbdisplay_h

#include <SoftwareSerial.h>

class DDInputOutput {
  public:
    virtual bool available();
    virtual char read();
    virtual void print(const char *p);
};

class DDSoftwareSerialIO: public DDInputOutput {
  public:
    DDSoftwareSerialIO(SoftwareSerial* pSS): DDInputOutput() {
      this->pSS = pSS;
      pSS->begin(115200);
    }
    bool available() {
      return pSS->available();
    }
    char read() {
      return pSS->read();
    } 
    void print(const char *p) {
      pSS->print(p); 
    }
  private:
    SoftwareSerial* pSS;  
};


class DDLayer {
  protected:
    DDLayer(const String& layerId) {
      this->layerId = String(layerId);
    }
  protected:
    String layerId;  
};


class MicroBitLayer: public DDLayer {
  public:
    MicroBitLayer(const String& layerId): DDLayer(layerId) {
    }
    void showNum(int num);

};

class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO);
    void connect();
    MicroBitLayer* createMicroBitLayer(int width, int height);
};



#endif
