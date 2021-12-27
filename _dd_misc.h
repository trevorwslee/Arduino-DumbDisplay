#ifndef _dd_misc_h
#define _dd_misc_h


template <class T>
class DDValueStore {
  public:
    DDValueStore(T value): storedValue(value) {}
    bool set(T value) {
      bool same = storedValue == value;
      storedValue = value;
      return !same;
    }
    T get() { return storedValue; }
  private:
    T storedValue;
};


class DDConnectVersionTracker {
  public:
    DDConnectVersionTracker(int version) {
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

#endif