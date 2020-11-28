#ifndef ddtester_h
#define ddtester_h

#include "Arduino.h"
#include "dumbdisplay.h"

class DDTester {
  public:
    virtual void testStep(int stepCount) {
    }
};

template <typename T> class DDTesterImpl: public DDTester {
  protected:
    DDTesterImpl(T *pLayer): DDTester() {
      this->pLayer = pLayer;
    }
  protected:
    T *pLayer;  
};

class MbTester: DDTesterImpl<MicroBitLayer> {
  public:
    MbTester(MicroBitLayer *pMbLayer): DDTesterImpl(pMbLayer) {
    }   
    void testStep(int stepCount); 
};

class TurtleTester: DDTesterImpl<TurtleLayer> {
  public:
    TurtleTester(TurtleLayer *pMbLayer): DDTesterImpl(pMbLayer) {
    }    
    void testStep(int stepCount);
};

MbTester* CreateMbTester(DumbDisplay& dumbdisplay);
TurtleTester* CreateTurtleTester(DumbDisplay& dumbdisplay);

#endif