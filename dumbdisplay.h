#ifndef dumbdisplay_h
#define dumbdisplay_h

#include <SoftwareSerial.h>

class DDInputOutput {
  public:
    DDInputOutput(bool enableSerial) {
      this->enableSerial = enableSerial;
      if (enableSerial)
        Serial.begin(115200);

    }
    inline bool allowSerial() {
      return enableSerial;
    }
    virtual bool available() {
      return enableSerial && Serial.available();
    }
    virtual char read() {
      return enableSerial ? Serial.read() : 0;
    }
    virtual void print(const char *p) {
      if (enableSerial)
        Serial.print(p);
    }
  protected:
    bool enableSerial;
};

class DDSoftwareSerialIO: public DDInputOutput {
  public:
    DDSoftwareSerialIO(SoftwareSerial* pSS, bool enableSerial): DDInputOutput(enableSerial) {
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
