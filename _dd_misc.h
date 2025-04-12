#ifndef _dd_misc_h
#define _dd_misc_h

const String DDEmptyString = String();

/// check if dumbdisplay is connected
bool DDConnected();
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

#include "__dd_misc_debug.h"
#include "__dd_misc_value.h"
#include "__dd_misc_layout.h"
#include "__dd_misc_layer.h"
#include "__dd_misc_connect.h"


#endif

