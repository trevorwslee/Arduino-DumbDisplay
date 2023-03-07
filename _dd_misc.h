#ifndef _dd_misc_h
#define _dd_misc_h


//#define LITTLE_ENDIAN 0
//#define BIG_ENDIAN    1

// return
// . 0 -- LITTLE_ENDIAN
// . 1 -- BIG_ENDIAN
inline int DDCheckEndian() {
    int i = 1;
    const char* p = (const char*) &i;
    if (p[0] == 1)
        return 0;//LITTLE_ENDIAN;
    else
        return 1;//BIG_ENDIAN;
}


template <class T>
class DDValueStore {
  public:
    DDValueStore(T value): storedValue(value) {}
    /* return true only if some value different set */
    bool set(T value) {
      bool same = storedValue == value;
      storedValue = value;
      return !same;
    }
    inline T get() { return storedValue; }
  private:
    T storedValue;
};


template <class T>
class DDValueRecord {
  public:
    /* initial value and known value will be set to the same */
    DDValueRecord(T value): value(value), knownValue(value) {}
    /* can set initial known value to be different from value */
    DDValueRecord(T value, T knownValue): value(value), knownValue(knownValue) {}
    inline operator T() { return value; } 
    inline T get() { return value; }
    inline void operator =(T value) { this->value = value; }
    inline void set(T value) { this->value = value; }
    inline T getRecorded() { return knownValue; }
    /* make value set previously known; i.e. set known value to be the same as value */
    /* return whether known value as different from value */ 
    bool record() {
      bool same = value == knownValue;
      knownValue = value;
      return !same;
    }
  private:
    T value;
    T knownValue;
};

template <class T>
class DDPendingValue {
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
    /* return whether there is pending value; if so, acknowledge and make the value not pending */
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


/* *** DEPRECATED ... please use DDAutoPinConfig *** */
template<int MAX_DEPTH> // MAX_DEPTH: depth of [nested] group
class DDAutoPinConfigBuilder {
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
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfigBuilder& addRemainingGroup(char dir) {
      addConfig(String(dir) + "(*)");
      return *this;
    }
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


class DDAutoPinConfig {
  public:
    // dir: 'H' / 'V' / 'S'
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
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfig& beginGroup(char dir) {
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
    DDAutoPinConfig& endGroup() {
      config.concat(')');
      depth -= 1;
      return *this;
    }
    DDAutoPinConfig& addLayer(DDLayer* layer) {
      addConfig(layer->getLayerId());
      return *this;
    }
    DDAutoPinConfig& beginPaddedGroup(int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
      depth += 1;
      started[depth] = false;
      return *this;
    }  
    DDAutoPinConfig& endPaddedGroup() {
      return endGroup();
    }
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfig& addRemainingGroup(char dir) {
      addConfig(String(dir) + "(*)");
      return *this;
    }
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



class DDConnectVersionTracker {
  public:
    /* . version: pass in -1 so that it will be considered a version change even when fresh start */
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


/* the same usage as standard delay(), but it allows DD chances to handle feedback */
void DDDelay(unsigned long ms);
/* give DD a chance to handle feedback */
void DDYield();


class DDLayoutHelper {
  public: 
    DDLayoutHelper(DumbDisplay& dumbdisplay): dumbdisplay(dumbdisplay), versionTracker(-1) {}
    //   this->layoutDoneBefore = false;
    // }
    ~DDLayoutHelper() {
      if (this->autoPinConfig != NULL) {
        delete this->autoPinConfig;
      }
    }
  public:
    // /* check whether layout done before; if not, please call startInitializeLayout() to begin layout */
    // bool checkNeedToInitializeLayout() {
    //   DDYield();
    //   return !layoutDoneBefore;
    // }  
    /* check whether layers need be updated, say, 1) just initialzed; or 2) DD reconnected */
    bool checkNeedToUpdateLayers(/*DumbDisplay& dumbdisplay*/) {
      DDYield();
      return versionTracker.checkChanged(dumbdisplay);
    }
    /* essentially dumbdisplay.recordLayerSetupCommands() */
    /* MUST call finishInitializeLayout() when done */
    void startInitializeLayout(/*DumbDisplay& dumbdisplay*/) {
      //this->layoutDoneBefore = true;
      dumbdisplay.recordLayerSetupCommands();
    }
    /* layerSetupPersistId is use for calling dumbdisplay.playbackLayerSetupCommands() */
    void finishInitializeLayout(/*DumbDisplay& dumbdisplay, */String layerSetupPersistId) {
      dumbdisplay.playbackLayerSetupCommands(layerSetupPersistId);
    }
    // dir: 'H' / 'V' / 'S'
    DDAutoPinConfig& newAutoPinConfig(char dir, int nestedDepth = 3) {
      if (autoPinConfig == NULL) {
        autoPinConfig = new DDAutoPinConfig(dir, nestedDepth); 
      }
      return *autoPinConfig;
    }
    void configAutoPin(/*DumbDisplay dumbdisplay*/) {
      if (autoPinConfig != NULL) {
        dumbdisplay.configAutoPin(autoPinConfig->build());
        delete autoPinConfig;
        autoPinConfig = NULL;
      }
    }
    void setIdleCalback(DDIdleCallback idleCallback) {
      dumbdisplay.setIdleCalback(idleCallback);
    }
  private:
    //bool layoutDoneBefore;
    DumbDisplay& dumbdisplay;
    DDConnectVersionTracker versionTracker;
    DDAutoPinConfig* autoPinConfig;
};


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