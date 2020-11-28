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

class MbDDTester: DDTesterImpl<MbDDLayer> {
  public:
    MbDDTester(MbDDLayer *pMbLayer): DDTesterImpl(pMbLayer) {
    }   
    void testStep(int stepCount); 
};

class TurtleDDTester: DDTesterImpl<TurtleDDLayer> {
  public:
    TurtleDDTester(TurtleDDLayer *pMbLayer): DDTesterImpl(pMbLayer) {
    }    
    void testStep(int stepCount);
};

MbDDTester* CreateMbTester(DumbDisplay& dumbdisplay);
TurtleDDTester* CreateTurtleTester(DumbDisplay& dumbdisplay);

#endif