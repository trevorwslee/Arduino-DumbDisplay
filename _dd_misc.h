#ifndef _dd_misc_h
#define _dd_misc_h

const String DDEmptyString = String();

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
      // int maxDepth = sizeof(started) / sizeof(bool) - 1;
      // if (depth > maxDepth) {
      //   // not expected
      //   depth = maxDepth;
      // }
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
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& beginPaddedGroup(char dir, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
      depth += 1;
      // int maxDepth = sizeof(started) / sizeof(bool) - 1;
      // if (depth > maxDepth) {
      //   // not expected
      //   depth = maxDepth;
      // }
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
    /// add a layer, with specified padding
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& addPaddedLayer(DDLayer* layer, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(" + String(layer->getLayerId()) + ")");
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

// #if __GNUC__ >= 8 && defined(ESP32)
//   #define DD_CPP_FUNCTIONAL
// #endif
// #if defined(DD_CPP_FUNCTIONAL)
//   #include <functional>
// #endif  

#ifndef DD_NO_PASSIVE_CONNECT

/// @brief
/// Helper class to manage passive connection to DumbDisplay.
/// The only method that should be called in `void loop() {}` is DDPassiveConnectionHelper::loop(), 
/// passing to it some callbacks
/// Note that it will call DumbDisplay::masterReset() when reconnected (i.e. lost previous connection)
/// @since v0.9.9
class DDMasterResetPassiveConnectionHelper {
  public:
    /// @param saveAndPlaybackWhenInit if true, will call DumbDisplay::recordLayerCommands() / DumbDisplay::playbackLayerCommands() before and after calling `initializeCallback`
    DDMasterResetPassiveConnectionHelper(DumbDisplay& dumbdisplay, bool saveAndPlaybackWhenInit = false) : dumbdisplay(dumbdisplay), saveAndPlaybackWhenInit(saveAndPlaybackWhenInit) {
      this->initState = -2;
    }
  public:
    /// @param initializeCallback called after DumbDisplay is connected (or reconnected)
    /// @param updateCallback called to update DumbDisplay components
    /// @param disconnectedCallback called after "master reset" DumbDisplay, i.e. lost previous connection
    bool loop(void (*initializeCallback)(), void (*updateCallback)(), void (*disconnectedCallback)() = NULL) {
// #if defined(DD_CPP_FUNCTIONAL)
//     bool loop(std::function<void()> initializeCallback, std::function<void()> updateCallback, std::function<void()> disconnectedCallback = NULL) {
// #else
//     bool loop(void (*initializeCallback)(), void (*updateCallback)(), void (*disconnectedCallback)() = NULL) {
// #endif
      DDConnectPassiveStatus connectStatus;
      dumbdisplay.connectPassive(&connectStatus);
      if (connectStatus.connected) {
        if (connectStatus.reconnecting) {
          // if reconnecting (i.e. lost previous connection, "master reset" DumbDisplay)
          dumbdisplay.masterReset();
          this->initState = 0;
          if (disconnectedCallback != NULL) disconnectedCallback();
          return false;
        }
        if (this->initState <= 0) {
          if (initializeCallback != NULL) {
            if (this->saveAndPlaybackWhenInit) {
              dumbdisplay.recordLayerCommands();
            }
            initializeCallback();
            if (this->saveAndPlaybackWhenInit) {
              dumbdisplay.playbackLayerCommands();
            }
          }
          this->initState = 1;
        }
        if (updateCallback != NULL) updateCallback();
          if (this->initState == -3) {
            // just masterReset
            this->initState = 0;
          } else {
            this->initState = 2;
          }
        return true;
      } else {
        if (this->initState == -2) {
          this->initState = 0;  // so initially will go to 0 first
        } else if (this->initState == 0) {
          this->initState = -1;
        }
      }
      return false;
    }
    inline bool initialized() { return this->initState > 0; }
    inline bool firstUpdated() { return this->initState > 1; }
    inline bool isIdle() { return this->initState <= 0; }
    inline bool justBecameIdle() { return this->initState == 0; }
    /// normally, "master reset" will be called automatically when lost connection; but can be called explicitly;
    /// note that if called explicitly, will not call disconnectedCallback;
    /// IMPORTANT: should only call it in updateCallback, and after calling it, should return immediately
    void masterReset() {
      dumbdisplay.masterReset();
      this->initState = -3;
    }
  public:
    DumbDisplay& dumbdisplay;  
  private:
    bool saveAndPlaybackWhenInit;
    int8_t initState;
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
    bool loop(void (*initializeCallback)(), void (*updateCallback)()) {
// #if defined(DD_CPP_FUNCTIONAL)
//     bool loop(std::function<void()> initializeCallback, std::function<void()> updateCallback) {
// #else
//     bool loop(void (*initializeCallback)(), void (*updateCallback)()) {
// #endif
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

/// helper for using of SelectionListLayerWrapper; suggested to use SelectionListLayerWrapper whenever it makes sense
class SelectionListLayerHelper {
  public:
    SelectionListLayerHelper(SelectionListDDLayer* selectionListLayer): selectionListLayer(selectionListLayer) {
      this->selectionCount = 0;
      this->selectionOffset = 0;
      this->viewSelectionCount = -1;
      this->listStateChangedCallback = NULL;
    }
  public:
    inline SelectionListDDLayer* getLayer() {
      return selectionListLayer;
    }
    inline int getOffset() {
      return selectionOffset;
    }
    inline int getSelectionCount() {
      return selectionCount;
    }
  public:
    /// set the "list state changed" callback; here "state" refers to "list add/remove text or change of offset"
    void setListStateChangedCallback(void (*listStateChangedCallback)(), int viewSelectionCount = -1) {
      this->listStateChangedCallback = listStateChangedCallback;
      this->viewSelectionCount = viewSelectionCount;
    }
  public:
    /// @param selectionIdx -1 means append as the last selection
    /// @return -1 if selectionIdx is out of range
    int add(int selectionIdx = -1) {
      int addSelectionIdx;
      if (selectionIdx == -1) {
        addSelectionIdx = selectionCount;
      } else {
        addSelectionIdx = selectionIdx;
      } 
      if (addSelectionIdx < 0 || addSelectionIdx > selectionCount) {
        return -1;
      }
      selectionListLayer->add(addSelectionIdx);
      selectionCount += 1;
      onListStateChanged();
      return addSelectionIdx;
    }  
    /// @param selectionIdx -1 means the last selection
    /// @return -1 if selectionIdx is out of range
    int remove(int selectionIdx = -1) {
      int removeSelectionIdx;
      if (selectionIdx == -1) {
        removeSelectionIdx = selectionCount;
      } else {
        removeSelectionIdx = selectionIdx;
      } 
      if (removeSelectionIdx < 0 || removeSelectionIdx >= selectionCount) {
        return -1;
      }
      selectionListLayer->remove(removeSelectionIdx);
      selectionCount -= 1;
      if (selectionOffset > selectionCount) {
        selectionOffset = selectionCount;
      }
      onListStateChanged();
      return removeSelectionIdx;
    }
    void setOffset(int offset) {
      selectionListLayer->offset(offset);
      selectionOffset = offset;
      onListStateChanged();
    }
    inline void decrementOffset() {
      setOffset(selectionOffset - 1);
    }
    inline void incrementOffset() {
      setOffset(selectionOffset + 1);
    }
    void scrollToView(int selectionIdx) {
      if (selectionIdx < selectionOffset) {
        setOffset(selectionIdx);
      } else {
        if (this->viewSelectionCount != -1) {
          if (selectionIdx >= (selectionOffset + this->viewSelectionCount)) {
            setOffset(selectionIdx - this->viewSelectionCount + 1);
          }
        }
      }  
    }
  protected:  
    void onListStateChanged() {
      if (this->listStateChangedCallback != NULL) {
        this->listStateChangedCallback();
      }
      // if (scrollUpLayer != NULL) {
      //   bool canScrollUp = selectionOffset > 0;
      //   scrollUpLayer->disabled(!canScrollUp);
      // }
      // if (scrollDownLayer != NULL) {
      //   bool canScrollDown = selectionOffset < (selectionCount - visibleSelectionCount);
      //   scrollDownLayer->disabled(!canScrollDown);
      // }
    }  
  private:  
    SelectionListDDLayer* selectionListLayer;
    int selectionCount;
    int selectionOffset;
    int viewSelectionCount;
    void (*listStateChangedCallback)();
    //DDLayer* scrollUpLayer;
    //DDLayer* scrollDownLayer;
};


/// wrapper for using of SelectionListLayerWrapper that help you to track the text added
class SelectionListLayerWrapper {
  public:
    SelectionListLayerWrapper(short bufferSizeInc = 2) {
      this->bufferSizeInc = bufferSizeInc;
      this->helper = NULL;
      this->textBuffer = NULL;
      this->textBufferSize = 0;
      this->trackedTextCount = 0;
    }
    /// initialize a new SelectionListDDLayer (note that any old ones will not be deleted)
    SelectionListDDLayer* initializeLayer(DumbDisplay& dumbdisplay,
                                          int colCount, int rowCount,
                                          int horiSelectionCount, int vertSelectionCount,
                                          int charHeight = 0, const String& fontName = "",
                                          bool canDrawDots = true, float selectionBorderSizeCharHeightFactor = 0.3) {
      if (this->helper != NULL) {
        delete this->helper;
        this->helper = NULL;
      }
      if (this->textBuffer != NULL) {
        delete[] this->textBuffer;
        this->textBuffer = NULL;
        this->textBufferSize = 0;
        this->trackedTextCount = 0;
      }
      SelectionListDDLayer* selectionListLayer = dumbdisplay.createSelectionListLayer(colCount, rowCount, horiSelectionCount, vertSelectionCount, charHeight, fontName, canDrawDots, selectionBorderSizeCharHeightFactor);
      this->helper = new SelectionListLayerHelper(selectionListLayer);
      this->viewHoriSelectionCount = horiSelectionCount;
      this->viewSelectionCount = horiSelectionCount * vertSelectionCount;
      // setScrollLayers(NULL, NULL);
      return selectionListLayer;
    }
  public:   
    inline SelectionListDDLayer* getLayer() {
      return helper->getLayer();
    }
    inline int getOffset() {
      return helper->getOffset();
    }
    inline int getSelectionCount() {
      return helper->getSelectionCount();
    }
    int getSelectionIndexFromView(int horiSelectionIdx, int vertSelectionIdx) {
      return this->getOffset() + (vertSelectionIdx * this->viewHoriSelectionCount) + horiSelectionIdx;
    }
    const String& getSelectionText(int selectionIdx) {
      return this->textBuffer[selectionIdx];
    }
    int findSelection(const String& text) {
      for (int i = 0; i < trackedTextCount; i++) {
        if (textBuffer[i] == text) {
          return i;
        }
      }
      return -1;
    }
  public:   
    /// set the "list state changed" callback; here "state" refers to "list add/remove text or change of offset"
    void setListStateChangedCallback(void (*listStateChangedCallback)()) {
      helper->setListStateChangedCallback(listStateChangedCallback, this->viewSelectionCount);
    }
    /// @param selectionIdx -1 means append as the last selection
    /// @return -1 if selectionIdx is out of range
    int addSelection(int selectionIdx, const String& text, const String& align = "L") {
      int idx = helper->add(selectionIdx);
      if (idx != -1) {
        helper->getLayer()->text(idx, text, 0, align);
        if ((trackedTextCount + 1) > textBufferSize) {
          int newSize = textBufferSize + bufferSizeInc;
          String* newTextBuffer = new String[newSize];
          for (int i = 0; i < textBufferSize; i++) {
            newTextBuffer[i] = textBuffer[i];
          }
          if (textBuffer != NULL) {
            delete[] textBuffer;
          }
          textBuffer = newTextBuffer;
          textBufferSize = newSize;
        }
        textBuffer[trackedTextCount++] = text;
      }
      return idx;
    }
    /// @param selectionIdx -1 means the last selection
    /// @return -1 if selectionIdx is out of range
    int removeSelection(int selectionIdx) {
      selectionIdx = helper->remove(selectionIdx);
      if (selectionIdx != -1) {
        trackedTextCount -= 1;
        for (int i = selectionIdx; i < trackedTextCount; i++) {
          textBuffer[i] = textBuffer[i + 1];
        }
      }
      return selectionIdx;
    }
    void removeAllSelections() {
      int nameCount = getSelectionCount();
      for (int i = 0; i < nameCount; i++) {
        removeSelection(0);
      }
    }
    void setOffset(int offset) {
      helper->setOffset(offset);
    }
    inline void incrementOffset() {
      helper->incrementOffset();
    }
    inline void decrementOffset() {
      helper->decrementOffset();
    }
    void scrollToView(int selectionIdx) {
      helper->scrollToView(selectionIdx);
    }
    void select(int selectionIdx, bool deselectTheOthers = true) {
      helper->getLayer()->select(selectionIdx, deselectTheOthers);
    }
    void deselect(int selectionIdx, bool selectTheOthers = false) {
      helper->getLayer()->deselect(selectionIdx, selectTheOthers);
    }
    void selected(int selectionIdx, bool selected, bool reverseTheOthers = false) {
      helper->getLayer()->selected(selectionIdx, selected, reverseTheOthers);
    }
    void selectAll() {
      helper->getLayer()->selectAll();
    }
    void deselectAll() {
      helper->getLayer()->deselectAll();
    }
  private: 
    SelectionListLayerHelper* helper; 
    String* textBuffer;
    short bufferSizeInc;
    int textBufferSize;
    int trackedTextCount;
    int viewHoriSelectionCount;
    int viewSelectionCount;
};

#endif

