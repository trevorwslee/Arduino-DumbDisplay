#ifndef _dd_io_h
#define _dd_io_h


class DDInputOutput {
  public:
    DDInputOutput(unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, false, true) {
    }
    DDInputOutput* newForSerialConnection() {
      return new DDInputOutput(serialBaud, false, true);
    }
    virtual ~DDInputOutput() {
    }
    virtual bool available() {
      //return Serial.available();
      return _The_DD_Serial != NULL && _The_DD_Serial->available(); 
    }
    virtual char read() {
      //return Serial.read();
      return _The_DD_Serial != NULL ? _The_DD_Serial->read() : 0;
    }
    virtual void print(const String &s) {
      //Serial.print(s);
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(s);
    }
    virtual void print(const char *p) {
      //Serial.print(p);
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(p);
    }
    virtual void flush() {
      //Serial.flush();
      if (_The_DD_Serial != NULL) _The_DD_Serial->flush();
    }
    virtual void keepAlive() {
    }
    virtual void validConnection() {
    }
    virtual void preConnect() {
      if (setupForSerial) {
        if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
        //Serial.begin(serialBaud);
      }
    }
  public:  
    bool isSerial() {
      return !backupBySerial && setupForSerial;
    }
    bool isBackupBySerial() {
      return backupBySerial;
    }
  protected:
    DDInputOutput(unsigned long serialBaud, bool backupBySerial, bool setupForSerial) {
      this->serialBaud = serialBaud;
      this->backupBySerial = backupBySerial;
      this->setupForSerial = setupForSerial;
    }
  protected:
    unsigned long serialBaud;
    bool backupBySerial;
    bool setupForSerial;
};

#endif