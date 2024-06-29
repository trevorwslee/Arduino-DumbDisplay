#ifndef _dd_misc_h
#define _dd_misc_h

#ifndef DD_NO_DEBUG_INTERFACE
class DrawTextDDDebugInterface: public DDDebugInterface {
  // public:
  //   OledDDDebugInterface(Adafruit_SSD1306& display, int x = 0, int y = 0/*, uint8_t fontSize = 2, uint8_t font = 1, */, bool indicateSendCommand = false): display(display) {
  //     this->x = x;
  //     this->y = y;
  //     //this->fontSize = fontSize;
  //     //this->font = font;
  //     this->indicateSendCommand = indicateSendCommand;
  //   }
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
        drawText(state, false);
      }
    }
    virtual void logError(const String& errMsg) {
        drawText("Err", true);
    }
  protected:  
    virtual void drawText(const char* text, bool isError) {
    }
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
class CompositDDDebugIntreface: public DDDebugInterface {
  public:
    CompositDDDebugIntreface(DDDebugInterface* debug1, DDDebugInterface* debug2){
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

/// check if dumbdisplay is connected
bool DDConnected();
/// the same usage as standard delay(), but it gives DD a chance to handle "feedbacks"
void DDDelay(unsigned long ms);
/// give DD a chance to handle "feedbacks"
void DDYield();


/// check whether system is LITTLE_ENDIAN or BIG_ENDIAN
/// @return 0 if LITTLE_ENDIAN; 1 if BIG_ENDIAN
inline int DDCheckEndian() {
    int i = 1;
    const char* p = (const char*) &i;
    if (p[0] == 1)
        return 0;  // LITTLE_ENDIAN;
    else
        return 1;  // BIG_ENDIAN;
}


/// Helper class for keeping value
template <class T> class DDValueStore {
  public:
    DDValueStore(T value): storedValue(value) {}
    /// @return true only if some value different set
    bool set(T value) {
      bool same = storedValue == value;
      storedValue = value;
      return !same;
    }
    inline T get() { return storedValue; }
  private:
    T storedValue;
};


/// Helper class for keeping changed value, until recorded (i.e. record() is called) to make the value known
template <class T> class DDValueRecord {
  public:
    /// initial value and known value will be set to the same
    DDValueRecord(T value): value(value), knownValue(value) {}
    /// can set initial known value to be different from value
    DDValueRecord(T value, T knownValue): value(value), knownValue(knownValue) {}
    inline operator T() { return value; } 
    inline T get() { return value; }
    inline void operator =(T value) { this->value = value; }
    inline void set(T value) { this->value = value; }
    inline T getRecorded() { return knownValue; }
    /// make value set previously known; i.e. set known value to be the same as value
    /// @return whether known value as different from value
    bool record() {
      bool same = value == knownValue;
      knownValue = value;
      return !same;
    }
  private:
    T value;
    T knownValue;
};

/// Helper class for keeping changed value until acknowledged (i.e. acknowledge() is called)
template <class T> class DDPendingValue {
  public:
    DDPendingValue() {
      this->valueIsPending = false;
    }
    DDPendingValue(T value): value(value) {
      this->valueIsPending = true;
    }
    inline operator T() { return value; } 
    inline T get() { return value; }
    inline void operator =(T value) { this->value = value; this->valueIsPending = true; }
    inline void set(T value) { this->value = value; this->valueIsPending = true; }
    /// acknowledge pending changed value
    /// @return whether there is pending value; if so, acknowledge and make the value not pending
    bool acknowledge() {
      if (valueIsPending) {
        valueIsPending = false;
        return true;
      } else {
        return false;
      }
    }
  private:
    T value;
    bool valueIsPending;
};


// /// ***Deprecated! Please use DDAutoPinConfig instead!***
// /// @deprecated
// template<int MAX_DEPTH> class DDAutoPinConfigBuilder {  // MAX_DEPTH: depth of [nested] group
//   public:
//     // dir: 'H' / 'V' / 'S'
//     DDAutoPinConfigBuilder(char dir) {
//       config = String(dir) + "(";
//       depth = 0;
//       started[depth] = false;
//     }
//   public:
//     // dir: 'H' / 'V' / 'S'
//     DDAutoPinConfigBuilder& beginGroup(char dir) {
//       addConfig(String(dir) + "(");
//       depth += 1;
//       started[depth] = false;
//       return *this;
//     }  
//     // DDAutoPinConfigBuilder& beginPaddedGroup(int left, int top, int right, int bottom) {
//     //   addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
//     //   depth += 1;
//     //   started[depth] = false;
//     //   return *this;
//     // }  
//     DDAutoPinConfigBuilder& endGroup() {
//       config.concat(')');
//       depth -= 1;
//       return *this;
//     }
//     DDAutoPinConfigBuilder& addLayer(DDLayer* layer) {
//       addConfig(layer->getLayerId());
//       return *this;
//     }
//     DDAutoPinConfigBuilder& beginPaddedGroup(int left, int top, int right, int bottom) {
//       addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
//       depth += 1;
//       started[depth] = false;
//       return *this;
//     }  
//     DDAutoPinConfigBuilder& endPaddedGroup() {
//       return endGroup();
//     }
//     // // dir: 'H' / 'V' / 'S'
//     // DDAutoPinConfigBuilder& addRemainingGroup(char dir) {
//     //   addConfig(String(dir) + "(*)");
//     //   return *this;
//     // }
//     const String& build() {
//       if (config.length() == 2) {
//         // just started
//         config.concat('*');
//       }
//       config.concat(')');
//       return config;
//     }  
//   private:  
//     void addConfig(const String& conf) {
//       if (started[depth]) {
//         config.concat('+');
//       } else {
//         started[depth] = true;
//       }
//       config.concat(conf);
//     }
//   private:
//     int depth;
//     bool started[MAX_DEPTH + 1];
//     String config;
// };


/// @brief
/// Class for building "auto pin" config, to be passed to DumbDisplay::configAutoPin().
/// @since v0.9.7-r2
class DDAutoPinConfig {
  public:
    /// @param dir directory of layers at the top level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    /// @param nestedDepth maximum depth of nesting; default is 5
    DDAutoPinConfig(char dir, int nestedDepth = 5) {
      
      config = String(dir) + "(";
      depth = 0;
      started = new bool[nestedDepth + 1];
      started[depth] = false;
    }
    ~DDAutoPinConfig() {
      delete started;
    }
  public:
    /// begin a layer group, creating a new level of nesting
    /// @param dir directory of layers at the new level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    DDAutoPinConfig& beginGroup(char dir) {
      addConfig(String(dir) + "(");
      depth += 1;
      int maxDepth = sizeof(started) / sizeof(bool) - 1;
      if (depth > maxDepth) {
        // not expected
        depth = maxDepth;
      }
      started[depth] = false;
      return *this;
    }  
    /// end a begun group, returning to the previous level of nesting
    DDAutoPinConfig& endGroup() {
      config.concat(')');
      depth -= 1;
      if (depth < 0) {
        // not expected
        depth = 0;
      }
      return *this;
    }
    /// begin a layer group, with specified padding
    /// @param dir directory of layers at the new level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    /// @param left left padding
    /// @param top top padding  
    /// @param right right padding
    /// @param bottom bottom padding
    DDAutoPinConfig& beginPaddedGroup(char dir, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
      depth += 1;
      int maxDepth = sizeof(started) / sizeof(bool) - 1;
      if (depth > maxDepth) {
        // not expected
        depth = maxDepth;
      }
      started[depth] = false;
      return beginGroup(dir);
    }  
    /// end begun padded group
    DDAutoPinConfig& endPaddedGroup() {
      endGroup();
      config.concat(')');
      depth -= 1;
      return *this;
    }
    /// add a layer to the current level
    DDAutoPinConfig& addLayer(DDLayer* layer) {
      if (layer != NULL) {
        addConfig(layer->getLayerId());
      }
      return *this;
    }
    /// add spacer, which is a placeholder layer with the specified size
    /// @param width width of the placeholder layer
    /// @param height height of the placeholder layer
    DDAutoPinConfig& addSpacer(int width, int height) {
      addConfig(String("<") + String(width) + "x" + String(height) + String(">"));
      return *this;
    }
    /// add the layout direction for the layers not included
    /// @param dir 'H' / 'V' / 'S
    DDAutoPinConfig& addRemainingGroup(char dir) {
      addConfig(String(dir) + "(*)");
      return *this;
    }
    /// build the "auto pin" config string, to be passed to DumbDisplay::configAutoPin()
    const String& build() {
      if (config.length() == 2) {
        // just started
        config.concat('*');
      }
      config.concat(')');
      return config;
    }  
  private:  
    void addConfig(const String& conf) {
      if (started[depth]) {
        config.concat('+');
      } else {
        started[depth] = true;
      }
      config.concat(conf);
    }
  private:
    int depth;
    bool *started;
    String config;
};



/// @brief
/// Helper class for tracking connection "version" change.
/// - initial connection will have version 1
/// - when reconnected (after disconnect) version will be incremented every such event
class DDConnectVersionTracker {
  public:
    /// @param version start-off version number; 
    ///                0 so that it will be considered a version change even when first start, 
    ///                1 if only want to detect re-connect
    DDConnectVersionTracker(int version = 0) {
      this->version = version;
    }
    bool checkChanged(DumbDisplay& dumbdisplay) {
      int oldVersion = this->version;
      this->version = dumbdisplay.getConnectVersion();
      return this->version != oldVersion;
    }
  private:
    int version;  
};



/// @brief
/// Helper class for managing layer layout plus update of the layers.
/// @since v0.9.7-r2
class DDLayoutHelper {
  public: 
    DDLayoutHelper(DumbDisplay& dumbdisplay): dumbdisplay(dumbdisplay), versionTracker(-1) {}
  public:
    /// check whether layers need be updated, say
    /// - just initialzed
    /// - DD reconnected
    bool checkNeedToUpdateLayers(/*DumbDisplay& dumbdisplay*/) {
      DDYield();
      return versionTracker.checkChanged(dumbdisplay);
    }
    /// essentially DumbDisplay::recordLayerSetupCommands()
    /// *MUST* call finishInitializeLayout() layout when done
    inline void startInitializeLayout(/*DumbDisplay& dumbdisplay*/) {
      dumbdisplay.recordLayerSetupCommands();
    }
    /// after calling startInitializeLayout(), call this to finish the layout of layers
    /// @param layerSetupPersistId is use for calling DumbDisplay.playbackLayerSetupCommands()
    inline void finishInitializeLayout(/*DumbDisplay& dumbdisplay, */String layerSetupPersistId) {
      dumbdisplay.playbackLayerSetupCommands(layerSetupPersistId);
    }
    /// basically DumbDisplay::configAutoPin()
    inline void configAutoPin(const String& layoutSpec) {
      dumbdisplay.configAutoPin(layoutSpec);
    }
    /// basically DumbDisplay::addRemainingAutoPinConfig()
    inline void addRemainingAutoPinConfig(const String& remainingLayoutSpec) {
      dumbdisplay.addRemainingAutoPinConfig(remainingLayoutSpec);
    }
    /// basically DumbDisplay::configPinFrame()
    inline void configPinFrame(int xUnitCount = 100, int yUnitCount = 100) {
      dumbdisplay.configPinFrame(xUnitCount, yUnitCount);
    }
    /// basically DumbDisplay::pinLayer()
    inline void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinLayer(pLayer, uLeft, uTop, uWidth, uHeight, align);
    }
    /// basically DumbDisplay::pinAutoPinLayers()
    inline void pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinAutoPinLayers(layoutSpec, uLeft, uTop, uWidth, uHeight, align);
    }
