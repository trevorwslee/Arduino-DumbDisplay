#ifndef _dd_io_h
#define _dd_io_h

/// @brief
/// Class for DD input/output; you explicitly constructed it, pass in when instantiate DumbDisplay,
/// then it will be use by DD internally for communication with DumbDisplay app.
/// There are several derived class like DDSoftwareSerialIO, DDWiFiServerIO, DDBluetoothSerialIO, DDBLESerialIO, DDPicoUart1IO etc
/// @since v0.9.8
class DDInputOutput {
  public:
    DDInputOutput(unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, false, true) {
    }
    DDInputOutput* newForSerialConnection() {
      return new DDInputOutput(serialBaud, false, true);
    }
    virtual ~DDInputOutput() {
    }
    virtual const char* getWhat() {
      return NULL;
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
    virtual void write(uint8_t b) {
      if (_The_DD_Serial != NULL) _The_DD_Serial->write(b);
    }
    virtual void write(const uint8_t *buf, size_t size) {
      for (size_t i = 0; i < size; i++) {
        write(buf[i]);
      }
    }    
    virtual void flush() {
      //Serial.flush();
      if (_The_DD_Serial != NULL) _The_DD_Serial->flush();
    }
    virtual void keepAlive() {
    }
    virtual void validConnection() {
    }
    virtual bool preConnect(bool firstCall) {
      if (true) {
        if (firstCall) {
          // since 2023-08-13
          if (setupForSerial) {
            if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
            //Serial.begin(serialBaud);
          }
        }
      }/* else {  since 2023-09-02, do not begin if not firstCall
        if (setupForSerial) {
          if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
          //Serial.begin(serialBaud);
        }
      }*/
      return true;
    }
    virtual bool canConnectPassive() {
      return true;
    }
    virtual bool canUseBuffer() {
      return false;
    }
  public:  
    inline bool isSerial() {
      return !backupBySerial && setupForSerial;
    }
    inline bool isForSerial() {
      return setupForSerial;  // since 2023-06-03
    }
    inline bool isBackupBySerial() {
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