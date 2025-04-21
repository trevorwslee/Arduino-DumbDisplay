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

