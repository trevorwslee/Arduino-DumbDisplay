#ifndef ddtester_h
#define ddtester_h

#include "dumbdisplay.h"


// #if  __GNUC__ >= 8
// void BasicDDTestLoop(DumbDisplay& dumbdisplay, bool passiveConnect = false, int builtinLEDPin = -1, std::function<void(bool connected)> notConnectedCallback = NULL);
// #else
void BasicDDTestLoop(DumbDisplay& dumbdisplay, bool passiveConnect = false, int builtinLEDPin = -1, void (*notConnectedCallback)(bool) = NULL);
//#endif

#endif