#ifndef DD_NO_IDLE_CALLBACK
    /// basically DumbDisplay::setIdleCallback()
    inline void setIdleCallback(DDIdleCallback idleCallback) {
      dumbdisplay.setIdleCallback(idleCallback);
    }
#endif
    // // deprecated
    // inline void setIdleCalback(DDIdleCallback idleCallback) {
    //   dumbdisplay.setIdleCalback(idleCallback);
    // }
  private:
    DumbDisplay& dumbdisplay;
    DDConnectVersionTracker versionTracker;
};


/// Utility class 
template<int MAX_LAYER_COUNT>
class DDFadingLayers {
  public:
    DDFadingLayers() {
      this->layerCount = 0;
      this->nextUseLayerIdx = 0;
    }
    void initAddLayer(DDLayer* layer) {
      if (layerCount < MAX_LAYER_COUNT) {
        layers[layerCount++] = layer;
      }
    }
    void clear() {
      for (int i = 0; i < layerCount; i++) {
        layers[i]->clear();
      }
      nextUseLayerIdx = 0;
    }
  public:  
    DDLayer* useLayer() {
      int layerIdx = nextUseLayerIdx;
      nextUseLayerIdx = (nextUseLayerIdx + 1) % layerCount;
      for (int i = 0; i < this->layerCount; i++) {
        int lidx = (layerIdx + i) % layerCount;
        DDLayer* layer = layers[lidx];
        if (i == 0) {
          layer->opacity(100);
          layer->clear();
        } else {
          int opacity = i * (100.0 / layerCount);
          layer->opacity(opacity);
        }
      }
      return layers[layerIdx];
    }  
  private:
    DDLayer* layers[MAX_LAYER_COUNT];
    int layerCount;
    int nextUseLayerIdx;
};


