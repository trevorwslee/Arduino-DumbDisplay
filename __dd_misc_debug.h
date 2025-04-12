#ifndef DD_NO_DEBUG_INTERFACE
class DrawTextDDDebugInterface: public DDDebugInterface {
  public:
    void setConnectionType(const char* connectionType) {
      this->connectionType = connectionType;
    }
  public:
    virtual void logConnectionState(DDDebugConnectionState connectionState) {
      const char* state = NULL;
      switch (connectionState) {
        case DDDebugConnectionState::DEBUG_NOT_CONNECTED:
          state = "NCed";
          break;
        case DDDebugConnectionState::DEBUG_CONNECTING:
          state = "Cing ";
          break;
        case DDDebugConnectionState::DEBUG_CONNECTED:
          state = "Ced  ";
          break;
        case DDDebugConnectionState::DEBUG_RECONNECTING:
          state = "RCing";
          break;
        case DDDebugConnectionState::DEBUG_RECONNECTED:
          state = "RCed ";
          break;
      }
      if (state != NULL) {
        if (true) {
          String s = state;
          if (connectionType != NULL) {
            s = String(connectionType) + ":" + s;
          }
          drawText(s.c_str(), false);
        } else {
          drawText(state, false);
        }
      }
    }
    virtual void logError(const String& errMsg) {
        drawText("Err", true);
    }
  protected:  
    virtual void drawText(const char* text, bool isError) {
    }
  protected:
    const char* connectionType;   
};
class ToSerialDDDebugInterface: public DDDebugInterface {
  public:
    virtual void logConnectionState(DDDebugConnectionState connectionState) {
      switch (connectionState) {
        case DDDebugConnectionState::DEBUG_NOT_CONNECTED:
          Serial.println("* DebugConnection: not connected");
          break;
        case DDDebugConnectionState::DEBUG_CONNECTING:
          Serial.println("* DebugConnection: connecting");
          break;
        case DDDebugConnectionState::DEBUG_CONNECTED:
          Serial.println("* DebugConnection: connected");
          break;
        case DDDebugConnectionState::DEBUG_RECONNECTING:
          Serial.println("* DebugConnection: reconnecting");
          break;
        case DDDebugConnectionState::DEBUG_RECONNECTED:
          Serial.println("* DebugConnection: reconnected");
          break;
      }
    }
    virtual void logError(const String& errMsg) {
      Serial.print("XXX Error: ");
      Serial.println(errMsg);
    }
};
class LedDDDebugInterface: public DDDebugInterface {
  public:
    LedDDDebugInterface(uint8_t ledPin) {
      this->ledPin = ledPin;
      pinMode(ledPin, OUTPUT);
    }
    virtual void logSendCommand(int state) {
      digitalWrite(ledPin, state == 1 ? HIGH : LOW);
    }
  private:
    uint8_t ledPin;
};
class CompositeDDDebugInterface: public DDDebugInterface {
  public:
    CompositeDDDebugInterface(DDDebugInterface* debug1, DDDebugInterface* debug2){
      this->debug1 = debug1;
      this->debug2 = debug2;
    }
    virtual void logConnectionState(DDDebugConnectionState connectionState) {
      if (debug1 != NULL) debug1->logConnectionState(connectionState);
      if (debug2 != NULL) debug2->logConnectionState(connectionState);
    }
    virtual void logSendCommand(int state) {
      if (debug1 != NULL) debug1->logSendCommand(state);
      if (debug2 != NULL) debug2->logSendCommand(state);
    }
    virtual void logError(const String& errMsg) {
      if (debug1 != NULL) debug1->logError(errMsg);
      if (debug2 != NULL) debug2->logError(errMsg);
    }
  private:
    DDDebugInterface* debug1;
    DDDebugInterface* debug2;    
};
#endif


