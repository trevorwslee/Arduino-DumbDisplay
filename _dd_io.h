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
      if (setupForSerial) {
        if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
        //Serial.begin(serialBaud);
      }
      return true;
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

class DDWriteOnyIO: public DDInputOutput {
  public:
    DDWriteOnyIO(DDInputOutput* io, uint16_t bufferSize = 8/*256*/): io(io) {
      this->bufferSize = bufferSize;
      this->buffer = new uint8_t[bufferSize];
      this->bufferedCount = 0;
    }
    ~DDWriteOnyIO() {
      delete this->buffer;
    }
    void print(const String &s) {
      print(s.c_str());
    }
    void print(const char *p) {
      int len = strlen(p);
      write((uint8_t*) p, len);
      // const char *c = p;
      // while (true) {
      //   if (*c == 0) {
      //     break;
      //   }
      //   c++;
      // }
      // int count = c - p;
      // write((uint8_t*) p, count);
    }
    void write(uint8_t b) {
      write(&b, 1);
    }
    void write(const uint8_t *buf, size_t size) {
      if (false) {
        io->write(buf, size);
      } else {
        if ((bufferedCount + size) > bufferSize) {
          flush();
        }
        if (size > bufferSize) {
          flush();
          io->write(buf, size);
        } else {
          const uint8_t *s = buf;
          uint8_t *t = buffer + bufferedCount;
          bool flushAfterward = false;
          for (int i = 0; i < size; i++) {
            if (*s == '\n') {
              flushAfterward = true;
            }
            *t = *s;
            s++;
            t++; 
            bufferedCount++;
          }
          //memcpy(buffer + bufferedCount, buf, size);
          //bufferedCount += size;
          if (flushAfterward) {
            flush();
          }
        }
      }
    }    
    void flush() {
      //Serial.println("--->");
      if (bufferedCount > 0) {
        io->write(buffer, bufferedCount);
        bufferedCount = 0;
      }
    }
  private:
    DDInputOutput* io;
    uint8_t bufferSize;
    uint8_t* buffer;  
    uint8_t bufferedCount;
};

#endif