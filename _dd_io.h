#ifndef _dd_io_h
#define _dd_io_h

/// @brief
/// Class for DD input/output; you explicitly constructed it, pass in when instantiate DumbDisplay,
/// then it will be use by DD internally for communication with DumbDisplay app.
/// There are several derived class like DDSoftwareSerialIO, DDWiFiServerIO, DDBluetoothSerialIO, DDBLESerialIO, DDPicoSerialIO etc
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
#ifdef DDIO_USE_DD_SERIAL      
      return _The_DD_Serial != NULL && _The_DD_Serial->available(); 
#else
      return Serial.available();
#endif
    }
    virtual char read() {
#ifdef DDIO_USE_DD_SERIAL      
      return _The_DD_Serial != NULL ? _The_DD_Serial->read() : 0;
#else
      return Serial.read();
#endif
    }
    virtual void print(const String &s) {
#ifdef DDIO_USE_DD_SERIAL      
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(s);
#else
      Serial.print(s);
#endif
    }
    virtual void print(const char *p) {
#ifdef DDIO_USE_DD_SERIAL      
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(p);
#else
      Serial.print(p);
#endif      
    }
    virtual void write(uint8_t b) {
#ifdef DDIO_USE_DD_SERIAL      
      if (_The_DD_Serial != NULL) _The_DD_Serial->write(b);
#else
      Serial.write(b);
#endif
    }
    virtual void write(const uint8_t *buf, size_t size) {
      for (size_t i = 0; i < size; i++) {
        write(buf[i]);
      }
    }    
    virtual void flush() {
#ifdef DDIO_USE_DD_SERIAL      
      if (_The_DD_Serial != NULL) _The_DD_Serial->flush();
#else
      Serial.flush();
#endif      
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
#ifdef DDIO_USE_DD_SERIAL      
            if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
#else
            Serial.begin(serialBaud);
#endif            
          }
        }
      }
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
    inline bool willUseSerial() {
      return setupForSerial || backupBySerial;
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