#ifndef ddtester_h
#define ddtester_h

#include "dumbdisplay.h"


void BasicDDTestLoop(DumbDisplay& dumbdisplay, bool passiveConnect = false, int builtinLEDPin = -1, std::function<void(bool connected)> notConnectedCallback = NULL);


#endif