#if __GNUC__ >= 8
  #include <functional>
#endif  

/// @brief
/// Helper class to manage passive connection to DumbDisplay.
/// The only method that should be called in `void loop() {}` is DDPassiveConnectionHelper::loop(), 
/// passing to it some callbacks
/// Note that it will call DumbDisplay::masterReset() when reconnected (i.e. lost previous connection)
/// @since v0.9.9
class DDMasterResetPassiveConnectionHelper {
  public:
    DDMasterResetPassiveConnectionHelper(DumbDisplay& dumbdisplay) : dumbdisplay(dumbdisplay) {
      this->init_state = -2;
    }
  public:
    /// @param initializeCallback called after DumbDisplay is connected (or reconnected)
    /// @param updateCallback called to update DumbDisplay components
    /// @param disconnectedCallback called after "master reset" DumbDisplay, i.e. lost previous connection
#if defined(DD_PASSIVE_CONNECTION_HELPER) || __GNUC__ >= 8
    bool loop(std::function<void()> initializeCallback, std::function<void()> updateCallback, std::function<void()> disconnectedCallback = NULL) {
#else
    bool loop(void (*initializeCallback)(), void (*updateCallback)(), void (*disconnectedCallback)() = NULL) {
#endif
      DDConnectPassiveStatus connectStatus;
      dumbdisplay.connectPassive(&connectStatus);
      if (connectStatus.connected) {
        if (connectStatus.reconnecting) {
          // if reconnecting (i.e. lost previous connection, "master reset" DumbDisplay)
          dumbdisplay.masterReset();
          this->init_state = 0;
          if (disconnectedCallback != NULL) disconnectedCallback();
          return false;
        }
        if (this->init_state <= 0) {
          if (initializeCallback != NULL) initializeCallback();
          this->init_state = 1;
        }
        if (updateCallback != NULL) updateCallback();
        this->init_state = 2;
        return true;
      } else {
        if (this->init_state == -2) {
          this->init_state = 0;  // so initially will go to 0 first
        } else if (this->init_state == 0) {
          this->init_state = -1;
        }
      }
      return false;
    }
    inline bool initialized() { return this->init_state > 0; }
    inline bool firstUpdated() { return this->init_state > 1; }
    inline bool isIdle() { return this->init_state <= 0; }
    inline bool justBecameIdle() { return this->init_state == 0; }
  public:
    DumbDisplay& dumbdisplay;  
  private:
    int8_t init_state;
};

/// @brief
/// Helper class to manage passive connection to DumbDisplay.
/// The only method that should be called in `void loop() {}` is DDPassiveConnectionHelper::loop(), 
/// passing to it some callbacks
/// Note that it will surround call to `initializeCallback` with calls to DumbDisplay::recordLayerSetupCommands() and DumbDisplay::playbackLayerSetupCommands() 
/// @since v0.9.9
class DDReconnectPassiveConnectionHelper {
  public:
    /// @param layerSetupPersistId used when calling DumbDisplay::playbackLayerSetupCommands()
    DDReconnectPassiveConnectionHelper(DumbDisplay& dumbdisplay, const String& layerSetupPersistId) : dumbdisplay(dumbdisplay) {
      this->layerSetupPersistId = layerSetupPersistId;
      this->init = false;
    }
  public:
    /// @param initializeCallback called after DumbDisplay is connected (or reconnected)
    /// @param updateCallback called to update DumbDisplay components
    /// @param disconnectedCallback called after "master reset" DumbDisplay, i.e. lost previous connection
#if defined(DD_PASSIVE_CONNECTION_HELPER) || __GNUC__ >= 8
    bool loop(std::function<void()> initializeCallback, std::function<void()> updateCallback) {
#else
    bool loop(void (*initializeCallback)(), void (*updateCallback)()) {
#endif
      if (dumbdisplay.connectPassive()) {
        if (!this->init) {
          this->dumbdisplay.recordLayerSetupCommands();
          if (initializeCallback != NULL) initializeCallback();
          this->dumbdisplay.playbackLayerSetupCommands(this->layerSetupPersistId);
          this->init = true;
        }
        if (updateCallback != NULL) updateCallback();
        return true;
      }
      return false;
    }
    inline bool initialized() { return this->init; }
  public:
    DumbDisplay& dumbdisplay;  
  private:
    String layerSetupPersistId;
    bool init;  
};

#endif