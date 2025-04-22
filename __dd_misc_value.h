

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

/// newState: the next state when it is due time for next state;
/// stateChangeInMillis: the time delay (in millis) for the new state to be effective (state changed)
struct DDChangeStateInfo {
  int newState;
  long stateChangeInMillis;
};

/// helper class for timing for changing stage;
/// call initialize() to set the initial state and next "stage change" delay (in millis);
/// call checkStateChange() to check if the new state became effective (changed), if so, stageChangedCallback is called to get back next "stage change" info
class DDTimedChangeStateHelper {
  public:
    void initialize(int initialState = -1, long stateChangeInMillis = 0) {
      this->currentState = initialState;
      this->nextStateChangeMillis = millis() + stateChangeInMillis;
    }
    void checkStatChange(DDChangeStateInfo (*stageChangedCallback)(int currentState)) {
      long now = millis();
      if (now >= nextStateChangeMillis) {
        DDChangeStateInfo info = stageChangedCallback(currentState);
        currentState = info.newState;
        nextStateChangeMillis = now + info.stateChangeInMillis;
      }
    }
  private:
    int currentState;   
    long nextStateChangeMillis;
};





