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

#endif