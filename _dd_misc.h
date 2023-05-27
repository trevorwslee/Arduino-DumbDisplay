#ifndef _dd_misc_h
#define _dd_misc_h



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


/// ***Deprecated! Please use DDAutoPinConfig instead!***
/// @deprecated
template<int MAX_DEPTH> class DDAutoPinConfigBuilder {  // MAX_DEPTH: depth of [nested] group
  public:
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfigBuilder(char dir) {
      config = String(dir) + "(";
      depth = 0;
      started[depth] = false;
    }
  public:
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfigBuilder& beginGroup(char dir) {
      addConfig(String(dir) + "(");
      depth += 1;
      started[depth] = false;
      return *this;
    }  
    // DDAutoPinConfigBuilder& beginPaddedGroup(int left, int top, int right, int bottom) {
    //   addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
    //   depth += 1;
    //   started[depth] = false;
    //   return *this;
    // }  
    DDAutoPinConfigBuilder& endGroup() {
      config.concat(')');
      depth -= 1;
      return *this;
    }
    DDAutoPinConfigBuilder& addLayer(DDLayer* layer) {
      addConfig(layer->getLayerId());
      return *this;
    }
    DDAutoPinConfigBuilder& beginPaddedGroup(int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
      depth += 1;
      started[depth] = false;
      return *this;
    }  
    DDAutoPinConfigBuilder& endPaddedGroup() {
      return endGroup();
    }
    // // dir: 'H' / 'V' / 'S'
    // DDAutoPinConfigBuilder& addRemainingGroup(char dir) {
    //   addConfig(String(dir) + "(*)");
    //   return *this;
    // }
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
    bool started[MAX_DEPTH + 1];
    String config;
};


/// @brief
/// Class for building "auto pin" config, to be passed to DumbDisplay::configAutoPin().
/// @since v0.9.7-r2
class DDAutoPinConfig {
  public:
    /// @param dir directory of layers at the top level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    DDAutoPinConfig(char dir, int nestedDepth = 3) {
      
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
      started[depth] = false;
      return *this;
    }  
    /// end a begun group, returning to the previous level of nesting
    DDAutoPinConfig& endGroup() {
      config.concat(')');
      depth -= 1;
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



/// Helper class for tracking connection "version" change, say, when reconnected afte disconnect
class DDConnectVersionTracker {
  public:
    /// @param version version number to start with; -1 so that it will be considered a version change even when fresh start
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
    /// basically DumbDisplay::configPinFrame()
    inline void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinLayer(pLayer, uLeft, uTop, uWidth, uHeight, align);
    }
    /// basically DumbDisplay::configPinFrame()
    inline void pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinAutoPinLayers(layoutSpec, uLeft, uTop, uWidth, uHeight, align);
    }
    /// basically DumbDisplay::configPinFrame()
    inline void setIdleCallback(DDIdleCallback idleCallback) {
      dumbdisplay.setIdleCallback(idleCallback);
    }
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




#endif