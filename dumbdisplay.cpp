#include <Arduino.h>
//#include <math.h>

#include "dumbdisplay.h"
#include "__dd_cpp_include.h"


// #ifndef DD_INIT_LAYER_COUNT
//   #define DD_INIT_LAYER_COUNT  5
// #endif
// #ifndef DD_LAYER_COUNT_INC
//   #define DD_LAYER_COUNT_INC   3
// #endif  
// #if DD_INIT_LAYER_COUNT < 1
//   #error "DD_INIT_LAYER_COUNT must be at least 1"
// #endif 
// #if DD_LAYER_COUNT_INC < 2
//   #error "DD_LAYER_COUNT_INC must be at least 2"
// #endif 



// #ifdef DD_NO_PASSIVE_CONNECT
//   #warning ??? DD_NO_PASSIVE_CONNECT set ???
// #else
//   #define SUPPORT_PASSIVE
//   #define SUPPORT_MASTER_RESET 
// #endif

// #ifdef DD_NO_FEEDBACK
//   #warning ??? DD_NO_FEEDBACK set ???
// #else
//   #define ENABLE_FEEDBACK
// #endif

// #ifdef ENABLE_FEEDBACK
//   //#define FEEDBACK_BUFFER_SIZE 4
//   #define HANDLE_FEEDBACK_DURING_DELAY
//   #define READ_BUFFER_USE_BUFFER
//   #ifndef DD_NO_FEEDBACK_BYTES
//     #define FEEDBACK_SUPPORT_BYTES
//   #endif   
// #endif


// HAND_SHAKE_GAP changed from 1000 to 500 since 2024-06-22
#define HAND_SHAKE_GAP 500
//#define VALIDATE_GAP 2000


#define STORE_LAYERS
#define MORE_KEEP_ALIVE



#define SUPPORT_ENCODE_OPER

// #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
//   #define PGM_READ_BYTERS
// #endif


#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
  #define TL_BUFFER_DATA_LEN 16
#else
  #define TL_BUFFER_DATA_LEN 128
  #define SUPPORT_USE_WOIO
#endif

#define SUPPORT_CONTAINER
#define CONTAINER_LAYER_ID     -9
#define CONTAINER_LAYER_ID_STR "-9"



#define TO_BOOL(val) (val ? "1" : "0")
//#define TO_EDIAN() String(DDCheckEndian())  since 2024-09-14, use the following
#define TO_EDIAN() TO_BOOL(DDCheckEndian())  


//#define DD_DEBUG_BASIC
//#define DD_DEBUG_HS   // TODO: disable DD_DEBUG_HS
//#define DD_DEBUG_SEND_COMMAND
//#define DEBUG_ECHO_COMMAND
//#define DEBUG_VALIDATE_CONNECTION
//#define DEBUG_RECEIVE_FEEDBACK
//#define DEBUG_ECHO_FEEDBACK
//#define DEBUG_SHOW_FEEDBACK
//#define DEBUG_TUNNEL_RESPONSE

//#define DEBUG_MISSING_ENDPOINT_C
//#define DEBUG_TUNNEL_RESPONSE_C

//#define DEBUG_SHOW_IN_LINE_CHARS
//#define DEBUG_READ_FEEDBACK_BYTES
//#define DEBUG_READ_PIXEL_IMAGE


//#define SUPPORT_LONG_PRESS_FEEDBACK

#ifdef DD_NO_IDLE_CALLBACK
  #warning ??? DD_NO_IDLE_CALLBACK set ???
#else
  #define SUPPORT_IDLE_CALLBACK
#endif

#ifdef DD_NO_CONNECT_VERSION_CHANGED_CALLBACK
  #warning ??? DD_NO_CONNECT_VERSION_CHANGED_CALLBACK set ???
#else
  #define SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
  #define FIRST_CONNECT_VERSION_CONSIDER_CHANGED
#endif

#ifdef DD_NO_TUNNEL
  #warning ??? DD_NO_TUNNEL set ???
#else
  #define SUPPORT_TUNNEL
#endif

//#define TUNNEL_TIMEOUT_MILLIS 30000

#define VALIDATE_CONNECTION
//#define DEBUG_WITH_LED


#define MASTER_RESET_KEEP_CONNECTED

#ifdef DD_NO_RECONNECT
  #warning ??? DD_NO_RECONNECT set ???
#else
  #define SUPPORT_RECONNECT
  //#define RECONNECT_NO_KEEP_ALIVE_MILLIS 5000
#endif

#define VALIDATE_GAP 1000
#define RECONNECTING_VALIDATE_GAP 500

//#define SHOW_KEEP_ALIVE
//#define DEBUG_RECONNECT_WITH_COMMENT
//#define RECONNECTED_RESET_KEEP_ALIVE


// not flush seems to be a bit better for Serial (lost data) ... BUT ... seems require to flush for STM32 ... PASSIVE/blink/blink.ino
#define FLUSH_AFTER_SENT_COMMAND true
#define YIELD_AFTER_SEND_COMMAND false
#define YIELD_AFTER_HANDLE_FEEDBACK true

#ifdef DD_NO_DEBUG_INTERFACE
  #warning ??? DD_NO_DEBUG_INTERFACE set ???
#else
  #define SUPPORT_DEBUG_INTERFACE
#endif

// #ifdef DD_DEBUG_LESS_MEMORY_FOOTPRINT
//   #warning ??? DD_DEBUG_LESS_MEMORY_FOOTPRINT set ???
// #undef SUPPORT_USE_WOIO
// // #undef SUPPORT_IDLE_CALLBACK
// // #undef SUPPORT_RECONNECT
//   #undef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
//   #undef FIRST_CONNECT_VERSION_CONSIDER_CHANGED
//   #undef SUPPORT_TUNNEL
//   #undef SUPPORT_DEBUG_INTERFACE
// #endif


#define USE_MALLOC_FOR_LAYER_ARRAY


//#define DD_SID "Arduino-c9"  // DD library version (compatibility)
//#define DD_SID "Arduino-c10"  // DD library version (EXPECTED_DD_LIB_COMPATIBILITY) ... since v0.9.9-v30
//#define DD_SID "Arduino-c11"  // DD library version (EXPECTED_DD_LIB_COMPATIBILITY) ... since v0.9.9-v31
//#define DD_SID "Arduino-c12"  // DD library version (EXPECTED_DD_LIB_COMPATIBILITY) ... since v0.9.9-v34
//#define DD_SID "Arduino-c13"  // DD library version (EXPECTED_DD_LIB_COMPATIBILITY) ... since v0.9.9-v40
#define DD_SID "Arduino-c14"  // DD library version (EXPECTED_DD_LIB_COMPATIBILITY) ... since v0.9.9-v50


#include "_dd_commands.h"


#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MEGA2560)
// void display_freeram() {
//   Serial.print(F("- SRAM left: "));
//   Serial.println(freeRam());
// }

int freeRam() {  // https://docs.arduino.cc/learn/programming/memory-guide/?_gl=1*1tn4gcs*_gcl_au*MTQwODU3MTY5MS4xNzE4Mzc0MjIy*FPAU*MTQwODU3MTY5MS4xNzE4Mzc0MjIy*_ga*NDU2NzU2Mzg0LjE3MDk2NDIyMTk.*_ga_NEXN8H46L5*MTcyMzcyMzQzNi40NC4xLjE3MjM3MjQyNzUuMC4wLjUyNDIxMTk0MQ..*_fplc*RUtpd2ZNeFBsZXQ5bzhWamxXemRJTFclMkYzRWwzVE1zayUyQko3Nmg1U1htVXJCb3NuS2xLd05pR3h1cGxxZUZPNEZzN1JSeThtSWdBa21SWkVTbG1YcktZY0JTZ0xrRVlWdW1ZakszVjl1OTBZbVRFelFVQkVFY1BiaEZPUEsyQSUzRCUzRA..
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}
#endif


long _DDIdleTimeoutMillis = DD_DEF_IDLE_TIMEOUT;
bool _DDDisableParamEncoding = false;


#ifdef SUPPORT_USE_WOIO
class DDWriteOnyIO: public DDInputOutput {
  public:
    DDWriteOnyIO(DDInputOutput* io, uint16_t bufferSize = 8/*256*/): io(io) {
      this->bufferSize = bufferSize;
      this->buffer = new uint8_t[bufferSize];
      this->bufferedCount = 0;
      this->keepBuffering = false;
    }
    ~DDWriteOnyIO() {
      delete this->buffer;
    }
    void print(const String &s) {
      print(s.c_str());
    }
    void print(const char *p) {
      int len = strlen(p);
      write((uint8_t*) p, len);
      // const char *c = p;
      // while (true) {
      //   if (*c == 0) {
      //     break;
      //   }
      //   c++;
      // }
      // int count = c - p;
      // write((uint8_t*) p, count);
    }
    void write(uint8_t b) {
      write(&b, 1);
    }
    void write(const uint8_t *buf, size_t size) {
      if ((bufferedCount + size) > bufferSize) {
        _flush();
      }
      if (size > bufferSize) {
        _flush();
        io->write(buf, size);
      } else {
        const uint8_t *s = buf;
        uint8_t *t = buffer + bufferedCount;
        bool flushAfterward = false;
        for (size_t i = 0; i < size; i++) {
          if (!this->keepBuffering) {
            if (*s == '\n') {
              flushAfterward = true;
            }
          }
          *t = *s;
          s++;
          t++; 
          bufferedCount++;
        }
        if (flushAfterward) {
          _flush();
        }
      }
    }    
    inline void flush() {
      if (this->keepBuffering) {
        return;
      }
      _flush();
    }
public:
    void setKeepBuffering(bool keep) {
      this->keepBuffering = keep;
    }    
private:
    void _flush() {
      if (bufferedCount > 0) {
        if (false) {
          Serial.print(". flush ");
          Serial.print(bufferedCount);
          if (this->keepBuffering) {
            Serial.print(" ... KEEP");
          }
          Serial.println();
        }
        io->write(buffer, bufferedCount);
        bufferedCount = 0;
      }
    }
  private:
    DDInputOutput* io;
    uint8_t bufferSize;
    uint8_t* buffer;  
    uint8_t bufferedCount;
    bool keepBuffering;
};
#endif


#define YIELD() delay(1)


#ifdef DDIO_USE_DD_SERIAL
DDSerial* _The_DD_Serial = NULL;
#endif

namespace DDImpl {

bool _CanLogToSerial();


class IOProxy {
  public: 
    IOProxy(DDInputOutput *pIO) {
      this->pIO = pIO;
#ifdef SUPPORT_RECONNECT      
      this->reconnectKeepAliveMillis = 0;
      this->lastKeepAliveMillis = 0;
      this->reconnectEnabled = false;
#endif
      this->reconnecting = false;
    }
    const char* getWhat() {
      return pIO->getWhat();
    }
    bool available();
    inline const String& get() {
        return data;
    }
    inline void clear() {
      data = "";
    }
    inline void print(const String &s) {
      pIO->print(s);
    }
    inline void print(const char *p) {
      pIO->print(p);
    }
    inline void write(uint8_t b) {
      pIO->write(b);
    }
    void keepAlive();
    void validConnection();
    void setReconnectRCId(const String& rcId) {
#ifdef SUPPORT_RECONNECT      
      this->reconnectRCId = rcId;
      this->reconnectEnabled = true;
      this->reconnectKeepAliveMillis = 0;
#endif
    }
    bool isReconnecting() {
      return this->reconnecting;
    }
#ifdef FEEDBACK_SUPPORT_BYTES
    inline bool charAvailable() {
      return pIO->available();
    }
    char getChar() {
      return pIO->read();
    }
#endif    
  private:
    DDInputOutput *pIO;
    bool fromSerial;
    String data;  
#if defined (SUPPORT_IDLE_CALLBACK) || defined (SUPPORT_RECONNECT)
    unsigned long lastKeepAliveMillis;
#endif
#ifdef SUPPORT_RECONNECT      
    bool reconnectEnabled;
    String reconnectRCId;
    long reconnectKeepAliveMillis;
#endif
    bool reconnecting;
};



/*volatile */bool _Connected = false;
/*volatile */int _ConnectVersion = 0;
//bool _Reconnecting = false;

#ifdef SUPPORT_IDLE_CALLBACK
  /*volatile */DDIdleCallback _IdleCallback = NULL; 
#endif
#ifdef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
  /*volatile */DDConnectVersionChangedCallback _ConnectVersionChangedCallback = NULL; 
#endif

#ifdef SUPPORT_DEBUG_INTERFACE
  DDDebugInterface *_DebugInterface;
#endif

bool IOProxy::available() {
  bool done = false;
  while (!done && pIO->available()) {
    char c =  pIO->read();
    if (c == '\n') {
      done = true;
    } else {
#ifdef DEBUG_SHOW_IN_LINE_CHARS
      int dataLen = data.length();
      if ((dataLen == 0 && c != '<') || (dataLen > 0 && dataLen < 30)) {
        Serial.print(". in:[");
        Serial.print(c);
        Serial.println("]");
      }
#endif      
      data += c;
//         data = data + c;
    }
  }
  return done;
}
// const String& IOProxy::get() {
//   return data;
// }
// void IOProxy::clear() {
//   //data.remove(0, data.length());
//   data = "";
// }
// void IOProxy::print(const String &s) {
//   pIO->print(s);
// }
// void IOProxy::print(const char *p) {
//   pIO->print(p);
// }
// void IOProxy::write(uint8_t b) {
//   pIO->write(b);
// }
void IOProxy::keepAlive() {
#if defined (SHOW_KEEP_ALIVE) || defined(DEBUG_RECONNECT_WITH_COMMENT)
  this->print("// KEEP ALIVE\n");
#endif
#ifdef SUPPORT_RECONNECT      
  this->lastKeepAliveMillis = millis();
#endif
  pIO->keepAlive();
}
void IOProxy::validConnection() {
#ifdef DEBUG_VALIDATE_CONNECTION
  Serial.print("... validate connection ... ");
  Serial.print(lastKeepAliveMillis);
  if (reconnectEnabled) {
    Serial.print(" ... RE enabled ");
  }
  Serial.println(" ...");
#endif 
  pIO->validConnection();
#if defined(SUPPORT_MASTER_RESET)
  if (lastKeepAliveMillis == 0) {  // since 2023-06-02
    lastKeepAliveMillis = millis();  // don't wait for keep alive 
  }
#endif  
#if defined (SUPPORT_IDLE_CALLBACK) || defined (SUPPORT_RECONNECT)
  bool needReconnect = false;
  if (this->lastKeepAliveMillis > 0) {
    long now = millis();
    long notKeptAliveMillis = now - this->lastKeepAliveMillis; 
    if (notKeptAliveMillis > _DDIdleTimeoutMillis) {
      needReconnect = true;
#ifdef SUPPORT_IDLE_CALLBACK      
      if (_IdleCallback != NULL) {
        long idleForMillis = notKeptAliveMillis - _DDIdleTimeoutMillis;
        _IdleCallback(idleForMillis, DDIdleConnectionState::IDLE_RECONNECTING);
      }
#endif      
#ifdef DD_DEBUG_BASIC  
      if (_CanLogToSerial()) Serial.println("!!! 'keep alive' message not received for too long ==> reconnect");
#endif
    }
#ifdef SUPPORT_MASTER_RESET
    if (needReconnect) {
      this->reconnecting = true;
    }
#endif        
#ifdef SUPPORT_RECONNECT
    if (this->reconnectEnabled && needReconnect) {
      this->reconnecting = true;
#ifdef SUPPORT_DEBUG_INTERFACE
      if (_DebugInterface != NULL) {
        _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_RECONNECTING);
      }
#endif
      YIELD();
#ifdef DEBUG_RECONNECT_WITH_COMMENT 
this->print("// NEED TO RECONNECT\n");
#endif
#ifdef DEBUG_VALIDATE_CONNECTION
      Serial.print("=== reconnect: ");
      Serial.println(this->reconnectRCId);
#endif
      this->print("%%>RECON>");
      this->print(DD_SID);
      this->print(":");
      this->print(this->reconnectRCId);
      // if (!_EnableDoubleClick) {
      //   this->print(",dblclk=0");
      // }
      this->print("\n");
      this->reconnectKeepAliveMillis = this->lastKeepAliveMillis;
    } else if (this->reconnectKeepAliveMillis > 0) {
      this->reconnecting = false;
      _ConnectVersion = _ConnectVersion + 1;
#ifdef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK   
      if (_ConnectVersionChangedCallback != NULL) {
        _ConnectVersionChangedCallback(_ConnectVersion);
      }
#endif   
#ifdef DEBUG_RECONNECT_WITH_COMMENT
      this->print("// DETECTED RECONNECTION\n");
#endif      
#ifdef DEBUG_VALIDATE_CONNECTION
      Serial.print("*** reconnected: ");
      Serial.println(_ConnectVersion);
#endif
      this->reconnectKeepAliveMillis = 0;
#ifdef RECONNECTED_RESET_KEEP_ALIVE
      this->lastKeepAliveMillis = millis();
#endif
#ifdef SUPPORT_DEBUG_INTERFACE
      if (_DebugInterface != NULL) {
        _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_RECONNECTED);
      }
#endif
    }
#endif
  }
#endif  
}



//volatile bool _Preconneced = false;
//volatile bool _Connected = false;
/*volatile */short _DDCompatibility = 0;  // see DD_APP_COMPATIBILITY
/*volatile */int _NextLid = 0;
/*volatile*/int _NextImgId = 0;
/*volatile*/int _NextBytesId = 0;



DDObject** _DDLayerArray = NULL;
int _MaxDDLayerCount = 0;

DDInputOutput* /*volatile */_DDIO = NULL;

#ifdef SUPPORT_USE_WOIO
DDInputOutput* /*volatile */_WODDIO = NULL;
/*volatile */uint16_t _SendBufferSize = 0;//DD_DEF_SEND_BUFFER_SIZE;
#else
#define _WODDIO _DDIO
#endif

#ifdef SUPPORT_CONTAINER
GraphicalDDLayer* __RootLayer = NULL;
#endif


inline void _SetIO(DDInputOutput* io, uint16_t sendBufferSize, long idleTimeout) {
  _DDIO = io;
#ifdef SUPPORT_USE_WOIO  
  if (_WODDIO != NULL) delete _WODDIO;
  if (sendBufferSize > 0 && io->canUseBuffer()) {
    _SendBufferSize = sendBufferSize;
    _WODDIO = new DDWriteOnyIO(io, sendBufferSize);
  } else {
    _SendBufferSize = 0;
    _WODDIO = io;
  }
#endif
  _DDIdleTimeoutMillis = idleTimeout;
}



IOProxy* /*volatile */_ConnectedIOProxy = NULL;
/*volatile */bool _ConnectedFromSerial = false;

bool _CanLogToSerial() {
  if (_ConnectedIOProxy != NULL) {
    return !_ConnectedFromSerial;
  } else {
    return _DDIO != NULL && !(_DDIO->isForSerial() || _DDIO->isBackupBySerial());
  }
}

// #ifdef DEBUG_WITH_LED
// /*volatile */int _DebugLedPin = -1;
// #endif
#ifdef DEBUG_ECHO_FEEDBACK 
/*volatile */bool _DebugEnableEchoFeedback = false;
#endif
// #ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
// volatile bool _NoEncodeInt = false;
// #endif


#define IS_FLOAT_ZERO(f) ((((f)<0?-(f):(f)) - 0.0) < 0.001)
#define IS_FLOAT_WHOLE(f) IS_FLOAT_ZERO((f) - (int) (f))

#define DD_FLOAT_DP 3

#ifdef DD_CONDENSE_COMMAND
  #define TO_C_INT(i) (DDIntEncoder(i).encoded())
// #ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
// #define TO_C_INT(i) (_NoEncodeInt ? String(i) : DDIntEncoder(i).encoded())
// #else
// #define TO_C_INT(i) (DDIntEncoder(i).encoded())
// #endif
  #define TO_NUM(num) IS_FLOAT_WHOLE(num) ? String((int) num) : String(num, DD_FLOAT_DP) 
#else
  #define TO_C_INT(i) String(i)
  #define TO_NUM(num) String(num) 
#endif
#define TO_C_NUM(num) IS_FLOAT_WHOLE(num) ? TO_C_INT((int) num) : String(num, DD_FLOAT_DP) 


void __SendErrorComment(const char* comment);

volatile bool _SendingCommand = false;
volatile bool _HandlingFeedback = false;



#ifdef SUPPORT_PASSIVE

#define _C_PRECONNECTING  1
#define _C_PRECONNECTED   3
#define _C_IOPROXY_SET    4
#define _C_HANDSHAKE      5
//#define _C_MASTER_RESET   6

struct _ConnectState {
  _ConnectState(): step(0) {}
  short step;
  long startMillis;
  long lastCallMillis;
  bool firstCall;
  long hsStartMillis;
  long hsNextMillis;
  IOProxy* pIOProxy;
  IOProxy* pBUSerialIOProxy;
  DDInputOutput *pBUSIO;
  short compatibility;
};
_ConnectState _C_state;

bool __Connect(/*bool calledPassive = false*/) {
// #if defined(MASTER_RESET_KEEP_CONNECTED)
//   if (_C_state.step == _C_MASTER_RESET) {
//       _C_state.step = 0;
//       return false;
//   }
// #endif
  bool mustLoop = !_DDIO->canConnectPassive();
  if (_C_state.step > 0 && _C_state.hsStartMillis > 0) {
    long diffMillis = millis() - _C_state.hsStartMillis;
    if (diffMillis > _DDIdleTimeoutMillis) {
      // start over
      _C_state.step = 0;
      return false;
    }
  }
// Serial.print(">");
// Serial.print(_C_state.step);
  if (_C_state.step == 0) {
    _C_state.startMillis = millis();
    _C_state.lastCallMillis = _C_state.startMillis;
    _C_state.hsStartMillis = 0;
    _C_state.firstCall = true;
    _C_state.step = _C_PRECONNECTING/*1*/;
 #ifdef SUPPORT_DEBUG_INTERFACE
   if (_DebugInterface != NULL) {
      _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_NOT_CONNECTED);
    }
#endif
  }
  if (_C_state.step == _C_PRECONNECTING/*1*/) {
    if (!_DDIO->preConnect(_C_state.firstCall)) {
#ifdef SUPPORT_IDLE_CALLBACK
      bool checkIdle = _IdleCallback != NULL;
      if (checkIdle/*!_IsInPassiveMode && _IdleCallback != NULL*/) {
        long now = millis();
        long diffMillis = now - _C_state.lastCallMillis;
        if (diffMillis >= HAND_SHAKE_GAP) {
          long idleForMillis = now - _C_state.startMillis;
          _IdleCallback(idleForMillis, DDIdleConnectionState::IDLE_NOT_CONNECTED);
          _C_state.lastCallMillis = now;
        }
      }
#endif      
      _C_state.firstCall = false;
      return false;
    }
    _C_state.step = _C_PRECONNECTED/*3*//*2*/;
#ifdef SUPPORT_DEBUG_INTERFACE
    if (_DebugInterface != NULL) {
      _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_CONNECTING);
    }
#endif
  }
  if (_C_state.step == _C_PRECONNECTED/*3*/) {
    _C_state.hsNextMillis = millis();
    //IOProxy ioProxy(_DDIO);
    if (_C_state.pIOProxy != NULL) {
      delete _C_state.pIOProxy;
    }
    if (_C_state.pBUSerialIOProxy != NULL) {
      delete _C_state.pBUSerialIOProxy;
    }
    if (_C_state.pBUSIO != NULL) {
      delete _C_state.pBUSIO;
    }
    _C_state.pIOProxy = new IOProxy(_DDIO);
    _C_state.pBUSerialIOProxy = NULL;
    _C_state.pBUSIO = NULL;
    if (_DDIO->isBackupBySerial()) {
      //pSIO = new DDInputOutput(_DDIO);
      _C_state.pBUSIO = _DDIO->newForSerialConnection();
      _C_state.pBUSerialIOProxy = new IOProxy(_C_state.pBUSIO);
    }
    _C_state.hsStartMillis = millis();
    _C_state.step = _C_IOPROXY_SET/*4*/;
    // faster
    //return false;
  }
  if (_C_state.step == _C_IOPROXY_SET/*4*/) {
    while (true) {
      if (mustLoop) YIELD();
      long now = millis();
      if (now > _C_state.hsNextMillis) {
#ifdef SUPPORT_DEBUG_INTERFACE
        if (_DebugInterface != NULL) {
          _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_CONNECTING);
        }
#endif
        _C_state.pIOProxy->print("ddhello\n");
        if (_C_state.pBUSerialIOProxy != NULL) {
#if defined(DD_EXPERIMENTAL)
          _C_state.pBUSerialIOProxy->print("ddhello");
          const char* viaWhat = _C_state.pIOProxy->getWhat();
          if (viaWhat != NULL) {
            _C_state.pBUSerialIOProxy->print("__via:");
            _C_state.pBUSerialIOProxy->print(viaWhat);
          }
          _C_state.pBUSerialIOProxy->print("\n");
#else
          _C_state.pBUSerialIOProxy->print("ddhello\n");
#endif
        }
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:ddhello");
#endif        
#ifdef SUPPORT_IDLE_CALLBACK
        bool checkIdle = _IdleCallback != NULL;
        if (checkIdle/*!_IsInPassiveMode && _IdleCallback != NULL*/) {
          long idleForMillis = now - _C_state.startMillis;
          _IdleCallback(idleForMillis, DDIdleConnectionState::IDLE_CONNECTING);
        }
#endif      
        _C_state.hsNextMillis = now + HAND_SHAKE_GAP;
      }
      bool fromBUSerial = false;
      bool available = _C_state.pIOProxy->available();
      if (!available && _C_state.pBUSerialIOProxy != NULL) {
        if (_C_state.pBUSerialIOProxy->available()) {
          available = true;
          fromBUSerial = true;
        }
      }
      if (available) {
        const String& data = fromBUSerial ? _C_state.pBUSerialIOProxy->get() : _C_state.pIOProxy->get();
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:data-[" + data + "]");
#endif  
        if (data == "ddhello") {
          if (fromBUSerial) {
            _SetIO(_C_state.pBUSIO, DD_DEF_SEND_BUFFER_SIZE, DD_DEF_IDLE_TIMEOUT);
            //_DDIO = pSIO;
            _C_state.pBUSIO = NULL;
          }
          if (_ConnectedIOProxy != NULL) {
            delete _ConnectedIOProxy;
          }
          _ConnectedIOProxy = new IOProxy(_DDIO);
//          _ConnectedFromSerial = fromSerial;
          //_ConnectedFromSerial = _DDIO->isSerial();
          _ConnectedFromSerial =  fromBUSerial || _DDIO->isSerial();
#ifdef DD_DEBUG_BASIC  
          if (_CanLogToSerial()) Serial.println("--- connection established");
#endif
          if (_CanLogToSerial()) {
            Serial.println("**********");
#ifdef DD_DEBUG_BASIC  
            Serial.print("* _DDIO.isSerial()=");
            Serial.println(_DDIO->isSerial());
            Serial.print("* _DDIO.isForSerial()=");
            Serial.println(_DDIO->isForSerial());
            Serial.print("* _DDIO.isBackupBySerial()=");
            Serial.println(_DDIO->isBackupBySerial());
            Serial.print("* _DDIO.canConnectPassive()=");
            Serial.println(_DDIO->canConnectPassive());
            Serial.print("* _DDIO.canUseBuffer()=");
            Serial.println(_DDIO->canUseBuffer());
#endif
#ifdef SUPPORT_USE_WOIO
            Serial.print("* _SendBufferSize=");
            Serial.println(_SendBufferSize);
#endif
            //Serial.print("* _EnableDoubleClick=");
            //Serial.println(_EnableDoubleClick ? "yes" : "no");
            Serial.println("**********");
            Serial.flush();
          }
          _C_state.step = _C_HANDSHAKE/*5*/;
          // if (mustLoop) {
          //   break;
          // }
          // return false;
          break;  // faster
        }
#ifdef DD_DEBUG_HS   
        if (_ConnectedIOProxy != NULL) {   
          Serial.println("handshake:DONE");
        } else {      
          Serial.println("handshake:... got=[" + data + "]");
        }
#endif        
        if (fromBUSerial) 
          _C_state.pBUSerialIOProxy->clear();
        else
          _C_state.pIOProxy->clear();  
      }
      if (!mustLoop) {
        break;
      }
    }
    // moved up
    // if (pSerialIOProxy != NULL)
    //   delete pSerialIOProxy;
    // if (pSIO != NULL)
    //   delete pSIO;
      if (!mustLoop) {
        return false;
      }
//Serial.print("$$$$$$$$$$ "); Serial.println(_C_state.step);      
  }
  //int compatibility = 0;
  if (_C_state.step == _C_HANDSHAKE/*5*/) { 
    //_C_state.compatibility = 0;
    if (true) {
      // since 2024-11-08 ... faster sent >init>
      _C_state.hsNextMillis = millis() + HAND_SHAKE_GAP;
    } else {
      _C_state.hsNextMillis = millis();
    }
    _C_state.step = 6;
  }
  if (_C_state.step == 6) { 
    //IOProxy ioProxy(_DDIO);
    while (true) {
      if (mustLoop) YIELD();
      long now = millis();
      if (now > _C_state.hsNextMillis) {
// #ifdef DEBUG_WITH_LED
//         if (debugLedPin != -1) {
//           debugLedOn = !debugLedOn;
//           digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
//         }
// #endif
//Serial.println((_ConnectedFromSerial ? "SERIAL" : "NON-SERIAL"));
        //ioProxy.print(">init>:Arduino-c1\n");
        _ConnectedIOProxy->/*ioProxy.*/print(">init>:");
        _ConnectedIOProxy->/*ioProxy.*/print(DD_SID);
        // if (!_EnableDoubleClick) {
        //   ioProxy.print(",dblclk=0");
        // }
        _ConnectedIOProxy->/*ioProxy.*/print("\n");
        _C_state.hsNextMillis = now + HAND_SHAKE_GAP;
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:sent-ddinit");
#endif  
      }
      if (_ConnectedIOProxy->/*ioProxy.*/available()) {
        const String& data = _ConnectedIOProxy->/*ioProxy.*/get();
        short compatibility = -1;
        if (data == "<init<") {
          //break;
          compatibility = 0;
          //_C_state.step = 7;
          //return false;
        } else if (data.startsWith("<init<:")) {
          compatibility = data.substring(7).toInt();
          //break;
          //_C_state.step = 7;
          //return false;
        } 
        if (compatibility != -1) {
          _C_state.compatibility = compatibility;
          _C_state.step = 7;
          if (true) {
            // since 2023-06-17
            _ConnectedIOProxy->/*ioProxy.*/clear();  
          }
          if (mustLoop) {
            break;
          }
          return false;
        } 
        _ConnectedIOProxy->/*ioProxy.*/clear();  
      }
      if (!mustLoop) {
        break;
      }
    }
    if (!mustLoop) {
      return false;
    }
  }
  _Connected = true;
  _ConnectVersion = 1;
//  _ConnectedIOProxy = new IOProxy(_DDIO);
  _DDCompatibility = _C_state.compatibility;
  if (false) {
    // ignore any input in 1000ms window
    delay(1000);
    while (_DDIO->available()) {
      _DDIO->read();
    }
  }
  if (true) {       
    _DDIO->print("// Connected to DD c" + String(_C_state.compatibility) + "\n"/*.c_str()*/);
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MEGA2560)
    _DDIO->print("// $ Free SRAM: " + String(freeRam() / 1024.0) + "KB" + "\n");
#elif defined(ESP32)
    _DDIO->print(String("// $ Sketch: ") + String(ESP.getSketchSize() / 1024.0) + "KB" + " / Free: " + String(ESP.getFreeSketchSpace() / 1024.0) + "KB" + "\n");
    _DDIO->print(String("// $ Heap: " + String(ESP.getHeapSize() / 1024.0) + "KB") + " / Free: " + String(ESP.getFreeHeap() / 1024.0) + "KB" + "\n");
    _DDIO->print(String("// $ PSRAM: " + String(ESP.getPsramSize() / 1024.0) + "KB") + " / Free: " + String(ESP.getFreePsram() / 1024.0) + "KB" + "\n");
#endif    
    //_DDIO->flush();
    if (false) {
      // *** debug code
      for (int i = 0; i < 10; i++) {
        delay(500);
        _DDIO->print("// connected to DD c" + String(_C_state.compatibility) + "\n"/*.c_str()*/);
      }
    }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** CONNECTED");
#endif        
    _C_state.step = 0;
  }
// #ifdef DEBUG_WITH_LED
//     if (debugLedPin != -1) {
//       digitalWrite(debugLedPin, LOW);
//     }
// #endif
    // if (false) {
    //   // *** debug code
    //   _DDIO->print("// connection to DD made\n");
    //    _sendCommand0("", "// *** connection made ***");
    // }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** DONE MAKE CONNECTION");
#endif
#if defined(SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK) && defined(FIRST_CONNECT_VERSION_CONSIDER_CHANGED)
    if (_ConnectVersionChangedCallback != NULL) {
      _ConnectVersionChangedCallback(_ConnectVersion);  // will be 1
    }
#endif   
  return true;
}
bool _Connect(bool calledPassive = false) {
  if (_Connected)
    return true;
  if (!calledPassive) {
    _C_state.step = 0;
  }
  while (true) {
    YIELD();
    if (__Connect()) {
#ifdef SUPPORT_DEBUG_INTERFACE
      if (_DebugInterface != NULL) {
        _DebugInterface->logConnectionState(DDDebugConnectionState::DEBUG_CONNECTED);
      }
#endif
      return true;
    }
    if (calledPassive) {
      return false;
    }
  }
}

#else

void _Connect(/*long maxWaitMillis = -1, bool calledPassive = false*/) {
  if (_Connected)
    return;
  {
    long startMillis = millis();
    long lastCallMillis = startMillis;
    bool firstCall = true;
    while (true) {
      YIELD();
      if (_DDIO->preConnect(firstCall)) {
        break;
      }
#ifdef SUPPORT_IDLE_CALLBACK
        if (_IdleCallback != NULL) {
          long now = millis();
          long diffMillis = now - lastCallMillis;
          if (diffMillis >= HAND_SHAKE_GAP) {
            long idleForMillis = now - startMillis;
            _IdleCallback(idleForMillis, DDIdleConnectionState::IDLE_NOT_CONNECTED);
            lastCallMillis = now;
          }
        }
#endif      
      firstCall = false;
    }
  }
//   if (!_DDIO->isSerial()) {
//     Serial.println("**********");
// #ifdef SUPPORT_USE_WOIO
//     Serial.print("* _SendBufferSize=");
//     Serial.println(_SendBufferSize);
// #endif
//     //Serial.print("* _EnableDoubleClick=");
//     //Serial.println(_EnableDoubleClick ? "yes" : "no");
//     Serial.println("**********");
//     Serial.flush();
//   }
  {
    long nextTime = 0;
    IOProxy ioProxy(_DDIO);
    IOProxy* pSerialIOProxy = NULL;
    DDInputOutput *pSIO = NULL;
    if (_DDIO->isBackupBySerial()) {
      //pSIO = new DDInputOutput(_DDIO);
      pSIO = _DDIO->newForSerialConnection();
      pSerialIOProxy = new IOProxy(pSIO);
    }
    long startMillis = millis();
    while (true) {
      YIELD();
      long now = millis();
      if (now > nextTime) {
        ioProxy.print("ddhello\n");
        if (pSerialIOProxy != NULL) 
          pSerialIOProxy->print("ddhello\n");
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:ddhello");
#endif        
#ifdef SUPPORT_IDLE_CALLBACK
        if (_IdleCallback != NULL) {
          long idleForMillis = now - startMillis;
          _IdleCallback(idleForMillis, DDIdleConnectionState::IDLE_CONNECTING);
        }
#endif      
        nextTime = now + HAND_SHAKE_GAP;
      }
      bool fromSerial = false;
      bool available = ioProxy.available();
      if (!available && pSerialIOProxy != NULL) {
        if (pSerialIOProxy->available()) {
          available = true;
          fromSerial = true;
        }
      }
      if (available) {
        const String& data = fromSerial ? pSerialIOProxy->get() : ioProxy.get();
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:data-" + data);
#endif        
        if (data == "ddhello") {
          if (fromSerial) {
            _SetIO(pSIO, DD_DEF_SEND_BUFFER_SIZE, DD_DEF_IDLE_TIMEOUT);
            //_DDIO = pSIO;
            pSIO = NULL;
          }
#ifdef MASTER_RESET_KEEP_CONNECTED
          if (_ConnectedIOProxy != NULL) {
            delete _ConnectedIOProxy;
          }
#endif          
          _ConnectedIOProxy = new IOProxy(_DDIO);
          _ConnectedFromSerial = fromSerial;
          //_ConnectedFromSerial = _DDIO->isSerial();
          break;
        }
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:DONE");
#endif        
        if (fromSerial) 
          pSerialIOProxy->clear();
        else
          ioProxy.clear();  
      }
    }
    if (pSerialIOProxy != NULL)
      delete pSerialIOProxy;
    if (pSIO != NULL)
      delete pSIO;
  }
  int compatibility = 0;
  { 
    long nextTime = 0;
    IOProxy ioProxy(_DDIO);
    while (true) {
      YIELD();
      long now = millis();
      if (now > nextTime) {
//Serial.println((_ConnectedFromSerial ? "SERIAL" : "NON-SERIAL"));
        //ioProxy.print(">init>:Arduino-c1\n");
        ioProxy.print(">init>:");
        ioProxy.print(DD_SID);
        // if (!_EnableDoubleClick) {
        //   ioProxy.print(",dblclk=0");
        // }
        ioProxy.print("\n");
        nextTime = now + HAND_SHAKE_GAP;
      }
      if (ioProxy.available()) {
        const String& data = ioProxy.get();
        if (data == "<init<")
          break;
        if (data.startsWith("<init<:")) {
            compatibility = data.substring(7).toInt();
            break;
        }  
        ioProxy.clear();  
      }
    }
  }
  _Connected = true;
  _ConnectVersion = 1;
//  _ConnectedIOProxy = new IOProxy(_DDIO);
  _DDCompatibility = compatibility;
  if (false) {
    // ignore any input in 1000ms window
    delay(1000);
    while (_DDIO->available()) {
      _DDIO->read();
    }
  }
  if (true) {       
    _DDIO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
    //_DDIO->flush();
    if (false) {
      // *** debug code
      for (int i = 0; i < 10; i++) {
        delay(500);
        _DDIO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
      }
    }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** CONNECTED");
#endif        
  }
    // if (false) {
    //   // *** debug code
    //   _DDIO->print("// connection to DD made\n");
    //    _sendCommand0("", "// *** connection made ***");
    // }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** DONE MAKE CONNECTION");
#endif        
}

#endif

int _AllocBytesId() {
  int bytesId = _NextBytesId++;
  return bytesId;
}
int _AllocImgId() {
  int imgId = _NextImgId++;
  return imgId;
}
int _AllocLid() {
  _Connect();
  int lid = _NextLid++;
#ifdef STORE_LAYERS  
  if (DD_LAYER_COUNT_INC > 0) {
    if (lid >= _MaxDDLayerCount) {
      if (true) {
        //int oriLayerCount = _MaxDDLayerCount;
        _MaxDDLayerCount = lid + (lid == 0 ? DD_INIT_LAYER_COUNT : DD_LAYER_COUNT_INC);
#ifdef USE_MALLOC_FOR_LAYER_ARRAY
        _DDLayerArray = (DDObject**) realloc(_DDLayerArray, _MaxDDLayerCount * sizeof(DDObject*));
#else
        int oriLayerCount = _MaxDDLayerCount;
        DDObject** oriLayerArray = _DDLayerArray;
        DDObject** layerArray = new DDObject*[_MaxDDLayerCount];
        if (oriLayerArray != NULL) {
          //memcpy(layerArray, oriLayerArray, (_MaxDDLayerCount - DD_LAYER_INC) * sizeof(DDObject*));
          for (int i = 0; i < oriLayerCount; i++) {
            layerArray[i] = oriLayerArray[i];
          }
          delete oriLayerArray;
        }
        _DDLayerArray = layerArray;
#endif
      } else {
        Serial.println("!!! unexpected");
        // _MaxDDLayerCount = lid + DD_LAYER_INC;
        // DDObject** oriLayerArray = _DDLayerArray;
        // DDObject** layerArray = (DDObject**) malloc(_MaxDDLayerCount * sizeof(DDObject*));
        // if (oriLayerArray != NULL) {
        //   memcpy(layerArray, oriLayerArray, (_MaxDDLayerCount - DD_LAYER_INC) * sizeof(DDObject*));
        //   free(oriLayerArray);
        // }
        // _DDLayerArray = layerArray;
      }
    }
  } else {
    Serial.println("!!! unexpected");
    // DDObject** oriLayerArray = _DDLayerArray;
    // DDObject** layerArray = (DDObject**) malloc((lid + 1) * sizeof(DDObject*));
    // if (oriLayerArray != NULL) {
    //   memcpy(layerArray, oriLayerArray, lid * sizeof(DDObject*));
    //   free(oriLayerArray);
    // }
    // _DDLayerArray = layerArray;
  }
#endif
  return lid;
}


inline int _LayerIdToLid(const String& layerId) {
  return atoi(layerId.c_str());
}
void _PostCreateLayer(DDLayer* pLayer) {
#ifdef STORE_LAYERS  
  int lid = _LayerIdToLid(pLayer->getLayerId());
  _DDLayerArray[lid] = pLayer;
#endif
}
void _PreDeleteLayer(DDLayer* pLayer) {
#ifdef STORE_LAYERS  
  int lid = _LayerIdToLid(pLayer->getLayerId());
  _DDLayerArray[lid] = NULL;
#endif
}

#ifdef SUPPORT_TUNNEL
inline int _AllocTid() {
  return _AllocLid();
}
void _PostCreateTunnel(DDTunnel* pTunnel, bool connectNow) {
#ifdef STORE_LAYERS  
  int8_t lid = _LayerIdToLid(pTunnel->getTunnelId());
  _DDLayerArray[lid] = pTunnel;
#endif
  pTunnel->afterConstruct(connectNow);
}
void _PreDeleteTunnel(DDTunnel* pTunnel) {
#ifdef STORE_LAYERS  
  int8_t lid = _LayerIdToLid(pTunnel->getTunnelId());
  _DDLayerArray[lid] = NULL;
#endif
}
#endif

#ifdef VALIDATE_CONNECTION
long _LastValidateConnectionMillis = 0;
#endif
void _ValidateConnection() {
#ifdef VALIDATE_CONNECTION
    if (_ConnectedIOProxy != NULL) {
      long validateGap = _ConnectedIOProxy->isReconnecting() ? RECONNECTING_VALIDATE_GAP : VALIDATE_GAP; 
      long now = millis();
      long diff = now - _LastValidateConnectionMillis;
      if (diff >= validateGap/*VALIDATE_GAP*//*2000*//*5000*/) {
        _ConnectedIOProxy->validConnection();
        _LastValidateConnectionMillis = now;
      }
    }
#endif
}

#ifdef READ_BUFFER_USE_BUFFER
String _ReadFeedbackBuffer;
#endif
#ifdef FEEDBACK_SUPPORT_BYTES
struct FeedbackDataBuffer {
  FeedbackDataBuffer(): pendingByteCount(-1), totalByteCount(0), pFeedback(NULL), pBytes(NULL) {}
  // inline const uint8_t* getFeedbackBytes() const {
  //   if (pendingByteCount == 0) {
  //     return pBytes;
  //   } else {
  //     return NULL;
  //   }
  // }
  int32_t pendingByteCount;  // -1 means no bytes; 0 means just used
  int32_t totalByteCount;
  String* pFeedback;
  uint8_t* pBytes;
};
FeedbackDataBuffer _ReadFeedbackDataBuffer;
#endif

// #ifdef FEEDBACK_SUPPORT_BYTES
// String* _ReadFeedback(String& buffer, uint8_t*& pFeedbackBytes) {
// #else
String* _ReadFeedback(String& buffer) {
//#endif
#ifdef VALIDATE_CONNECTION
    // long now = millis();
    // long diff = now - _LastValidateConnectionMillis;
    // if (diff >= VALIDATE_GAP/*2000*//*5000*/) {
    //   /*_Reconnecting = !*/_ConnectedIOProxy->validConnection();
    //   _LastValidateConnectionMillis = now;
    // }
    _ValidateConnection();
#endif
#ifdef FEEDBACK_SUPPORT_BYTES
  if (_ConnectedIOProxy != NULL) {
    if (_ReadFeedbackDataBuffer.pendingByteCount > 0)  {
      while (_ConnectedIOProxy->charAvailable()) {
        char c = _ConnectedIOProxy->getChar();
        int byteIdx = _ReadFeedbackDataBuffer.totalByteCount - _ReadFeedbackDataBuffer.pendingByteCount;
#ifdef DEBUG_READ_FEEDBACK_BYTES
        if (byteIdx < 20) {
          Serial.print(". FBb[");
          Serial.print(byteIdx);
          Serial.print("]:[");
          Serial.print(c);
          Serial.print("](");
          Serial.print((int) c);
          Serial.println(")");
        }
#endif        
        _ReadFeedbackDataBuffer.pBytes[byteIdx] = c;
        if (--_ReadFeedbackDataBuffer.pendingByteCount == 0) {
          //pFeedbackBytes = _ReadFeedbackDataBuffer.pBytes;
#ifdef DEBUG_READ_FEEDBACK_BYTES
          Serial.print("*** done reading ");
          Serial.print(_ReadFeedbackDataBuffer.totalByteCount);
          Serial.print(" FB bytes for [");
         if (_ReadFeedbackDataBuffer.pFeedback->length() < 100) {
            Serial.print(*_ReadFeedbackDataBuffer.pFeedback);
         }
          Serial.println("]");
#endif
          return _ReadFeedbackDataBuffer.pFeedback;  
        }
      }
      return NULL;
    }
    if (_ReadFeedbackDataBuffer.pendingByteCount == 0)  {
      if (_ReadFeedbackDataBuffer.pBytes != NULL) {
#ifdef DEBUG_READ_FEEDBACK_BYTES
        Serial.println("*** delete FB bytes");
#endif
        delete _ReadFeedbackDataBuffer.pBytes;
        _ReadFeedbackDataBuffer.pBytes = NULL;
      }
      _ReadFeedbackDataBuffer.pendingByteCount = -1;
#ifdef DEBUG_READ_FEEDBACK_BYTES
        Serial.println("*** done with FB bytes");
#endif
    }
  }
#endif
  if (_ConnectedIOProxy == NULL || !_ConnectedIOProxy->available()) {
    return NULL;
  }
  const String& data = _ConnectedIOProxy->get();
#ifdef DEBUG_RECEIVE_FEEDBACK
  Serial.print("received: ");  
  Serial.println(data);
#endif
#ifdef READ_BUFFER_USE_BUFFER
#ifdef FEEDBACK_SUPPORT_BYTES
    int sepIdx = data.indexOf(">`bYtEs`>");
    if (sepIdx != -1) {
      if (data.endsWith("@")) {
        long byteCount = data.substring(sepIdx + 9, data.length() - 1).toInt();
        buffer = data.substring(0, sepIdx);
#ifdef DEBUG_READ_FEEDBACK_BYTES
        Serial.print("*** start reading ");
        Serial.print(byteCount);
        Serial.print(" FB bytes for ["); 
        if (buffer.length() < 100) {
          Serial.print(buffer);
        }
        Serial.println("]"); 
#endif
        _ReadFeedbackDataBuffer.pFeedback = &buffer;
        _ReadFeedbackDataBuffer.pendingByteCount = byteCount;
        _ReadFeedbackDataBuffer.totalByteCount = byteCount;
        _ReadFeedbackDataBuffer.pBytes = new uint8_t[byteCount];
        _ConnectedIOProxy->clear();
        return NULL;
      }
    } else {
      buffer = data;
    }
#else
    buffer = data;
#endif
    _ConnectedIOProxy->clear();
    return &buffer;
#else    
    String* pResData = new String(data);
    _ConnectedIOProxy->clear();
    return pResData;
  // const char* dataStr = data.c_str();
  // int dataLen = strlen(dataStr);
  // char* resDataStr = (char*) malloc(dataLen + 1);
  // strcpy(resDataStr, dataStr);
  // _ConnectedIOProxy->clear()
  // return resDataStr;
#endif
}



void __SendCommand(const String& layerId, const char* command, const String* pParam1, const String* pParam2, const String* pParam3, const String* pParam4, const String* pParam5, const String* pParam6, const String* pParam7, const String* pParam8, const String* pParam9) {
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print("// *** sent");
#endif        
#ifdef SUPPORT_CONTAINER
  if (layerId != "") {
    if (layerId == CONTAINER_LAYER_ID_STR) {
      _WODDIO->print("00");  // special TX layerId for container
    } else {
      _WODDIO->print(layerId);
    }
    _WODDIO->print(".");
  }
#else
  if (layerId != "") {
    _WODDIO->print(layerId/*.c_str()*/);
    _WODDIO->print(".");
  }
#endif  
#ifdef SUPPORT_ENCODE_OPER
  if (_DDCompatibility >= 3 && !_DDDisableParamEncoding && layerId != "" && command[0] == '#') {
    char encoded[3];
    encoded[0] = 14 + ((command[1] > '9') ? ((command[1] - 'a') + 10) : (command[1] - '0'));
    encoded[1] = 14 + ((command[2] > '9') ? ((command[2] - 'a') + 10) : (command[2] - '0'));
    encoded[2] = 0;
    _WODDIO->print(encoded);
  } else { 
    _WODDIO->print(command);
    if (pParam1 != NULL) {
      _WODDIO->print(":");
    }
  }   
#else
  _WODDIO->print(command);
  #ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" ...");
  #endif        
  if (pParam1 != NULL) {
  #ifdef DD_DEBUG_SEND_COMMAND          
    Serial.print(" [1|");
    Serial.print(*pParam1);
    Serial.print(" |]");
  #endif        
    _WODDIO->print(":");
  }
#endif
  if (pParam1 != NULL) {
    _WODDIO->print(*pParam1/*pParam1->c_str()*/);
    if (pParam2 != NULL) {
      _WODDIO->print(",");
      _WODDIO->print(*pParam2/*pParam2->c_str()*/);
      if (pParam3 != NULL) {
        _WODDIO->print(",");
        _WODDIO->print(*pParam3/*pParam3->c_str()*/);
        if (pParam4 != NULL) {
          _WODDIO->print(",");
          _WODDIO->print(*pParam4/*pParam4->c_str()*/);
          if (pParam5 != NULL) {
            _WODDIO->print(",");
            _WODDIO->print(*pParam5/*pParam5->c_str()*/);
            if (pParam6 != NULL) {
              _WODDIO->print(",");
              _WODDIO->print(*pParam6/*pParam6->c_str()*/);
              if (pParam7 != NULL) {
                _WODDIO->print(",");
                _WODDIO->print(*pParam7/*pParam7->c_str()*/);
                if (pParam8 != NULL) {
                  _WODDIO->print(",");
                  _WODDIO->print(*pParam8/*pParam8->c_str()*/);
                  if (pParam9 != NULL) {
                    _WODDIO->print(",");
                    _WODDIO->print(*pParam9/*pParam9->c_str()*/);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" COMMAND ");
#endif        
  _WODDIO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WODDIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
#ifdef DEBUG_ECHO_COMMAND
  _DDIO->print("// ");
  _DDIO->print(command);
  _DDIO->print("\n");
  _DDIO->flush();
#endif  
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.println(command);
#endif        
}  
void __SendComment(const char* comment, bool isError = false) {
  if (isError) {
#ifdef SUPPORT_DEBUG_INTERFACE
      if (_DebugInterface != NULL) {
        _DebugInterface->logError(comment);
      }
#endif
  }
  _WODDIO->print("//");
  if (isError) {
    _WODDIO->print("X");
  }
  _WODDIO->print(" ");
  _WODDIO->print(comment);
  _WODDIO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WODDIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
}
void __SendErrorComment(const char* comment) {
  __SendComment(comment, true);
}
#ifdef DEBUG_MISSING_ENDPOINT_C  
void __SendComment(const String& comment, bool isError = false) {
  Serial.print("//");
  if (isError) {
    Serial.print("X");
  }
  Serial.print(" <<");
  // Serial.print(String(comment.length()).c_str());
  // Serial.print("/");
  Serial.print(comment.length());
  Serial.print(">> ");
  int len = comment.length();
  for (int i = 0; i < len; i++) {
    _WODDIO->write(comment.charAt(i));
  }
  _WODDIO->print(" --\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WODDIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
}
#else
inline void __SendComment(const String& comment, bool isError = false) {
  __SendComment(comment.c_str(), isError);
}
#endif  
void _HandleFeedback();
void _SendCommand(const String& layerId, const char* command, const String* pParam1 = NULL, const String* pParam2 = NULL, const String* pParam3 = NULL, const String* pParam4 = NULL, const String* pParam5 = NULL, const String* pParam6 = NULL, const String* pParam7 = NULL, const String* pParam8 = NULL, const String* pParam9 = NULL) {
  bool alreadySendingCommand = _SendingCommand;  // not very accurate
  _SendingCommand = true;

#ifdef SUPPORT_DEBUG_INTERFACE
  if (_DebugInterface != NULL) {
    _DebugInterface->logSendCommand(1);
  }
#endif
// #ifdef DEBUG_WITH_LED
//   int debugLedPin = _DebugLedPin;
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, HIGH);
//   }
// #endif   

  if (command != NULL) {
    __SendCommand(layerId, command, pParam1, pParam2, pParam3, pParam4, pParam5, pParam6, pParam7, pParam8, pParam9);
  }

#ifdef ENABLE_FEEDBACK
  if (!alreadySendingCommand) {
    _HandleFeedback();
  }
#endif

#ifdef SUPPORT_DEBUG_INTERFACE
  if (_DebugInterface != NULL) {
    _DebugInterface->logSendCommand(0);
  }
#endif  
// #ifdef DEBUG_WITH_LED
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, LOW);
//   }  
// #endif

  _SendingCommand = false;
}
void __SendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
//Serial.println("//&&" + specialData);
  _WODDIO->print("%%>");
  _WODDIO->print(specialType);
  _WODDIO->print(".");
  if (specialId != "") {
    _WODDIO->print(specialId);
    if (specialCommand != NULL) {
      _WODDIO->print(":");
      _WODDIO->print(specialCommand);
    }
    _WODDIO->print(">");
  }
    if (specialData != "") {
      _WODDIO->print(specialData);
    }
  _WODDIO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WODDIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
}
int __FillZeroCompressedBytes(const uint8_t *bytes, int byteCount, uint8_t *toBytes, bool readFromProgramSpace) {
  int compressedByteCount = 0;
  uint8_t zeroCount = 0;
  for (int i = 0; i < byteCount; i++) {
    bool isLast = i == (byteCount - 1);
    uint8_t b;
    if (readFromProgramSpace) {
      b = pgm_read_byte(bytes + i);
    } else {
      b = bytes[i];
    }
    bool isZero = b == 0;
    if (isZero) {
      zeroCount++;
    }
    if (!isZero || zeroCount == 120 || isLast) {
      if (zeroCount > 0) {
        if (toBytes != NULL) {
          if (readFromProgramSpace) {
            _DDIO->write(0);
            _DDIO->write(zeroCount);
            compressedByteCount += 2;
          } else {
            toBytes[compressedByteCount++] = 0;
            toBytes[compressedByteCount++] = zeroCount;
          }
        } else {
          compressedByteCount += 2;
        }
        zeroCount = 0;
      }
      if (!isZero) {
        if (toBytes != NULL) {
          if (readFromProgramSpace) {
            _DDIO->write(b);
            compressedByteCount += 1;
          } else {
            toBytes[compressedByteCount++] = b;
          }
        } else {
          compressedByteCount += 1;
        }
      }
    }
  }
  return compressedByteCount;
}
void __SendByteArrayPortion(const char* bytesNature, const uint8_t *bytes, int byteCount, char compressMethod) {
  bool readFromProgramSpace = false;
  if (compressMethod == DD_PROGRAM_SPACE_COMPRESS_BA_0) {
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
    readFromProgramSpace = true;
#endif
    compressMethod = DD_COMPRESS_BA_0;
  }
  if (_DDCompatibility < 3) {
    compressMethod = 0;
  }
  int compressedByteCount = -1;
  if (compressMethod == DD_COMPRESS_BA_0) {
    //compressedByteCount = __CountZeroCompressedBytes(bytes, byteCount, false);
    compressedByteCount = __FillZeroCompressedBytes(bytes, byteCount, NULL, readFromProgramSpace);
    if (compressedByteCount > byteCount) {
      compressMethod = 0;
      compressedByteCount = -1;
    }
  }
  _DDIO->print("|bytes|>");
  if (_DDCompatibility >= 5 && bytesNature != NULL) {
//Serial.print("*** BYTES NATURE: ");
//Serial.println(bytesNature);    
    _DDIO->print(bytesNature);
    _DDIO->print("#");
  }
  _DDIO->print(String(byteCount));
  if (compressedByteCount != -1) {
    _DDIO->print("@0>");
    _DDIO->print(String(compressedByteCount));
  }
  _DDIO->print(":");
  if (true) {
    if (compressedByteCount != -1) {
      //__CountZeroCompressedBytes(bytes, byteCount, true);
      if (readFromProgramSpace) {
        uint8_t dummy;
        __FillZeroCompressedBytes(bytes, byteCount, &dummy, true);
      } else {
        uint8_t *compressedBytes = new uint8_t[compressedByteCount];
        __FillZeroCompressedBytes(bytes, byteCount, compressedBytes, false);
        _DDIO->write(compressedBytes, compressedByteCount);
        delete[] compressedBytes;  // was just delete compressedBytes 
      }
    } else {
      if (readFromProgramSpace) {
        for (int i = 0; i < byteCount; i++) {
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
          _DDIO->write(pgm_read_byte(bytes[i]));
#else          
          _DDIO->write(bytes[i]);
#endif          
        }
      } else {
        _DDIO->write(bytes, byteCount);
      }
    }
  } else {
    for (int i = 0; i < byteCount; i++) {
      uint8_t b = bytes[i];
      _DDIO->write(b);
    }
  }
  if (FLUSH_AFTER_SENT_COMMAND) {
    _DDIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
}
// void __SendByteArrayPortionForTunnel(const String& tunnelId, const uint8_t *bytes, int byteCount, char compressMethod) {
//   String bytesId = String("tunnel:") + tunnelId;
//   __SendByteArrayPortion(bytesId, bytes, byteCount, compressMethod);
// }
void _SendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
  bool alreadySendingCommand = _SendingCommand;  // not very accurate
  _SendingCommand = true;
// #ifdef DEBUG_WITH_LED
//   int debugLedPin = _DebugLedPin;
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, HIGH);
//   }
// #endif   
  __SendSpecialCommand(specialType, specialId, specialCommand, specialData);
  if (!alreadySendingCommand) {
    _HandleFeedback();
  }
// #ifdef DEBUG_WITH_LED
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, LOW);
//   }  
// #endif
  _SendingCommand = false;
}


// bool _CanLogToSerial() {
//   if (!_ConnectedFromSerial || !_Connected) {
//     return true;
//   } else {
//     return false;
//   }
// }
// inline void _LogToSerial(const String& logLine) {
//   if (!_ConnectedFromSerial || !_Connected) {
//     Serial.println(logLine);  // in case not connected ... hmm ... assume ... Serial.begin() called
//   } else {
//     _SendCommand("", ("// " + logLine).c_str());
//   }
// }


// #ifdef READ_BUFFER_USE_BUFFER
// String _ReadFeedbackBuffer;
// #endif

void _HandleFeedback() {
  if (!_HandlingFeedback) {
    _HandlingFeedback = true;
#ifdef READ_BUFFER_USE_BUFFER
  String* pFeedback = _ReadFeedback(_ReadFeedbackBuffer);
#else
    String buffer;
    String* pFeedback = _ReadFeedback(buffer);
#endif
// #ifdef READ_BUFFER_USE_BUFFER
//     String buffer;
//    String* pFeedback = _ReadFeedback(buffer);
// #endif
    if (pFeedback != NULL) {
#ifdef DEBUG_SHOW_FEEDBACK
      Serial.print("FB: ");
      Serial.println(*pFeedback);
#endif      
#ifdef MORE_KEEP_ALIVE
      // keep alive whenever received something
      _ConnectedIOProxy->keepAlive();
#endif        
      if (*(pFeedback->c_str()) == '<') {
        if (pFeedback->length() == 1) {
#ifndef MORE_KEEP_ALIVE
          // keep alive
          _ConnectedIOProxy->keepAlive();
#endif   
        }
        else {
#ifdef SUPPORT_TUNNEL
// Serial.println("LT-[" + *pFeedback + "] (" + pFeedback->length() + ")");
          if (pFeedback->startsWith("<lt.")) {
#if defined(DEBUG_TUNNEL_RESPONSE_C)               
__SendComment("LT++++fb--" + *pFeedback);
#endif
            int idx = pFeedback->indexOf('<', 4);
            //Serial.println("LT+" + String(idx));
            if (idx != -1) {
              //Serial.println("LT++" + String(idx));
              String tid = pFeedback->substring(4, idx);
              String data = pFeedback->substring(idx + 1);
              //const String* pData = &data;
              bool final = false;
              idx = tid.indexOf(':');
              //const char *pCommand = NULL;
              //Serial.println("LT+++" + String(idx));
              if (idx != -1) {
                String command = tid.substring(idx + 1);
                tid = tid.substring(0, idx);
#ifdef DEBUG_SHOW_FEEDBACK
//Serial.println("LT-command" + data);
Serial.println("LT-command:[" + command + "]");
#endif
                if (command == "final") {
                  final = true;
                } else if (command == "error") {
                  final = true;
                  data = "";
                } else {
                  data = "???" + command + "???";
                }
              }/* else {
                data += "\n";
              }*/
              int lid = _LayerIdToLid(tid);
              DDTunnel* pTunnel = (DDTunnel*) _DDLayerArray[lid];
              if (pTunnel != NULL) {
#ifdef DEBUG_TUNNEL_RESPONSE                
Serial.println("LT++++ [" + data + "] (" + data.length() + ") - final:" + String(final));
#endif
#ifdef DEBUG_TUNNEL_RESPONSE_C                
__SendComment("LT++++" + data + " - final:" + String(final));
#endif
#ifdef FEEDBACK_SUPPORT_BYTES
                uint8_t* fbBytes = _ReadFeedbackDataBuffer.pBytes;
                pTunnel->handleInput(data, fbBytes, final);
                _ReadFeedbackDataBuffer.pBytes = NULL;
#else
                pTunnel->handleInput(data, NULL, final);
#endif                
              }
            }
          }
#endif          
        }
        pFeedback = NULL;
      }
    }
    if (pFeedback != NULL) {
#ifdef DEBUG_ECHO_FEEDBACK
      if (_DebugEnableEchoFeedback) {
          _SendCommand("", ("// feedback -- " + *pFeedback).c_str());
        if (!_ConnectedFromSerial) {
          Serial.print("// FB -- ");
          Serial.print(*pFeedback);
          Serial.print("\n");
          Serial.flush();
        }
      }
#endif
#ifdef STORE_LAYERS
      int bufLen = pFeedback->length() + 1;
      char buf[bufLen];
      pFeedback->toCharArray(buf, bufLen);
      bool ok = false;
      int lid = -1;
      DDFeedbackType type = DDFeedbackType::CLICK;
      int16_t x = 0;
      int16_t y = 0;
      char* pText = NULL;      
      char* token = strtok(buf, ".");
      if (token != NULL) {
        lid = _LayerIdToLid(token);
        token = strtok(NULL, ":");
      }
      if (token != NULL) {
        //Serial.println("FBT:[" + String(token) + "]");
        if (*token >= '0' && *token <= '9' && *(token + 1) == 0) {
          x = *token - '0';
          y = 0;
          ok = true;  // got x and y
          token = strtok(NULL, "");  // want the rest
        } else {
          if (strcmp(token, "longpress") == 0 || strcmp(token, "L") == 0) {
            type = LONGPRESS;
          } else if (strcmp(token, "doubleclick") == 0 || strcmp(token, "D") == 0) {
            type = DOUBLECLICK;
          } else if (strcmp(token, "move") == 0 || strcmp(token, "M") == 0) {
            type = MOVE;
          } else if (strcmp(token, "up") == 0 || strcmp(token, "u") == 0) {
            type = UP;
          } else if (strcmp(token, "down") == 0 || strcmp(token, "d") == 0) {
            type = DOWN;
          } else if (strcmp(token, "custom") == 0 || strcmp(token, "c") == 0) {
            type = CUSTOM;
          } 
          token = strtok(NULL, ",");
        }
      } else {
        ok = true;
      }
      if (!ok) {
        // getting x and y
        if (token != NULL) {
          x = atoi(token);
          token = strtok(NULL, ",");
        }
        if (token != NULL) {
          y = atoi(token);
          ok = true;
          token = strtok(NULL, "");  // want the rest
        }
      }
      if (token != NULL) {
        pText = token;
      }
      if (ok) {
        if (lid < 0 || lid >= _NextLid) {
          ok = false;
        }
      }
      if (ok) {
#ifdef SUPPORT_TUNNEL
        DDLayer* pLayer = (DDLayer*) _DDLayerArray[lid];
#else
        DDLayer* pLayer = _DDLayerArray[lid];
#endif
        if (pLayer != NULL) {
#ifdef ENABLE_FEEDBACK          
          DDFeedbackHandler handler = pLayer->getFeedbackHandler();
          if (handler != NULL) {
            DDFeedback feedback;
            feedback.x = x;
            feedback.y = y;
            if (pText != NULL) {
              feedback.text = String(pText);
            }
            handler(pLayer, type/*CLICK*/, feedback);
            //_SendCommand("", ("// feedback (" + String(lid) + ") -- " + *pFeedback).c_str());
          } else {
            DDFeedbackManager *pFeedbackManager = pLayer->getFeedbackManager();
            if (pFeedbackManager != NULL) {
              pFeedbackManager->pushFeedback(type, x, y, pText);
            }
          }
#endif
        }
      }
#endif  
#ifndef READ_BUFFER_USE_BUFFER
      delete pFeedback;
#endif      
    }
    _HandlingFeedback = false;
    if (YIELD_AFTER_HANDLE_FEEDBACK) {
      YIELD();
    }    
  }
}

inline void _Delay(unsigned long ms) {
#ifdef ENABLE_FEEDBACK
  unsigned long delayMillis = ms;
	unsigned long startMillis = millis();
  while (true) {
    _HandleFeedback();
    long remainMillis = delayMillis - (millis() - startMillis);
    if (remainMillis > 20) {
      delay(20);
    } else {
      if (remainMillis >= 1) {
        delay(remainMillis);
      } else {
        yield();
      }
      break;
    }
  }
#else
  delay(ms);
#endif
}

// inline void _OLD_Delay(unsigned long ms) {
// #ifdef ENABLE_FEEDBACK
//   unsigned long delayMicros = ms * 1000;
// 	unsigned long start = micros();
//   while (true) {
//     _HandleFeedback();
//     unsigned long remain = delayMicros - (micros() - start);
//     if (remain > 20000) {
//       delay(20);
//     } else {
//       if (remain > 1000) {
//         delay(remain / 1000);
//       }
//       break;
//     }
//   }
// #else
//   delay(ms);
// #endif
// }

inline void _Yield() {
#ifdef ENABLE_FEEDBACK
  _HandleFeedback();
#endif
  yield();
}



inline void _sendCommand0(const String& layerId, const char *command) {
  _SendCommand(layerId, command);
}  
inline void _sendCommand1(const String& layerId, const char *command, const String& param1) {
  _SendCommand(layerId, command, &param1);
}  
inline void _sendCommand2(const String& layerId, const char *command, const String& param1, const String& param2) {
  _SendCommand(layerId, command, &param1, &param2);
}  
inline void _sendCommand3(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3) {
  _SendCommand(layerId, command, &param1, &param2, &param3);
}  
inline void _sendCommand4(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4);
}
inline void _sendCommand5(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5);
}
inline void _sendCommand6(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6);
}
inline void _sendCommand7(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6, const String& param7) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6, &param7);
}
inline void _sendCommand8(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6, const String& param7, const String& param8) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6, &param7, &param8);
}
inline void _sendCommand9(const String& layerId, const char *command, const String& param1, const String& param2, const String& param3, const String& param4, const String& param5, const String& param6, const String& param7, const String& param8, const String& param9) {
  _SendCommand(layerId, command, &param1, &param2, &param3, &param4, &param5, &param6, &param7, &param8, &param9);
}
inline void _sendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
#ifdef SUPPORT_TUNNEL
  _SendSpecialCommand(specialType, specialId, specialCommand, specialData);
#endif
}
void _setLTBufferCommand(const String& data) {
#ifdef SUPPORT_TUNNEL
  if (true) {
    int dataLen = data.length();
    int i = 0;
    while (i < dataLen) {
      int len = dataLen - i;
      if (len > TL_BUFFER_DATA_LEN) {
        len = TL_BUFFER_DATA_LEN;
      }
      int j = i + len;
      String partData = data.substring(i, j);
      _sendSpecialCommand("ltbuf", "", NULL, partData);
      i = j;
    }
  } else {
    _sendSpecialCommand("ltbuf", "", NULL, data);
  }
#endif
}
void _sendByteArrayAfterCommand(const uint8_t *bytes, int byteCount, char compressMethod = 0) {
  __SendByteArrayPortion(NULL, bytes, byteCount, compressMethod);
  _sendCommand0("", C_KAL);  // send a "keep alive" command to make sure and new-line is sent
}
void _sendByteArrayAfterCommandChunked(const String& bytesId, const uint8_t *bytes, int byteCount, bool isFinalChunk = false) {
  char compressMethod = 0;  // assume compressionMethod 0
  String bytesNature = String(isFinalChunk ? "|" : ".") + "&" + bytesId;
  __SendByteArrayPortion(bytesNature.c_str(), bytes, byteCount, compressMethod);
  _sendCommand0("", C_KAL);  // send a "keep alive" command to make sure and new-line is sent
}





}




using namespace DDImpl;


#ifndef ENABLE_FEEDBACK
void _sendFeedbackDisableComment() {
#if defined(DD_NO_FEEDBACK)
  __SendErrorComment("\"feedback\" disabled due to DD_NO_FEEDBACK");
#endif
}
#endif
#ifndef SUPPORT_TUNNEL
void _sendTunnelDisabledComment() {
#ifdef DD_NO_TUNNEL
  __SendErrorComment("\"tunnel\" disabled due to DD_NO_TUNNEL");
#endif  
}
#endif


// DDFeedbackManager::DDFeedbackManager(/*int8_t bufferSize*/) {
//   this->nextArrayIdx = 0;
//   this->validArrayIdx = 0;
// }
// const DDFeedback* DDFeedbackManager::getFeedback() {
// #ifdef ENABLE_FEEDBACK
//   if (nextArrayIdx == validArrayIdx) return NULL;
//   const DDFeedback* pFeedback = &feedbackArray[validArrayIdx];
//   validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
//   return pFeedback;
// #else
//   return NULL;
// #endif    
// }
// void DDFeedbackManager::pushFeedback(DDFeedbackType type, int16_t x, int16_t y, const char* pText) {
// #ifdef ENABLE_FEEDBACK
//   feedbackArray[nextArrayIdx].type = type;
//   feedbackArray[nextArrayIdx].x = x;
//   feedbackArray[nextArrayIdx].y = y;
//   feedbackArray[nextArrayIdx].text = pText != NULL ? pText : "";
//   nextArrayIdx = (nextArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
//   if (nextArrayIdx == validArrayIdx)
//     validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
// #endif
// }


DDLayer::DDLayer(int8_t layerId)/*: DDObject(DD_OBJECT_TYPE_LAYER)*/ {
  this->objectType = DD_OBJECT_TYPE_LAYER;
  this->layerId = String(layerId);
#ifdef ENABLE_FEEDBACK          
  this->pFeedbackManager = NULL;
  this->feedbackHandler = NULL;
#endif
}
DDLayer::~DDLayer() {
#ifdef DD_DEBUG_BASIC  
  if (_CanLogToSerial()) Serial.println("--- delete DDLayer");
#endif
  _PreDeleteLayer(this);
#ifdef ENABLE_FEEDBACK          
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
#endif
} 
void DDLayer::visible(bool visible) {
  _sendCommand1(layerId, C_visible, TO_BOOL(visible));
}
void DDLayer::transparent(bool transparent) {
  _sendCommand1(layerId, C_transparent, TO_BOOL(transparent));
}
void DDLayer::disabled(bool disabled) {
  _sendCommand1(layerId, C_disabled, TO_BOOL(disabled));
}
void DDLayer::opacity(int opacity) {
  _sendCommand1(layerId, C_opacity, String(opacity));
}
void DDLayer::alpha(int alpha) {
  _sendCommand1(layerId, C_alpha, String(alpha));
}
void DDLayer::blend(const String& color, int alpha, const String& mode) {
  _sendCommand3(layerId, C_blend, color, String(alpha), mode);
}
void DDLayer::noblend() {
  _sendCommand0(layerId, C_noblend);
}
// void DDLayer::opacity(int opacity) {
//   if (_DDCompatibility >= 2) {
//       setAlpha(opacity);
//   } else {
//     _sendCommand1(layerId, C_opacity, String(opacity));
//   }
// }
void DDLayer::border(float size, const String& color, const String& shape, float extraSize) {
  if (IS_FLOAT_ZERO(extraSize)) {
    _sendCommand3(layerId, C_border, TO_NUM(size), color, shape);
  } else {
    _sendCommand4(layerId, C_border, TO_NUM(size), color, shape, TO_NUM(extraSize));
  }
}
void DDLayer::noBorder() {
  _sendCommand0(layerId, C_border);
}
void DDLayer::padding(float size) {
  _sendCommand1(layerId, C_padding, TO_NUM(size));
}
void DDLayer::padding(float left, float top, float right, float bottom) {
  _sendCommand4(layerId, C_padding, TO_NUM(left), TO_NUM(top), TO_NUM(right), TO_NUM(bottom));
}
void DDLayer::noPadding() {
  _sendCommand0(layerId, C_padding);
}
void DDLayer::margin(float size) {
  _sendCommand1(layerId, C_margin, TO_NUM(size));
}
void DDLayer::margin(float left, float top, float right, float bottom) {
  _sendCommand4(layerId, C_margin, TO_NUM(left), TO_NUM(top), TO_NUM(right), TO_NUM(bottom));
}
void DDLayer::noMargin() {
  _sendCommand0(layerId, C_margin);
}
void DDLayer::clear() {
  _sendCommand0(layerId, C_clear);
}
// void DDLayer::backgroundColor(long color) {
//   _sendCommand1(layerId, "bgcolor", HEX_COLOR(color));
// }
void DDLayer::backgroundColor(const String& color, int opacity) {
  if (opacity < 100) {
    _sendCommand2(layerId, C_bgcolor, color, String(opacity));
  } else {
    _sendCommand1(layerId, C_bgcolor, color);
  }
}
void DDLayer::noBackgroundColor() {
  _sendCommand0(layerId, C_nobgcolor);
}
void DDLayer::backgroundImage(const String& backgroundImageName, const String& drawBackgroundOptions, int refImageWidth) {
  if (refImageWidth > 0) {
    _sendCommand3(layerId, C_bgimg, backgroundImageName, drawBackgroundOptions, String(refImageWidth));
  } else {
    _sendCommand2(layerId, C_bgimg, backgroundImageName, drawBackgroundOptions);
  }
}
/// set no layer background image
void DDLayer::noBackgroundImage() {
  _sendCommand0(layerId, C_nobgimg);
}
void DDLayer::exportAsBackgroundImage(bool replace, bool noDrawBackground, int exportAsWidth) {
  if (exportAsWidth > 0) {
    _sendCommand3(layerId, C_explayerasbgimg, TO_BOOL(replace), TO_BOOL(noDrawBackground), String(exportAsWidth));
  } else {
    _sendCommand2(layerId, C_explayerasbgimg, TO_BOOL(replace), TO_BOOL(noDrawBackground));
  }
}
void DDLayer::animateBackgroundImage(float fps, bool reset, const String& options) {
  _sendCommand3(layerId, C_anibgimg, TO_NUM(fps), TO_BOOL(reset), options);  
}
void DDLayer::stopAnimateBackgroundImage(bool reset) {
  _sendCommand1(layerId, C_stopanibgimg, TO_BOOL(reset));  
}
void DDLayer::flash() {
  _sendCommand0(layerId, C_flash);
}
void DDLayer::flashArea(int x, int y) {
  _sendCommand2(layerId, C_flasharea, String(x), String(y));
}
void DDLayer::explicitFeedback(int16_t x, int16_t y, const String& text, DDFeedbackType type, const String& option) {
  String tp = "";
  if (type == DDFeedbackType::CLICK) {
    tp = "CLICK";
  } else if (type == DDFeedbackType::LONGPRESS) {
    tp = "LONGPRESS";
  } else if (type == DDFeedbackType::DOUBLECLICK) {
    tp = "DOUBLECLICK";
  } else if (type == DDFeedbackType::MOVE) {
    tp = "MOVE";
  } else if (type == DDFeedbackType::UP) {
    tp = "UP";
  } else if (type == DDFeedbackType::DOWN) {
    tp = "DOWN";
  } else if (type == DDFeedbackType::CUSTOM) {
    tp = "CUSTOM";
  }
  _sendCommand5(layerId, C_explicitfeedback, String(x), String(y), tp, option, text);
}

// void DDLayer::writeComment(const String& comment) {
//   _sendCommand0("", ("// " + layerId + ": " + comment).c_str());
// }
void DDLayer::_enableFeedback() {
#ifdef ENABLE_FEEDBACK  
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
  pFeedbackManager = new DDFeedbackManager(/*FEEDBACK_BUFFER_SIZE + 1*/);
#else
  _sendFeedbackDisableComment();
#endif
}
void DDLayer::enableFeedback(const String& autoFeedbackMethod, const String& allowFeedbackTypes) {
#ifdef ENABLE_FEEDBACK  
  if (_DDCompatibility >= 10) {
    _sendCommand3(layerId, C_feedback, TO_BOOL(true), autoFeedbackMethod, allowFeedbackTypes);
  } else {
    _sendCommand2(layerId, C_feedback, TO_BOOL(true), autoFeedbackMethod);
  }
  feedbackHandler = NULL;
  _enableFeedback();
  // if (pFeedbackManager != NULL)
  //   delete pFeedbackManager;
  // pFeedbackManager = new DDFeedbackManager(FEEDBACK_BUFFER_SIZE + 1);  // need 1 more slot
#else
  _sendFeedbackDisableComment();
#endif
}
void DDLayer::disableFeedback() {
#ifdef ENABLE_FEEDBACK          
  _sendCommand1(layerId, C_feedback, TO_BOOL(false));
  feedbackHandler = NULL;
  if (pFeedbackManager != NULL) {
    delete pFeedbackManager;
    pFeedbackManager = NULL;
  }
#endif
}
const DDFeedback* DDLayer::getFeedback() {
#ifdef ENABLE_FEEDBACK          
  //yield();
  _HandleFeedback();
  return pFeedbackManager != NULL ? pFeedbackManager->getFeedback() : NULL;
#else
  return NULL;
#endif
}
void DDLayer::setFeedbackHandler(DDFeedbackHandler handler, const String& autoFeedbackMethod, const String& allowFeedbackTypes) {
#ifdef ENABLE_FEEDBACK          
  bool enable = handler != NULL;
  if (_DDCompatibility >= 10) {
    _sendCommand3(layerId, C_feedback, TO_BOOL(true), autoFeedbackMethod, allowFeedbackTypes);
  } else {
    _sendCommand2(layerId, C_feedback, TO_BOOL(enable), autoFeedbackMethod);
  }
  feedbackHandler = handler;
  if (pFeedbackManager != NULL) {
    delete pFeedbackManager;
    pFeedbackManager = NULL;
  }
#else
  _sendFeedbackDisableComment();
#endif
}

void MultiLevelDDLayer::addLevel(const String& levelId, float width, float height, bool switchToIt) {
  if (IS_FLOAT_ZERO(width) && IS_FLOAT_ZERO(height)) {
    if (switchToIt) {
      _sendCommand2(layerId, C_addlevel, levelId, TO_BOOL(switchToIt));
    } else {
      _sendCommand1(layerId, C_addlevel, levelId);
    }
  } else {
    _sendCommand4(layerId, C_addlevel, levelId, TO_NUM(width), TO_NUM(height), TO_BOOL(switchToIt));
  }
}
void MultiLevelDDLayer::addTopLevel(const String& levelId, float width, float height, bool switchToIt) {
  if (IS_FLOAT_ZERO(width) && IS_FLOAT_ZERO(height)) {
    if (switchToIt) {
      _sendCommand2(layerId, C_addtoplevel, levelId, TO_BOOL(switchToIt));
    } else {
      _sendCommand1(layerId, C_addtoplevel, levelId);
    }
  } else {
    _sendCommand4(layerId, C_addtoplevel, levelId, TO_NUM(width), TO_NUM(height), TO_BOOL(switchToIt));
  }
}
void MultiLevelDDLayer::switchLevel(const String& levelId, bool addIfMissing) {
  _sendCommand2(layerId, C_switchlevel, levelId, TO_BOOL(addIfMissing));
}
void MultiLevelDDLayer::pushLevel() {
  _sendCommand0(layerId, C_pushlevel);
}
void MultiLevelDDLayer::pushLevelAndSwitchTo(const String& switchTolevelId, bool addIfMissing) {
  _sendCommand2(layerId, C_pushlevel, switchTolevelId, TO_BOOL(addIfMissing));
}
void MultiLevelDDLayer::popLevel() {
  _sendCommand0(layerId, C_poplevel);
}
void MultiLevelDDLayer::levelOpacity(int opacity) {
  _sendCommand1(layerId, C_levelopacity, String(opacity));  
}
void MultiLevelDDLayer::levelTransparent(bool transparent) {
  _sendCommand1(layerId, C_leveltransparent, TO_BOOL(transparent));  
}
void MultiLevelDDLayer::setLevelAnchor(float x, float y, long reachInMillis) {
  if (reachInMillis > 0) {
    _sendCommand3(layerId, C_setlevelanchor, TO_NUM(x), TO_NUM(y), String(reachInMillis));
  } else {
    _sendCommand2(layerId, C_setlevelanchor, TO_NUM(x), TO_NUM(y));
  }
}
void MultiLevelDDLayer::moveLevelAnchorBy(float byX, float byY, long reachInMillis) {
  if (reachInMillis > 0) {
    _sendCommand3(layerId, C_movelevelanchorby, TO_NUM(byX), TO_NUM(byY), String(reachInMillis));
  } else {
    _sendCommand2(layerId, C_movelevelanchorby, TO_NUM(byX), TO_NUM(byY));
  }
}
void MultiLevelDDLayer::registerLevelBackground(const String& backgroundId, const String& backgroundImageName, const String& drawBackgroundOptions) {
  _sendCommand3(layerId, C_reglevelbg, backgroundId, backgroundImageName, drawBackgroundOptions);  
}
void MultiLevelDDLayer::exportLevelAsRegisteredBackground(const String& backgroundId, bool replace) {
  _sendCommand2(layerId, C_explevelasregbg, backgroundId, TO_BOOL(replace));  
}
void MultiLevelDDLayer::setLevelBackground(const String& backgroundId, const String& backgroundImageName, const String& drawBackgroundOptions) {
  if (backgroundImageName == "") {
    _sendCommand1(layerId, C_setlevelbg, backgroundId);  
  } else {
    _sendCommand3(layerId, C_setlevelbg, backgroundId, backgroundImageName, drawBackgroundOptions);  
  }
}
void MultiLevelDDLayer::setLevelNoBackground() {
    _sendCommand0(layerId, C_setlevelnobg);  
}
void MultiLevelDDLayer::animateLevelBackground(float fps, bool reset, const String& options) {
  _sendCommand3(layerId, C_anilevelbg, TO_NUM(fps), TO_BOOL(reset), options);  
}
void MultiLevelDDLayer::stopAnimateLevelBackground(bool reset) {
    _sendCommand1(layerId, C_stopanilevelbg, TO_BOOL(reset));  
}
void MultiLevelDDLayer::reorderLevel(const String& levelId, const String& how) {
  _sendCommand2(layerId, C_reordlevel, levelId, how);  
}
void MultiLevelDDLayer::exportLevelsAsImage(const String& imageFileName, bool cacheItNotSave) {
  _sendCommand2(layerId, C_explevelsasimg, imageFileName, TO_BOOL(cacheItNotSave));  
}
void MultiLevelDDLayer::deleteLevel(const String& levelId) {
  _sendCommand1(layerId, C_dellevel, levelId);  
}

void MbDDLayer::showIcon(MbIcon icon) {
  _sendCommand1(layerId, "shi", String(icon));
}
void MbDDLayer::showArrow(MbArrow arrow) {
  _sendCommand1(layerId, "sha", String(arrow));
}
void MbDDLayer::showNumber(int num) {
  _sendCommand1(layerId, "shn", String(num));
}
void MbDDLayer::showString(const String& str) {
  _sendCommand1(layerId, "shs", str);
}
void MbDDLayer::plot(int x, int y) {
  _sendCommand2(layerId, "pl", String(x), String(y));
}
void MbDDLayer::unplot(int x, int y) {
  _sendCommand2(layerId, "upl", String(x), String(y));
}
void MbDDLayer::toggle(int x, int y) {
  _sendCommand2(layerId, "tggl", String(x), String(y));
}
void MbDDLayer::showLeds(const String& ledPattern) {
  _sendCommand1(layerId, "shledpat", ledPattern);
}
// void MicroBitLayer::clearScreen() {
//   _sendCommand0(layerId, "cs");
// }
// void MbDDLayer::ledColor(long color) {
//   _sendCommand1(layerId, "ledc", HEX_COLOR(color));
// }
void MbDDLayer::ledColor(const String& color) {
  _sendCommand1(layerId, "ledc", color);
}
MbImage* MbDDLayer::createImage(const String& ledPattern) {
  int imgId = _AllocImgId();
  MbImage *pImage = new MbImage(imgId);
  _sendCommand2(layerId, "crimg", pImage->getImageId(), ledPattern);
  return pImage;
}
void MbDDLayer::releaseImage(MbImage *pImage) {
  _sendCommand1(layerId, "delimg", pImage->getImageId());
  delete pImage;
}
void MbDDLayer::showImage(MbImage *pImage, int xOff) {
  _sendCommand2(layerId, "shimg", pImage->getImageId(), String(xOff));
}
void MbDDLayer::scrollImage(MbImage *pImage, int xOff, long interval) {
  _sendCommand3(layerId, "sclimg", pImage->getImageId(), String(xOff), String(interval));
}

void TurtleDDLayer::forward(int distance, bool withPen) {
  _sendCommand1(layerId, withPen ? "fd" : "dlfd", String(distance));
}
void TurtleDDLayer::backward(int distance, bool withPen) {
  _sendCommand1(layerId, withPen ? "bk" : "dlbk", String(distance));
}
void TurtleDDLayer::leftTurn(int angle) {
  _sendCommand1(layerId, "lt", String(angle));
}
void TurtleDDLayer::rightTurn(int angle) {
  _sendCommand1(layerId, "rt", String(angle));
}
void TurtleDDLayer::home(bool withPen) {
  _sendCommand0(layerId, withPen ? "home" : "jhome");
}
void TurtleDDLayer::goTo(int x, int y, bool withPen) {
  _sendCommand2(layerId, withPen ? "goto" : "jto", String(x), String(y));
}
void TurtleDDLayer::goBy(int byX, int byY, bool withPen) {
  _sendCommand2(layerId, withPen ? "goby" : "jby", String(byX), String(byY));
}
void TurtleDDLayer::setHeading(int angle) {
  _sendCommand1(layerId, "seth", String(angle));
}
void TurtleDDLayer::penUp() {
  _sendCommand0(layerId, "pu");
}
void TurtleDDLayer::penDown() {
  _sendCommand0(layerId, "pd");
}
void TurtleDDLayer::beginFill() {
  _sendCommand0(layerId, "begin_fill");
}
void TurtleDDLayer::endFill() {
  _sendCommand0(layerId, "end_fill");
}
void TurtleDDLayer::penSize(int size) {
  _sendCommand1(layerId, C_pensize, String(size));
}
// void TurtleDDLayer::penColor(long color) {
//   _sendCommand1(layerId, "pencolor", HEX_COLOR(color));
// }
void TurtleDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, C_pencolor, color);
}
// void TurtleDDLayer::fillColor(long color) {
//   _sendCommand1(layerId, "fillcolor", HEX_COLOR(color));
// }
void TurtleDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, C_fillcolor, color);
}
void TurtleDDLayer::noFillColor() {
  _sendCommand0(layerId, C_nofillcolor);
}
void TurtleDDLayer::penFilled(bool filled) {
  _sendCommand1(layerId, "pfilled", TO_BOOL(filled));
}
void TurtleDDLayer::setTextSize(int size) {
  _sendCommand1(layerId, C_ptextsize, String(size));
}
void TurtleDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, C_ptextfont, fontName, String(size));
}
void TurtleDDLayer::dot(int size, const String& color) {
  _sendCommand2(layerId, C_dot, String(size), color);
}
void TurtleDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? C_ccircle : C_circle, String(radius));
}
void TurtleDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? C_coval : C_oval, String(width), String(height));
}
void TurtleDDLayer::arc(int width, int height, int startAngle, int sweepAngle, bool centered) {
  _sendCommand4(layerId, centered ? C_carc : C_arc, String(width), String(height), String(startAngle), String(sweepAngle));
}
void TurtleDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? C_crect : C_rect, String(width), String(height));
}
void TurtleDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, C_trisas, String(side1), String(angle), String(side2));
}
void TurtleDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, C_trisas, String(side), String(angle));
}
void TurtleDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, C_poly, String(side), String(vertexCount));
}
void TurtleDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? C_cpolyin : C_cpoly, String(radius), String(vertexCount));
}
void TurtleDDLayer::write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? C_drawtext : C_write, text);
}

void LedGridDDLayer::turnOn(int x, int y) {
  _sendCommand2(layerId, C_ledon, String(x), String(y));
}
void LedGridDDLayer::turnOff(int x, int y) {
  _sendCommand2(layerId, C_ledoff, String(x), String(y));
}
void LedGridDDLayer::toggle(int x, int y) {
  _sendCommand2(layerId, C_ledtoggle, String(x), String(y));
}
void LedGridDDLayer::turnOnEx(int x, int y, const String& onColor) {
  _sendCommand3(layerId, C_ledonex, String(x), String(y), onColor);
}
void LedGridDDLayer::bitwise(unsigned int bits, int y, bool reverse) {
  _sendCommand2(layerId, reverse ? C_bitbybit : C_bitwise, String(y), String(bits));
}
void LedGridDDLayer::bitwise2(unsigned int bits_0, unsigned int bits_1, int y, bool reverse) {
  _sendCommand3(layerId, reverse ? C_bitbybit : C_bitwise, String(y), String(bits_0), String(bits_1));
}
void LedGridDDLayer::bitwise3(unsigned int bits_0, unsigned int bits_1, unsigned int bits_2, int y, bool reverse) {
  _sendCommand4(layerId, reverse ? C_bitbybit : C_bitwise, String(y), String(bits_0), String(bits_1), String(bits_2));
}
void LedGridDDLayer::bitwise4(unsigned int bits_0, unsigned int bits_1, unsigned int bits_2, unsigned int bits_3, int y, bool reverse) {
  _sendCommand5(layerId, reverse ? C_bitbybit : C_bitwise, String(y), String(bits_0), String(bits_1), String(bits_2), String(bits_3));
}
void LedGridDDLayer::horizontalBar(int count, bool rightToLeft) {
  _sendCommand2(layerId, C_ledhoribar, String(count), TO_BOOL(rightToLeft));
}
void LedGridDDLayer::verticalBar(int count, bool bottomToTop) {
  _sendCommand2(layerId, C_ledvertbar, String(count), TO_BOOL(bottomToTop));
}
void LedGridDDLayer::horizontalBarEx(int count, int startX, const String& color) {
  _sendCommand3(layerId, C_ledhoribarex, String(count), String(startX), color);
}
void LedGridDDLayer::verticalBarEx(int count, int startY, const String& color) {
  _sendCommand3(layerId, C_ledvertbarex, String(count), String(startY), color);
}
// void LedGridDDLayer::onColor(long color) {
//   _sendCommand1(layerId, "ledoncolor", HEX_COLOR(color));
// }
void LedGridDDLayer::onColor(const String& color) {
  _sendCommand1(layerId, C_ledoncolor, color);
}
// void LedGridDDLayer::offColor(long color) {
//   _sendCommand1(layerId, "ledoffcolor", HEX_COLOR(color));
// }
void LedGridDDLayer::offColor(const String& color) {
  _sendCommand1(layerId, C_ledoffcolor, color);
}
void LedGridDDLayer::noOffColor() {
  _sendCommand0(layerId, C_ledoffcolor);
}


void LcdDDLayer::print(const String& text) {
  _sendCommand1(layerId, C_print, text);
}
void LcdDDLayer::home() {
  _sendCommand0(layerId, C_home);
}
void LcdDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, C_setcursor, String(x), String(y));
}
void LcdDDLayer::cursor() {
  _sendCommand1(layerId, C_cursor, TO_BOOL(true));
}
void LcdDDLayer::noCursor() {
  _sendCommand1(layerId, C_cursor, TO_BOOL(false));
}
void LcdDDLayer::autoscroll() {
  _sendCommand1(layerId, C_autoscroll, TO_BOOL(true));
}
void LcdDDLayer::noAutoscroll() {
  _sendCommand1(layerId, C_autoscroll, TO_BOOL(false));
}
void LcdDDLayer::display() {
  _sendCommand1(layerId, C_display, TO_BOOL(true));
}
void LcdDDLayer::noDisplay() {
  _sendCommand1(layerId, C_display, TO_BOOL(false));
}
void LcdDDLayer::scrollDisplayLeft() {
  _sendCommand0(layerId, C_scrollleft);
}
void LcdDDLayer::scrollDisplayRight() {
  _sendCommand0(layerId, C_scrollright);
}
void LcdDDLayer::writeLine(const String& text, int y, const String& align) {
  _sendCommand3(layerId, C_writeline, String(y), align, text);
}
void LcdDDLayer::writeRightAlignedLine(const String& text, int y) {
  _sendCommand3(layerId, C_writeline, String(y), "R", text);
}
void LcdDDLayer::writeCenteredLine(const String& text, int y) {
  _sendCommand3(layerId, C_writeline, String(y), "C", text);
} 
void LcdDDLayer::pixelColor(const String &color) {
  _sendCommand1(layerId, C_pixelcolor, color);
}
void LcdDDLayer::bgPixelColor(const String &color, bool sameForBackgroundColor, int backgroundOpacity) {
  _sendCommand1(layerId, C_bgpixelcolor, color);
  if (sameForBackgroundColor) {
    backgroundColor(color, backgroundOpacity);
  }
}
void LcdDDLayer::noBgPixelColor() {
  _sendCommand0(layerId, C_bgpixelcolor);
}


void SelectionBaseDDLayer::pixelColor(const String &color) {
  _sendCommand1(layerId, C_pixelcolor, color);
}
void SelectionBaseDDLayer::pixelColor(const String &color, bool selected) {
  _sendCommand2(layerId, C_pixelcolor, color, TO_BOOL(selected));
}
// void SelectionBaseDDLayer::bgPixelColor(const String &color, bool sameForBackgroundColor, int backgroundOpacity) {
//   _sendCommand1(layerId, C_bgpixelcolor, color);
//   if (sameForBackgroundColor) {
//     backgroundColor(color, backgroundOpacity);
//   }
// }
void SelectionBaseDDLayer::selectAll() {
  _sendCommand0(layerId, C_select);
}
void SelectionBaseDDLayer::deselectAll() {
  _sendCommand0(layerId, C_deselect);
}
void SelectionBaseDDLayer::highlightBorder(bool forSelected, const String& borderColor, const String& borderShape) {
  _sendCommand3(layerId, C_highlighborder, TO_BOOL(forSelected), borderColor, borderShape);
}


void SelectionDDLayer::text(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx, const String& align) {
  _sendCommand5(layerId, C_text, String(y), String(horiSelectionIdx), String(vertSelectionIdx), align, text);
}
void SelectionDDLayer::textCentered(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx) {
  _sendCommand5(layerId, C_text, String(y), String(horiSelectionIdx), String(vertSelectionIdx), "C", text);
}
void SelectionDDLayer::textRightAligned(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx) {
  _sendCommand5(layerId, C_text, String(y), String(horiSelectionIdx), String(vertSelectionIdx), "R", text);
}
void SelectionDDLayer::unselectedText(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx, const String& align) {
  _sendCommand5(layerId, C_unselectedtext, String(y), String(horiSelectionIdx), String(vertSelectionIdx), align, text);
}
void SelectionDDLayer::unselectedTextCentered(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx) {
  _sendCommand5(layerId, C_unselectedtext, String(y), String(horiSelectionIdx), String(vertSelectionIdx), "C", text);
}
void SelectionDDLayer::unselectedTextRightAligned(const String& text, int y, int horiSelectionIdx, int vertSelectionIdx) {
  _sendCommand5(layerId, C_unselectedtext, String(y), String(horiSelectionIdx), String(vertSelectionIdx), "R", text);
}
void SelectionDDLayer::select(int horiSelectionIdx, int vertSelectionIdx, bool deselectTheOthers) {
  _sendCommand3(layerId, C_select, String(horiSelectionIdx), String(vertSelectionIdx), TO_BOOL(deselectTheOthers));
}
void SelectionDDLayer::deselect(int horiSelectionIdx, int vertSelectionIdx, bool selectTheOthers) {
  _sendCommand3(layerId, C_deselect, String(horiSelectionIdx), String(vertSelectionIdx), TO_BOOL(selectTheOthers));
}
void SelectionDDLayer::selected(bool selected, int horiSelectionIdx, int vertSelectionIdx, bool reverseTheOthers) {
  if (selected) {
    _sendCommand3(layerId, C_select, String(horiSelectionIdx), String(vertSelectionIdx), TO_BOOL(reverseTheOthers));
  } else {
    _sendCommand3(layerId, C_deselect, String(horiSelectionIdx), String(vertSelectionIdx), TO_BOOL(reverseTheOthers));
  }
}

void SelectionListDDLayer::add(int selectionIdx) {
  _sendCommand1(layerId, C_add, String(selectionIdx));
}
void SelectionListDDLayer::remove(int selectionIdx) {
  _sendCommand1(layerId, C_remove, String(selectionIdx));
}
void SelectionListDDLayer::offset(int offset) {
  _sendCommand1(layerId, C_offset, String(offset));
}
void SelectionListDDLayer::text(int selectionIdx, const String& text, int y, const String& align) {
  _sendCommand4(layerId, C_text, String(y), String(selectionIdx), align, text);
}
void SelectionListDDLayer::textCentered(int selectionIdx, const String& text, int y) {
  _sendCommand4(layerId, C_text, String(y), String(selectionIdx), "C", text);
}
void SelectionListDDLayer::textRightAligned(int selectionIdx, const String& text, int y) {
  _sendCommand4(layerId, C_text, String(y), String(selectionIdx), "R", text);
}
void SelectionListDDLayer::unselectedText(int selectionIdx, const String& text, int y, const String& align) {
  _sendCommand4(layerId, C_unselectedtext, String(y), String(selectionIdx), align, text);
}
void SelectionListDDLayer::unselectedTextCentered(int selectionIdx, const String& text, int y) {
  _sendCommand4(layerId, C_unselectedtext, String(y), String(selectionIdx), "C", text);
}
void SelectionListDDLayer::unselectedTextRightAligned(int selectionIdx, const String& text, int y) {
  _sendCommand4(layerId, C_unselectedtext, String(y), String(selectionIdx), "R", text);
}
void SelectionListDDLayer::select(int selectionIdx, bool deselectTheOthers) {
  _sendCommand2(layerId, C_select, String(selectionIdx), TO_BOOL(deselectTheOthers));
}
void SelectionListDDLayer::deselect(int selectionIdx, bool selectTheOthers) {
  _sendCommand2(layerId, C_deselect, String(selectionIdx), TO_BOOL(selectTheOthers));
}
void SelectionListDDLayer::selected(int selectionIdx, bool selected, bool reverseTheOthers) {
  if (selected) {
    _sendCommand2(layerId, C_select, String(selectionIdx), TO_BOOL(reverseTheOthers));
  } else {
    _sendCommand2(layerId, C_deselect, String(selectionIdx), TO_BOOL(reverseTheOthers));
  }
}


void GraphicalDDLayer::setRotation(int8_t rotationType) {
  _sendCommand1(layerId, C_setrot, String(rotationType));
}
void GraphicalDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, C_setcursor, String(x), String(y));
}
void GraphicalDDLayer::moveCursorBy(int byX, int byY) {
  _sendCommand2(layerId, C_movecursorby, String(byX), String(byY));
}
// void GraphicalDDLayer::setTextColor(const String& color) {
//   _sendCommand1(layerId, "textcolor", color);
// }
void GraphicalDDLayer::setTextColor(const String& color, const String& bgColor) {
  _sendCommand2(layerId, C_textcolor, color, bgColor);
}
void GraphicalDDLayer::setTextSize(int size) {
  _sendCommand1(layerId, C_textsize, String(size));
}
void GraphicalDDLayer::setTextFont() {
  _sendCommand0(layerId, C_textfont);
}
void GraphicalDDLayer::setTextFont(const String& fontName) {
  _sendCommand1(layerId, C_textfont, fontName);
}
void GraphicalDDLayer::setTextWrap(bool wrapOn) {
  _sendCommand1(layerId, C_settextwrap, TO_BOOL(wrapOn));
}
void GraphicalDDLayer::fillScreen(const String& color) {
  _sendCommand1(layerId, C_fillscreen, color);
}
void GraphicalDDLayer::print(const String& text) {
  _sendCommand1(layerId, C_print, text);
}
void GraphicalDDLayer::println(const String& text) {
  _sendCommand1(layerId, C_println, text);
}
void GraphicalDDLayer::drawChar(int x, int y, char c, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, C_drawchar, TO_C_INT(x), TO_C_INT(y), color, bgColor, String(size), String(c));
}
void GraphicalDDLayer::drawStr(int x, int y, const String& string, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, C_drawstr, TO_C_INT(x), TO_C_INT(y), color, bgColor, String(size), string);
}
void GraphicalDDLayer::drawTextLine(const String& text, int y, const String& align, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, C_drawtextline, TO_C_INT(y), align, color, bgColor, String(size), text);
}
void GraphicalDDLayer::drawPixel(int x, int y, const String& color) {
  _sendCommand3(layerId, C_drawpixel, TO_C_INT(x), TO_C_INT(y), color);
}
void GraphicalDDLayer::drawLine(int x1, int y1, int x2, int y2, const String& color) {
  _sendCommand5(layerId, C_drawline, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), color);
}
void GraphicalDDLayer::drawRect(int x, int y, int w, int h, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand4(layerId, c_drawrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h));
    } else {
      _sendCommand6(layerId, c_drawrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
    }
  } else {
      _sendCommand6(layerId, c_drawrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
  }
}
void GraphicalDDLayer::drawOval(int x, int y, int w, int h, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand4(layerId, c_drawoval, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h));
    } else {
      _sendCommand6(layerId, c_drawoval, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
    }
  } else {
    _sendCommand6(layerId, c_drawoval, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
  }
}
void GraphicalDDLayer::drawArc(int x, int y, int w, int h, int startAngle, int sweepAngle, bool useCenter, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand7(layerId, c_drawarc, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(startAngle), TO_C_INT(sweepAngle), TO_BOOL(useCenter));
    } else {
      _sendCommand9(layerId, c_drawarc, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(startAngle), TO_C_INT(sweepAngle), TO_BOOL(useCenter), color, TO_BOOL(filled));
    }
  } else {
    _sendCommand9(layerId, c_drawarc, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(startAngle), TO_C_INT(sweepAngle), TO_BOOL(useCenter), color, TO_BOOL(filled));
  }
}
// void GraphicalDDLayer::fillRect(int x, int y, int w, int h, const String& color) {
//   _sendCommand6(layerId, "drawrect", String(x), String(y), String(w), String(h), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawCircle(int x, int y, int r, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand3(layerId, C_drawcircle, TO_C_INT(x), TO_C_INT(y), TO_C_INT(r));
    } else {
      _sendCommand5(layerId, C_drawcircle, TO_C_INT(x), TO_C_INT(y), TO_C_INT(r), color, TO_BOOL(filled));
    }
  } else {
    _sendCommand5(layerId, C_drawcircle, TO_C_INT(x), TO_C_INT(y), TO_C_INT(r), color, TO_BOOL(filled));
  }
}
// void GraphicalDDLayer::fillCircle(int x, int y, int r, const String& color) {
//   _sendCommand5(layerId, "drawcircle", String(x), String(y), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand6(layerId, C_drawtriangle, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), TO_C_INT(x3), TO_C_INT(y3));
    } else {
      _sendCommand8(layerId, C_drawtriangle, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), TO_C_INT(x3), TO_C_INT(y3), color, TO_BOOL(filled));
    }
  } else {
    _sendCommand8(layerId, C_drawtriangle, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), TO_C_INT(x3), TO_C_INT(y3), color, TO_BOOL(filled));
  }
}
// void GraphicalDDLayer::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color) {
//   _sendCommand8(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawRoundRect(int x, int y, int w, int h, int r, const String& color, bool filled) {
  if (_DDCompatibility >= 11) {
    if (color == "" && !filled) {
      _sendCommand5(layerId, C_drawroundrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(r));
    } else {
      _sendCommand7(layerId, C_drawroundrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(r), color, TO_BOOL(filled));
    }
  } else {
    _sendCommand7(layerId, C_drawroundrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(r), color, TO_BOOL(filled));
  }
}
// void GraphicalDDLayer::fillRoundRect(int x, int y, int w, int h, int r, const String& color) {
//   _sendCommand7(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::forward(int distance, bool withPen) {
  if (_DDCompatibility >= 12) {
    _sendCommand2(layerId, C_fd, String(distance), TO_BOOL(!withPen));
  } else {
    _sendCommand1(layerId, C_fd, String(distance));
  }
}
void GraphicalDDLayer::backward(int distance, bool withPen) {
  _sendCommand2(layerId, C_bk, String(distance), TO_BOOL(!withPen));
  //_sendCommand1(layerId, C_fd, String(distance));
}
void GraphicalDDLayer::leftTurn(int angle) {
  _sendCommand1(layerId, C_lt, String(angle));
}
void GraphicalDDLayer::rightTurn(int angle) {
  _sendCommand1(layerId, C_rt, String(angle));
}
void GraphicalDDLayer::setHeading(int angle) {
  _sendCommand1(layerId, C_seth, String(angle));
}
// void GraphicalDDLayer::goTo(int x, int y, bool withPen) {
//   _sendCommand2(layerId, withPen ? "goto" : "jto", String(x), String(y));
// }
// void GraphicalDDLayer::penUp() {
//   _sendCommand0(layerId, "pu");
// }
// void GraphicalDDLayer::penDown() {
//   _sendCommand0(layerId, "pd");
// }
void GraphicalDDLayer::penSize(int size) {
  _sendCommand1(layerId, C_pensize, String(size));
}
void GraphicalDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, C_pencolor, color);
}
void GraphicalDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, C_fillcolor, color);
}
void GraphicalDDLayer::noFillColor() {
  _sendCommand0(layerId, C_nofillcolor);
}
void GraphicalDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? C_ccircle : C_circle, String(radius));
}
void GraphicalDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? C_coval : C_oval, String(width), String(height));
}
void GraphicalDDLayer::arc(int width, int height, int startAngle, int sweepAngle, bool centered) {
  _sendCommand4(layerId, centered ? C_carc : C_arc, String(width), String(height), String(startAngle), String(sweepAngle));
}
void GraphicalDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? C_crect : C_rect, String(width), String(height));
}
void GraphicalDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, C_trisas, String(side1), String(angle), String(side2));
}
void GraphicalDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, C_trisas, String(side), String(angle));
}
void GraphicalDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, C_poly, String(side), String(vertexCount));
}
void GraphicalDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? C_cpolyin : C_cpoly, String(radius), String(vertexCount));
}
void GraphicalDDLayer::loadImageFile(const String& imageFileName, int w, int h, const String& asImageFileName) {
  _sendCommand4(layerId, C_loadimagefile, imageFileName, String(w), String(h), asImageFileName);
}
void GraphicalDDLayer::loadImageFileCropped(const String& imageFileName, int x, int y, int w, int h, const String& asImageFileName, int scaleW, int scaleH) {
  if (scaleW == 0 && scaleH == 0) {
    _sendCommand6(layerId, C_loadimagefilecropped, imageFileName, String(x), String(y), String(w), String(h), asImageFileName);
  } else {
    _sendCommand8(layerId, C_loadimagefilecropped, imageFileName, String(x), String(y), String(w), String(h), asImageFileName, String(scaleW), String(scaleH));
  }
}
void GraphicalDDLayer::cacheImage(const String& imageName, const uint8_t *bytes, int byteCount, char compressMethod) {
  _sendCommand2("", C_CACHEIMG, layerId, imageName);
  _sendByteArrayAfterCommand(bytes, byteCount, compressMethod);
}
void GraphicalDDLayer::cachePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color, char compressMethod) {
  //int byteCount = width * height / 8; 
  int byteCount = (width + 7) * height / 8; 
  _sendCommand5("", C_CACHEPIXIMG, layerId, imageName, String(width), String(height), color);
  _sendByteArrayAfterCommand(bytes, byteCount, compressMethod);
}
void GraphicalDDLayer::cachePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options, char compressMethod) {
  int byteCount = 2 * width * height; 
  _sendCommand6("", C_CACHEPIXIMG16, layerId, imageName, String(width), String(height), TO_EDIAN(), options);
  _sendByteArrayAfterCommand((uint8_t*) data, byteCount, compressMethod);
}
void GraphicalDDLayer::cachePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options, char compressMethod) {
  int byteCount = width * height; 
  _sendCommand5("", C_CACHEPIXIMGGS, layerId, imageName, String(width), String(height), options);
  _sendByteArrayAfterCommand(data, byteCount, compressMethod);
}
void GraphicalDDLayer::saveCachedImageFile(const String& imageName, const String& asImageName) {
  _sendCommand3("", C_SAVECACHEDIMG, layerId, imageName, asImageName);
}
#ifdef ESP32
void GraphicalDDLayer::saveCachedImageFileWithTS(const String& imageName, const String& asImageName, int64_t imageTimestamp) {
  _sendCommand4("", C_SAVECACHEDIMG, layerId, imageName, asImageName, String(imageTimestamp));
}
#endif
void GraphicalDDLayer::saveCachedImageFileAsync(const String& imageName, const String& asImageName) {
  _sendCommand5("", C_SAVECACHEDIMG, layerId, imageName, asImageName, String(0), TO_BOOL(true));
}
#ifdef ESP32
void GraphicalDDLayer::saveCachedImageFileWithTSAsync(const String& imageName, const String& asImageName, int64_t imageTimestamp) {
  _sendCommand5("", C_SAVECACHEDIMG, layerId, imageName, asImageName, String(imageTimestamp), TO_BOOL(true));
}
#endif
void GraphicalDDLayer::saveCachedImageFiles(const String& stitchAsImageName) {
  _sendCommand2("", C_SAVECACHEDIMGS, layerId, stitchAsImageName);
}
void GraphicalDDLayer::unloadImageFile(const String& imageFileName) {
  _sendCommand1(layerId, C_unloadimagefile, imageFileName);
}
void GraphicalDDLayer::unloadAllImageFiles() {
  _sendCommand0(layerId, C_unloadallimagefiles);
}
void GraphicalDDLayer::drawImageFile(const String& imageFileName, int x, int y, int w, int h, const String& options) {
  if (_DDCompatibility >= 11) {
    if (x == 0 && y == 0 && w == 0 && h == 0) {
      _sendCommand2(layerId, C_drawimagefile, imageFileName, options);
    } else if (x == 0 && y == 0) {
      if (options == "") {
        _sendCommand3(layerId, C_drawimagefile, imageFileName, String(w), String(h));
      } else {
        _sendCommand4(layerId, C_drawimagefile, imageFileName, String(w), String(h), options);
      }  
    } else {
      if (options == "") {
        _sendCommand5(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h));
      } else {
        _sendCommand6(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h), options);
      }
    }
  } else { 
    if (w == 0 && h == 0 && options == "") {
      _sendCommand3(layerId, C_drawimagefile, imageFileName, String(x), String(y));
    } else {
      if (options == "") {
        _sendCommand5(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h));
      } else {
        _sendCommand6(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h), options);
      }
    }
  }
}
void GraphicalDDLayer::drawImageFileFit(const String& imageFileName, int x, int y, int w, int h, const String& options) {
  if (x == 0 && y == 0 && w == 0 && h == 0 && options == "") {
    _sendCommand1(layerId, C_drawimagefilefit, imageFileName);
  } else {
    _sendCommand6(layerId, C_drawimagefilefit, imageFileName, String(x), String(y), String(w), String(h), options);
  }
}
void GraphicalDDLayer::write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? C_drawtext : C_write, text);
}


void SevenSegmentRowDDLayer::segmentColor(const String& color) {
  _sendCommand1(layerId, C_segcolor, color);
}
void SevenSegmentRowDDLayer::resetSegmentOffColor(const String& color) {
  _sendCommand1(layerId, C_resetsegoffcolor, color);
}
void SevenSegmentRowDDLayer::resetSegmentOffNoColor() {
  _sendCommand0(layerId, C_resetsegoffcolor);
}
void SevenSegmentRowDDLayer::turnOn(const String& segments, int digitIdx) {
  _sendCommand2(layerId, C_segon, segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::turnOff(const String& segments, int digitIdx) {
  _sendCommand2(layerId, C_segoff, segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::setOn(const String& segments, int digitIdx) {
  _sendCommand2(layerId, C_setsegon, segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::showDigit(int digit, int digitIdx) {
  _sendCommand2(layerId, C_showdigit, String(digit) , String(digitIdx));
}
void SevenSegmentRowDDLayer::showNumber(float number, const String& padding) {
  if (IS_FLOAT_WHOLE(number)) {
    _sendCommand2(layerId, C_shownumber, String((int) number), padding);
  } else {
    _sendCommand2(layerId, C_shownumber, String(number, 5), padding);
  }
}
void SevenSegmentRowDDLayer::showHexNumber(int16_t number) {
  _sendCommand1(layerId, C_showhex, String(number));
}
void SevenSegmentRowDDLayer::showFormatted(const String& formatted, bool completeReplace, int startIdx) {
  _sendCommand3(layerId, C_showformatted, formatted, TO_BOOL(completeReplace), String(startIdx));
}

void JoystickDDLayer::autoRecenter(bool autoRecenter) {
  _sendCommand1(layerId, C_autorecenter, TO_BOOL(autoRecenter));
}
void JoystickDDLayer::colors(const String& stickColor, const String& stickOutlineColor, const String& socketColor, const String& socketOutlineColor) {
  _sendCommand4(layerId, C_colors, stickColor, stickOutlineColor, socketColor, socketOutlineColor);
}
void JoystickDDLayer::moveToPos(int16_t x, int16_t y, bool sendFeedback) {
  _sendCommand3(layerId, C_movetopos, TO_C_INT(x), TO_C_INT(y), TO_BOOL(sendFeedback));
}
void JoystickDDLayer::moveToCenter(bool sendFeedback) {
  _sendCommand1(layerId, C_movetocenter, TO_BOOL(sendFeedback));
}
void JoystickDDLayer::valueRange(int16_t minValue, int16_t maxValue, int valueStep, bool sendFeedback) {
    if (_DDCompatibility >= 8) {
      _sendCommand4(layerId, C_valuerange, String(minValue), String(maxValue), String(valueStep), TO_BOOL(sendFeedback));
    } else {
      _sendCommand3(layerId, C_valuerange, String(minValue), String(maxValue), TO_BOOL(sendFeedback));
    }
}
void JoystickDDLayer::snappy(bool snappy) {
  _sendCommand1(layerId, C_snappy, TO_BOOL(snappy));
}
void JoystickDDLayer::showValue(bool show, const String& color) {
  _sendCommand2(layerId, C_showvalue, TO_BOOL(show), color);
}


void PlotterDDLayer::label(const String& key, const String& lab) {
  _sendCommand2(layerId, C_label, key, lab);
}
void PlotterDDLayer::set(const String& key, float value) {
  _sendCommand2(layerId, "", key, TO_C_NUM(value));
}  
void PlotterDDLayer::set(const String& key1, float value1, const String& key2, float value2) {
  _sendCommand4(layerId, "", key1, TO_C_NUM(value1), key2, TO_C_NUM(value2));
}  
void PlotterDDLayer::set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3) {
  _sendCommand6(layerId, "", key1, TO_C_NUM(value1), key2, TO_C_NUM(value2), key3, TO_C_NUM(value3));
  //_sendCommand6(layerId, "", key1, String(value1), key2, String(value2), key3, String(value3));
}  
void PlotterDDLayer::set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3, const String& key4, float value4) {
  _sendCommand8(layerId, "", key1, TO_C_NUM(value1), key2, TO_C_NUM(value2), key3, TO_C_NUM(value3), key4, TO_C_NUM(value4));
}  

 void TomTomMapDDLayer::goTo(float latitude, float longitude, const String& label) {
  _sendCommand3(layerId, C_goto, TO_NUM(latitude), TO_NUM(longitude), label);
}
void TomTomMapDDLayer::zoomTo(float latitude, float longitude, float zoomLevel, const String& label) {
  _sendCommand4(layerId, C_zoomto, TO_NUM(latitude), TO_NUM(longitude), TO_NUM(zoomLevel), label);
}
void TomTomMapDDLayer::zoom(float zoomLevel) {
  _sendCommand1(layerId, C_zoom, TO_NUM(zoomLevel));
}

void TerminalDDLayer::print(const String& val) {
  _sendCommand1(layerId, C_print, val);
}
void TerminalDDLayer::println(const String& val) {
  _sendCommand1(layerId, C_println, val);
}

void WebViewDDLayer::loadUrl(const String& url) {
  _sendCommand1(layerId, C_loadurl, url);
}
void WebViewDDLayer::loadHtml(const String& html) {
  _sendCommand1(layerId, C_loadhtml, html);
}
void WebViewDDLayer::execJs(const String& js) {
  _sendCommand1(layerId, C_execjs, js);
}

void DumbDisplayWindowDDLayer::connect(const String& deviceType, const String& deviceName, const String& deviceAddress) {
  _sendCommand3(layerId, C_connect, deviceType, deviceName, deviceAddress);
}
void DumbDisplayWindowDDLayer::disconnect() {
  _sendCommand0(layerId, C_disconnect);
}

void RtspClientDDLayer::start(const String& url) {
  _sendCommand1(layerId, "start", url);
}
void RtspClientDDLayer::stop() {
  _sendCommand0(layerId, "stop");
}




// bool DDInputOutput::available() {
//   return Serial.available();
// }
// char DDInputOutput::read() {
//   return Serial.read();
// }
// void DDInputOutput::print(const String &s) {
//     Serial.print(s);
// }
// void DDInputOutput::print(const char *p) {
//     Serial.print(p);
// }
// void DDInputOutput::flush() {
//   Serial.flush();
// }
// void DDInputOutput::preConnect() {
//   if (setupForSerial)
//     Serial.begin(serialBaud);
// }

//#ifdef SUPPORT_TUNNEL
DDTunnel::DDTunnel(const String& type, int8_t tunnelId, const String& paramsParam, const String& endPointParam/*, bool connectNow*/):
  type(type), tunnelId(String(tunnelId)), endPoint(endPointParam), params(paramsParam) {
    this->objectType = DD_OBJECT_TYPE_TUNNEL;
  // this->arraySize = bufferSize;
  // this->dataArray = new String[bufferSize];
  // this->nextArrayIdx = 0;
  // this->validArrayIdx = 0;
//  this->done = false;
  //this->done = true;
  //this->timedOut = false;
  this->doneState = 1;
  // if (connectNow) {
  //   reconnect();
  // }
// #ifdef DEBUG_MISSING_ENDPOINT_C  
//     if (this->endPoint.c_str() == NULL) {
//       __SendComment("XXXXX");
//     }
//   __SendComment("ConstructEP!!! aaa");
//   __SendComment(this->endPoint);
//   __SendComment("ConstructEP!!! bbb");
//   __SendComment(endPointParam);
//   __SendComment("ConstructEP!!! ccc");
// #endif
}
void DDTunnel::afterConstruct(bool connectNow) {
// #ifdef DEBUG_MISSING_ENDPOINT_C  
// __SendComment("Before!!!");
// //__SendComment("==> https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/lock-unlocked.png");
// __SendComment(endPoint);
// //__SendComment(("AfterConstructEP -- " + endPoint).c_str());
// //__SendComment("AfterConstructEP -- " + String(endPoint.length()));
// __SendComment("After!!!");
// #endif
  if (connectNow) {
    _reconnect();
  }
}
DDTunnel::~DDTunnel() {
#ifdef SUPPORT_TUNNEL	
#ifdef DD_DEBUG_BASIC  
  if (_CanLogToSerial()) Serial.println("--- delete DDTunnel");
#endif
  _PreDeleteTunnel(this);
  //delete this->dataArray;
#endif
} 
void DDTunnel::_reconnect(const String& extraParams) {
  if (true) {
    if (endPoint.c_str() == NULL) {
      //__SendComment("DDTunnel::reconnect() - invalid tunnel endpoint", true);
      __SendErrorComment("invalid tunnel endpoint");
      return;
    }
  }
#ifdef DEBUG_MISSING_ENDPOINT_C  
_sendCommand0("", ("// EP -- " + endPoint).c_str());
#endif
  if (endPoint != "") {
    //nextArrayIdx = 0;
    //validArrayIdx = 0;
//    done = false;
//    timedOut = false;
    //for (int i = 0; i < arraySize; i++) {
      //dataArray[i] = "";
    //}
    if (_DDCompatibility >= 11) {  // since 2025-04-19 for extraParams
      String connectParams = params;
      if (extraParams.length() > 0) {
        if (connectParams.length() > 0) {
          connectParams.concat(",");
        }
        connectParams.concat(extraParams);
      }
      _setLTBufferCommand(type);
      if (connectParams.length() > 0) {
        _setLTBufferCommand(":");
        _setLTBufferCommand(connectParams);
      }
      _setLTBufferCommand("@");
      if (headers.length() > 0 || attachmentId.length() > 0) {
        _setLTBufferCommand(headers);
        _setLTBufferCommand("~");
        _setLTBufferCommand(attachmentId);
        _setLTBufferCommand("@");
      }
      _setLTBufferCommand(endPoint);
      _sendSpecialCommand("lt", tunnelId, "reconnect", "");
    } else if (_DDCompatibility >= 4) {
      _setLTBufferCommand(type);
      if (params.length() > 0) {
        _setLTBufferCommand(":");
        _setLTBufferCommand(params);
      }
      _setLTBufferCommand("@");
      if (headers.length() > 0 || attachmentId.length() > 0) {
        _setLTBufferCommand(headers);
        _setLTBufferCommand("~");
        _setLTBufferCommand(attachmentId);
        _setLTBufferCommand("@");
      }
      _setLTBufferCommand(endPoint);
      _sendSpecialCommand("lt", tunnelId, "reconnect", "");
    } else {
      String data;
      data.concat(type);
      if (params.length() > 0) {
        data.concat(":");
        data.concat(params);
      }
      data.concat("@");
      if (headers.length() > 0 || attachmentId.length() > 0) {
        data.concat(headers);
        data.concat("~");
        data.concat(attachmentId);
        data.concat("@");
      }
      data.concat(endPoint);
      if (_DDCompatibility >= 4) {
        int dataLen = data.length();
        int i = 0;
        while (i < dataLen) {
          int len = dataLen - i;
          if (len > TL_BUFFER_DATA_LEN) {
            len = TL_BUFFER_DATA_LEN;
          }
          int j = i + len;
          String partData = data.substring(i, j);
          _sendSpecialCommand("ltbuf", "", NULL, partData);
          i = j;
        }
        data = "";
      }
      _sendSpecialCommand("lt", tunnelId, "reconnect", data);
    }
    connectMillis = millis();
    //done = false;
    //timedOut = false;
    doneState = 0;
  }
}
void DDTunnel::_reconnectTo(const String& endPoint, const String& extraParams) {
  this->endPoint = endPoint;
//Serial.print("//!! endPoint: ");
//Serial.print(endPoint);
//Serial.print(" ==> ");
//Serial.println(this->endPoint);
  if (true) {
    if (this->endPoint.c_str() == NULL) {
      __SendErrorComment("failed to set endPoint");
    }
  }
  _reconnect(extraParams);
}
void DDTunnel::release() {
  if (doneState == 0/*!done*/) {
    _sendSpecialCommand("lt", this->tunnelId, "disconnect", "");
  }
  //done = true;
  if (doneState == 0) {
    doneState = 1;
  }
}
// int DDTunnel::_count() {
//   return (arraySize + validArrayIdx - nextArrayIdx) % arraySize;
// }
bool DDTunnel::_eof(long timeoutMillis) {
  // if (true) {
  //   delay(200);
  // } else {
  //   yield();
  // }
  _HandleFeedback();
//#ifdef DD_DEF_TUNNEL_TIMEOUT
  // if (timeoutMillis <= 0) {
  //   //timeoutMillis = DD_DEF_TUNNEL_TIMEOUT;
  // }
  if (/*done*/doneState != 0 || timeoutMillis <= 0) {
// #ifdef DEBUG_TUNNEL_RESPONSE
// Serial.println("_EOF: " + (done ? "DONE" : "NOT DONE"));
// #endif                
// #ifdef DEBUG_TUNNEL_RESPONSE_C                
// __SendComment("_EOF: " + (done ? "DONE" : "NOT DONE"));
// #endif
      return doneState != 0/*done*/;
  }
  long diff = millis() - connectMillis;
  if (diff > timeoutMillis/*diff > TUNNEL_TIMEOUT_MILLIS*/) {
#ifdef DEBUG_TUNNEL_RESPONSE
    Serial.println("_EOF: XXX TIMEOUT XXX");
#endif                
    __SendErrorComment("*** TUNNEL TIMEOUT ***");
    doneState = -1;
    //timedOut = true;
    _sendSpecialCommand("lt", this->tunnelId, "disconnect", "");  // disconnect since 2024-05-25 ... if reconnected, might have residual data from previous connection for slow protocol like BLE
    //done = true;  // treat as done
    return true;
  }
  return false;
// #else
//     return /*nextArrayIdx == validArrayIdx && */done;
// #endif    
}
// void DDTunnel::_readLine(String &buffer) {
//   if (nextArrayIdx == validArrayIdx) {
//     buffer = "";
//   } else {
//     buffer = dataArray[validArrayIdx];
//     dataArray[validArrayIdx] = "";
//     validArrayIdx = (validArrayIdx + 1) % arraySize;
//   }
// }
void DDTunnel::_writeLine(const String& data) {
//Serial.println("//--");
  _sendSpecialCommand("lt", tunnelId, NULL, data);
}
void DDTunnel::_writeSound(const String& soundName) {
  _sendSpecialCommand("lt", tunnelId, "send_sound", soundName);
}
//void DDTunnel::handleInput(const String& data, bool final) {
void DDTunnel::doneHandleInput(bool final) {
//if (final) Serial.println("//final:" + data);
  // if (!final || data != "") {
  //   dataArray[nextArrayIdx] = data;
  //   nextArrayIdx  = (nextArrayIdx + 1) % arraySize;
  //   if (nextArrayIdx == validArrayIdx)
  //     validArrayIdx = (validArrayIdx + 1) % arraySize;
  // }
  if (final) {
    //this->done = true;
    if (doneState == 0) {
      doneState = 1;
    }
  }
//Serial.println(String("// ") + (final ? "f" : "."));
}
DDBufferedTunnel::DDBufferedTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize):
  DDTunnel(type, tunnelId, params, endPoint/*, connectNow, bufferSize*/) {
  bufferSize = bufferSize + 1;  // need one more
  this->arraySize = bufferSize;
  this->dataArray = new String[bufferSize];
#ifdef FEEDBACK_SUPPORT_BYTES
  this->fbByesArray = new uint8_t*[bufferSize];
  for (int i = 0; i < arraySize; i++) {
    fbByesArray[i] = NULL;
  }
#endif
  this->nextArrayIdx = 0;
  this->validArrayIdx = 0;
  //this->done = false;
}
DDBufferedTunnel::~DDBufferedTunnel() {
  for (int i = 0; i < arraySize; i++) {
    dataArray[i] = "";
#ifdef FEEDBACK_SUPPORT_BYTES
    if (fbByesArray[i] != NULL) {
      delete fbByesArray[i];
    }
#endif
  }
  // TODO: check ... there seems to be issue (hang) delete dataArray
  //delete dataArray;
  //delete fbByesArray;
} 
void DDBufferedTunnel::_reconnect(const String& extraParams) {
  nextArrayIdx = 0;
  validArrayIdx = 0;
  //done = false;
  for (int i = 0; i < arraySize; i++) {
    dataArray[i] = "";
  }
#ifdef FEEDBACK_SUPPORT_BYTES
  for (int i = 0; i < arraySize; i++) {
    if (fbByesArray[i] != NULL) {
      delete fbByesArray[i];
    }
    fbByesArray[i] = NULL;
  }
#endif
  //_sendSpecialCommand("lt", tunnelId, "reconnect", endPoint);
  this->DDTunnel::_reconnect(extraParams);
}
void DDBufferedTunnel::release() {
  // if (!done) {
  //   _sendSpecialCommand("lt", this->tunnelId, "disconnect", "");
  // }
  // done = true;
  this->DDTunnel::release();
}
bool DDBufferedTunnel::pending() {
  return DDTunnel::_pending() || nextArrayIdx != validArrayIdx;
}
int DDBufferedTunnel::_count() {
  //int count = (arraySize + validArrayIdx - nextArrayIdx) % arraySize;
  int count = (arraySize + nextArrayIdx - validArrayIdx) % arraySize;
// #ifdef DEBUG_TUNNEL_RESPONSE                
// Serial.print("COUNT: ");
// Serial.println(count);
// #endif
  return count;
}
bool DDBufferedTunnel::_eof(long timeoutMillis) {
  if (false) {  // disabled since 2023-07-18
      _HandleFeedback();
  }
  if (!this->DDTunnel::_eof(timeoutMillis)) {
    return false;
  }
  if (this->_timedOut()) {
    return true;
  }
// #ifdef DEBUG_TUNNEL_RESPONSE                
// Serial.print("CHECK EOF ...");
// Serial.print("validArrayIdx:");
// Serial.print(validArrayIdx);
// Serial.print(" / nextArrayIdx:");
// Serial.println(nextArrayIdx);
// #endif
  return nextArrayIdx == validArrayIdx; 
  //bool eof = (nextArrayIdx == validArrayIdx) && this->DDTunnel::_eof();
// #ifdef DEBUG_TUNNEL_RESPONSE                
// Serial.print("EOF:");
// Serial.println(eof);
// #endif
//  return eof;
}
bool DDBufferedTunnel::_readLine(String &buffer, uint8_t** pFBBytes) {
  if (nextArrayIdx == validArrayIdx) {
    buffer = "";
    if (pFBBytes != NULL) {
      *pFBBytes = NULL;
    }
    return false;
  } else {
    buffer = dataArray[validArrayIdx];
    dataArray[validArrayIdx] = "";
#ifdef FEEDBACK_SUPPORT_BYTES
    if (pFBBytes != NULL) {
      *pFBBytes = fbByesArray[validArrayIdx];
    } else {
      if (fbByesArray[validArrayIdx] != NULL) {
        delete fbByesArray[validArrayIdx];
      }
    }
    fbByesArray[validArrayIdx] = NULL;
#endif
    validArrayIdx = (validArrayIdx + 1) % arraySize;
    return true;
  }
}

// void DDTunnel::_writeLine(const String& data) {
// //Serial.println("//--");
//   _sendSpecialCommand("lt", tunnelId, NULL, data);
// }
void DDBufferedTunnel::handleInput(const String& data, uint8_t* fbBytes, bool final) {
#ifdef DEBUG_TUNNEL_RESPONSE
Serial.print("DATA:");
Serial.print(data);
Serial.print(" ... validArrayIdx:");
Serial.print(validArrayIdx);
Serial.print(" / nextArrayIdx:");
Serial.print(nextArrayIdx);
#endif
//if (final) Serial.println("//final:" + data);
  if (!final || data != "") {
    dataArray[nextArrayIdx] = data;
#ifdef FEEDBACK_SUPPORT_BYTES
    if (fbByesArray[nextArrayIdx] != NULL) {  // overflow???
      delete fbByesArray[nextArrayIdx];
    }
    fbByesArray[nextArrayIdx] = fbBytes;
#endif
    nextArrayIdx  = (nextArrayIdx + 1) % arraySize;
    if (nextArrayIdx == validArrayIdx)
      validArrayIdx = (validArrayIdx + 1) % arraySize;
  }
  //this->DDTunnel::handleInput(data, fbBytes, final);
  doneHandleInput(final);
  //if (final)
    //this->done = true;
//Serial.println(String("// ") + (final ? "f" : "."));
#ifdef DEBUG_TUNNEL_RESPONSE
Serial.print(" ==> validArrayIdx:");
Serial.print(validArrayIdx);
Serial.print(" / nextArrayIdx:");
Serial.println(nextArrayIdx);
#endif  
}
String DDBufferedTunnel::readLine() {
  String buffer;
  _readLine(buffer);
  return buffer;
}
bool DDBufferedTunnel::read(String& fieldId, String& fieldValue) {
  fieldId = "";
  if (!_readLine(fieldValue)) {
    return false;
  }
  if (_DDCompatibility >= 6) {
    if (fieldValue.length() > 0) {
      if (fieldValue.charAt(0) == '"') {
        int idx = fieldValue.indexOf("\":");
        if (idx != -1) {
          fieldId = fieldValue.substring(1, idx);
          fieldValue = fieldValue.substring(idx + 2);
        }
      } else {
        int idx = fieldValue.indexOf(":");
        if (idx != -1) {
          fieldId = fieldValue.substring(0, idx);
          fieldValue = fieldValue.substring(idx + 1);
        }
      }
    }
  } else {
    int idx = fieldValue.indexOf(":");
    if (idx != -1) {
      fieldId = fieldValue.substring(0, idx);
      fieldValue = fieldValue.substring(idx + 1);
    }
  }
  return true;
}
// bool JsonDDTunnel::read(String& fieldId, String& fieldValue) {
//   fieldId = "";
//   if (!_readLine(fieldValue)) {
//     return false;
//   }
//   int idx = fieldValue.indexOf(":");
//   if (idx != -1) {
//     fieldId = fieldValue.substring(0, idx);
//     fieldValue = fieldValue.substring(idx + 1);
//   }
//   return true;
//   // String buffer;
//   // if (!_readLine(buffer)) {
//   //   return false;
//   // }
//   // int idx = buffer.indexOf(":");
//   // if (idx != -1) {
//   //   fieldId = buffer.substring(0, idx);
//   //   fieldValue = buffer.substring(idx + 1);
//   // } else {
//   //   fieldId = "";
//   //   fieldValue = buffer;
//   // }
//   // return true;
// }
void SimpleToolDDTunnel::_reconnect(const String& extraParams) {
  this->result = 0;
  this->DDBufferedTunnel::_reconnect(extraParams);
}
int SimpleToolDDTunnel::checkResult() {
  if (this->result == 0) {
    if (count() > 0) {
      String fieldId;
      String fieldValue;
      read(fieldId, fieldValue);
#ifdef DEBUG_TUNNEL_RESPONSE      
Serial.print("GOT ");
Serial.print(fieldId);
Serial.print(" = ");
Serial.println(fieldValue); 
#endif    
      if (fieldId == "result") {
        this->result = fieldValue == "ok" ? 1 : -1;
      }
#ifdef DEBUG_TUNNEL_RESPONSE_C      
__SendComment("GOT [" + fieldId + "] = [" + fieldValue + "] ==> result=" + this->result); 
#endif    
    } else if (eof()) {
      // not quite expected
#ifdef DEBUG_TUNNEL_RESPONSE      
Serial.println("XXX EOF???");
#endif
#ifdef DEBUG_TUNNEL_RESPONSE_C      
__SendComment("XXX EOF???");
#endif
      this->result = -1;
    }
  }
  return this->result;
}

void ImageDownloadDDTunnel::reconnectTo(const String& endPoint, const String& cropUIConfig) {
  String extraParams = "";
  if (cropUIConfig.length() > 0) {
    extraParams = "CUI:" + cropUIConfig;
  }
  _reconnectTo(endPoint, extraParams);
}


void GpsServiceDDTunnel::reconnectForLocation(int repeat) {
  if (repeat == -1) {
      reconnectTo("location");
  } else {
      reconnectTo("location?repeat=" + String(repeat));
  }
}
bool GpsServiceDDTunnel::readLocation(DDLocation& location) {
  String value;
  if (!_readLine(value)) {
    return false;
  }
  int idx = value.indexOf("/");
  if (idx == -1) {
    return false;
  }
  String latitude = value.substring(0, idx);
  String longitude = value.substring(idx + 1);
  location.latitude = latitude.toFloat();
  location.longitude = longitude.toFloat();
  return true;
}

void ObjectDetectDemoServiceDDTunnel::reconnectForObjectDetect(const String& imageName) {
  reconnectTo("detect?imageName=" + imageName);
}
void ObjectDetectDemoServiceDDTunnel::reconnectForObjectDetectFrom(GraphicalDDLayer* pGraphicalLayer, const String& imageName) {
  reconnectTo("detectfrom?layerId=" + pGraphicalLayer->getLayerId() + "?imageName=" + imageName);
}


bool ObjectDetectDemoServiceDDTunnel::readObjectDetectResult(DDObjectDetectDemoResult& objectDetectResult) {
  String value;
  if (!_readLine(value)) {
    return false;
  }
  int sepIdx1 = value.indexOf('-');
  int sepIdx2 = value.indexOf('-', sepIdx1 + 1);
  int sepIdx3 = value.indexOf('-', sepIdx2 + 1);
  int sepIdx4 = value.indexOf(':', sepIdx3 + 1);
  if (sepIdx1 == -1 || sepIdx2 == -1 || sepIdx3 == -1 || sepIdx4 == -1) {
      return false;
  }
  objectDetectResult.left = value.substring(0, sepIdx1).toInt();
  objectDetectResult.top = value.substring(sepIdx1 + 1, sepIdx2).toInt();
  objectDetectResult.right = value.substring(sepIdx2 + 1, sepIdx3).toInt();
  objectDetectResult.bottom = value.substring(sepIdx3 + 1, sepIdx4).toInt();
  objectDetectResult.label = value.substring(sepIdx4 + 1);
  return true;
}

DDImageData::~DDImageData() {
  if (bytes != NULL) delete bytes;
}

void DDImageData::transferTo(DDImageData& imageData) {
  imageData.release();
  // if (imageData.bytes != NULL) {
  //   delete imageData.bytes;
  // }
  imageData.width = width;
  imageData.height = height;
  imageData.byteCount = byteCount;
  imageData.bytes = bytes;
  width = 0;
  height = 0;
  byteCount = 0;
  bytes = NULL;
}
void DDImageData::release() {
  if (bytes != NULL) {
    delete bytes;
    bytes = NULL;
  }
  width = 0;
  height = 0;
  byteCount = 0;
}


DDPixelImage16::~DDPixelImage16() {
  if (data != NULL) delete data;
}
void DDPixelImage16::transferTo(DDPixelImage16& imageData) {
  imageData.release();
  // if (imageData.data != NULL) {
  //   delete imageData.data;
  // }
  imageData.width = width;
  imageData.height = height;
  imageData.byteCount = byteCount;
  imageData.data = data;
  width = 0;
  height = 0;
  byteCount = 0;
  data = NULL;
}
void DDPixelImage16::release() {
  if (data != NULL) {
    delete data;
    data = NULL;
  }
  width = 0;
  height = 0;
  byteCount = 0;
}




void ImageRetrieverDDTunnel::reconnectForPixelImage(const String& imageName, int width, int height, bool fit) {
  reconnectTo("pixImg?name=" + imageName + "&width=" + String(width) + "&height=" + String(height) + "&fit=" + TO_BOOL(fit));
}
void ImageRetrieverDDTunnel::reconnectForPixelImage16(const String& imageName, int width, int height, bool fit, bool grayscale) {
  reconnectTo("pixImg16?name=" + imageName + "&width=" + String(width) + "&height=" + String(height) + "&fit=" + TO_BOOL(fit) + "&grayscale=" + TO_BOOL(grayscale));
}
void ImageRetrieverDDTunnel::reconnectForPixelImageGS(const String& imageName, int width, int height, bool fit) {
  reconnectTo("pixImgGS?name=" + imageName + "&width=" + String(width) + "&height=" + String(height) + "&fit=" + TO_BOOL(fit));
}
void ImageRetrieverDDTunnel::reconnectForJpegImage(const String& imageName, int width, int height, int quality, bool fit) {
  reconnectTo("jpeg?name=" + imageName + "&width=" + String(width) + "&height=" + String(height) + "&quality=" + quality + "&fit=" + TO_BOOL(fit));
}

bool _returnFailedReadImageData(DDImageData& imageData, uint8_t* bytesBuffer) {
  if (bytesBuffer != NULL) {
    delete bytesBuffer;
  }
  imageData.width = 0;
  imageData.height = 0;
  imageData.byteCount = 0;
  if (imageData.bytes != NULL) {
    delete imageData.bytes;
  }
  imageData.bytes = new uint8_t[0];
  return true;
}

bool ImageRetrieverDDTunnel::_readImageData(DDImageData& imageData, short type) {  // type: 0=BW, 1=8, 2=16, 3:JPEG
  String value;
  uint8_t* fbBytes;
  if (!_readLine(value, &fbBytes)) {
    return false;
  }
  // imageData.width = 0;
  // imageData.height = 0;
  // imageData.byteCount = 0;
  // imageData.bytes = NULL;
  int widthSepIdx = value.indexOf("x");
  int heightSepIdx = value.indexOf("|", widthSepIdx + 1);
  int byteCountSepIdx = value.indexOf("|:", heightSepIdx + 1);  // image bytes starts with ':'
#ifdef DEBUG_READ_PIXEL_IMAGE
  Serial.print("=== _readImageData (");
  Serial.print(type);
  Serial.println(") ===");
  Serial.print("*** widthSepIdx=");
  Serial.print(widthSepIdx);
  Serial.print(" / heightSepIdx=");
  Serial.print(heightSepIdx);
  Serial.print(" / byteCountSepIdx=");
  Serial.println(byteCountSepIdx);
#endif
  if (widthSepIdx == -1 || heightSepIdx == -1 || byteCountSepIdx == -1) {
    if (_CanLogToSerial()) {
      //int count = fmin(value.length(), 15);
      int count = value.length();
      if (count > 15) {
        count = 15;
      }
      Serial.print(String("XXX invalid (") + String(value.length()) + ")[" + value.substring(0, count) + "] ... XXX");
    }
    __SendErrorComment("invalid delimiters");
    return _returnFailedReadImageData(imageData, NULL);
  }
  int width = value.substring(0, widthSepIdx).toInt();
  int height = value.substring(widthSepIdx + 1, heightSepIdx).toInt();
  int byteCount = value.substring(heightSepIdx + 1, byteCountSepIdx).toInt();
#ifdef DEBUG_READ_PIXEL_IMAGE
  Serial.print("*** width=");
  Serial.print(width);
  Serial.print(" / height=");
  Serial.print(height);
  Serial.print(" / byteCount=");
  Serial.println(byteCount);
#endif
  if (width <= 0 || height <= 0 || byteCount <= 0) {
    __SendErrorComment("invalid width/height/byteCount");
    return _returnFailedReadImageData(imageData, NULL);
  }
  int expectedByteCount = 0;
  if (type == 0) {
    // BW
    expectedByteCount = (width + 7) * height / 8;
  } else if (type == 1) {
    // 8
    expectedByteCount = width * height;
  } else if (type == 2) {
    // 16
    expectedByteCount = 2 * width * height;
  }
#ifdef DEBUG_READ_PIXEL_IMAGE
  if (expectedByteCount > 0) {
    Serial.print("byteCount=");
    Serial.print(byteCount);
    Serial.print(" vs ");
    Serial.print("expectedByteCount=");
    Serial.println(expectedByteCount);
  }
#endif
  if (expectedByteCount > 0 && byteCount != expectedByteCount) {
    __SendErrorComment("byteCount mismatch");
    return true;
  }
  uint8_t* bytes;
  if (fbBytes != NULL) {
    bytes = fbBytes;
#ifdef DEBUG_READ_PIXEL_IMAGE
    Serial.print("***");
    Serial.print(byteCount);
    Serial.println(" bytes from FB bytes *****");
#endif
  } else {
    bytes = new uint8_t[byteCount];
    const char* p = value.c_str() + byteCountSepIdx + 2;
#ifdef DEBUG_READ_PIXEL_IMAGE
    Serial.println(*(p - 1));  // should be :
#endif
    int i = 0;
    while (i < byteCount) {
      char c = *p++;
      char b;
      if (c == '\\') {
        char nc = *p++;
        if (nc == 'n') {
          b = 10;
        } else if (nc == '_') {
          b = '\\';
        } else {
          b = nc;
        }
      } else {
        if (c == '0') {
          b = 0;
        } else {
          b = c;
        }
      }
      if (i >= byteCount) {
        __SendErrorComment("bytes overflow");
        return _returnFailedReadImageData(imageData, bytes);
      }
#ifdef DEBUG_READ_PIXEL_IMAGE
      if (i <= 100) {
        Serial.print("|");
        Serial.print(i);
        Serial.print(":");
        Serial.print(b, HEX);
        if (i == 100) {
          Serial.println();
        }
      }
#endif
      if (true) {  // UTF-8
        if (b & 0b10000000) {
          // 2 chars
          char nc0 = *p++;
          b = ((b & 0b11) << 6) + (nc0 & 0b111111);
        }
      }
      // if (i >= byteCount) {
      //   __SendErrorComment("bytes overflow");
      //   delete bytes;
      //   return NULL;
      // }
      bytes[i++] = b;
    }
#ifdef DEBUG_READ_PIXEL_IMAGE
  Serial.print("*** interpreted byteCount=");
  Serial.print(byteCount);
  Serial.print(" / i=");
  Serial.println(i);
#endif
  }  
  //pixelImage16.data = (uint16_t*) data;
  imageData.width = width;
  imageData.height = height;
  imageData.byteCount = byteCount;
  if (imageData.bytes != NULL) {
    delete imageData.bytes;
  }
  imageData.bytes = bytes;
  return true;
}

bool ImageRetrieverDDTunnel::readPixelImage(DDPixelImage& pixelImage) {
  return _readImageData(pixelImage, 0);
}
bool ImageRetrieverDDTunnel::readPixelImage16(DDPixelImage16& pixelImage16) {
  DDImageData imageData;
  if (!_readImageData(imageData, 2)) {
    return false;
  }
  pixelImage16.width = imageData.width;
  pixelImage16.height = imageData.height;
  pixelImage16.byteCount = imageData.byteCount;
  if (pixelImage16.data != NULL) {
    delete pixelImage16.data;
  }
  pixelImage16.data = (uint16_t* ) imageData.bytes;
  imageData.bytes = NULL;
#ifdef DEBUG_READ_PIXEL_IMAGE
  int byteCount = 2 * pixelImage16.width * pixelImage16.height;
  int maxI = byteCount / 2;
  if (maxI > 100) {
    maxI = 100;
  }
  for (int i = 0; i < maxI; i++) {
    uint16_t v = pixelImage16.data[i];
    if (v != 0) {
      Serial.print("/");
      Serial.print(i);
      Serial.print(":");
      Serial.print(v, HEX);
    }
  }
  Serial.println();
#endif
  return true;
}
bool ImageRetrieverDDTunnel::readPixelImageGS(DDPixelImage& pixelImage) {
  return _readImageData(pixelImage, 1);
}
bool ImageRetrieverDDTunnel::readPixelImageGS16(DDPixelImage16& pixelImage16) {
  DDImageData imageData;
  if (!_readImageData(imageData, 1)) {
    return false;
  }
  int width = imageData.width;
  int height = imageData.height;
  uint8_t* bytes = imageData.bytes;
  imageData.bytes = NULL;
  bool bigEdian = TO_EDIAN();
  int newByteCount = 2 * width * height;
  uint8_t* newData = NULL;
  if (newByteCount > 0) {
    newData = new uint8_t[newByteCount];
    int i_d = 0;
    int i_nd = 0;
    for (int h = 0; h < height; h++) {
      for (int w = 0; w < width; w++) {
        //uint8_t d = data[h * width + w];
        uint8_t d = bytes[i_d++];
        uint8_t c5 = 0b11111 & ((int) ((double) 0b11111 * (double) d / (double) 0xff)); 
        uint8_t c6 = 0b111111 & ((int) ((double) 0b111111 * (double) d / (double) 0xff)); 
        uint8_t lower = (c6 << 5) + c5;
        uint8_t higher = (c5 << 3) + (c6 >> 3);
        if (bigEdian) {
          uint8_t temp = lower;
          lower = higher;
          higher = temp;
        }
  // #ifdef DEBUG_READ_PIXEL_IMAGE
  //       Serial.print("[");
  //       Serial.print(lower);
  //       Serial.print("+");
  //       Serial.print(higher);
  //       Serial.print("]");
  // #endif
        // newData[i_nd++] = lower;
        // newData[i_nd++] = higher;
        newData[i_nd++] = higher;
        newData[i_nd++] = lower;
      }
    }
  }
  pixelImage16.width = width;
  pixelImage16.height = height;
  pixelImage16.byteCount = newByteCount;
  if (pixelImage16.data != NULL) {
    delete pixelImage16.data;
  }
  pixelImage16.data = (uint16_t*) newData;
#ifdef DEBUG_READ_PIXEL_IMAGE
  int byteCount = 2 * pixelImage16.width * pixelImage16.height;
  int maxI = byteCount / 2;
  if (maxI > 100) {
    maxI = 100;
  }
  for (int i = 0; i < maxI; i++) {
    uint16_t v = pixelImage16.data[i];
    if (v != 0) {
      Serial.print("/");
      Serial.print(i);
      Serial.print(":");
      Serial.print(v, HEX);
    }
  }
  Serial.println();
#endif
  return true;
}
bool ImageRetrieverDDTunnel::readJpegImage(DDJpegImage& jpeg) {
  return _readImageData(jpeg, 3);
}



JsonDDTunnelMultiplexer::JsonDDTunnelMultiplexer(JsonDDTunnel** tunnels, int8_t tunnelCount) {
  this->tunnelCount = tunnelCount;
  //this->tunnels = tunnels;
  this->tunnels = (JsonDDTunnel**) malloc(tunnelCount * sizeof(JsonDDTunnel*));
  memcpy(this->tunnels, tunnels, tunnelCount * sizeof(JsonDDTunnel*));

}
JsonDDTunnelMultiplexer::~JsonDDTunnelMultiplexer() {
  free(this->tunnels);
}
int JsonDDTunnelMultiplexer::count() {
  int count = 0;
  for (int i = 0; i < tunnelCount; i++) {
    if (tunnels[i] != NULL) {
      count += tunnels[i]->count();
    }
  }
  return count;
}
bool JsonDDTunnelMultiplexer::eof() {
  for (int i = 0; i < tunnelCount; i++) {
    if (tunnels[i] != NULL) {
      if (!tunnels[i]->eof()) return false;
    }
  }
  return true;
}
int JsonDDTunnelMultiplexer::read(String& fieldId, String& fieldValue) {
  for (int i = 0; i < tunnelCount; i++) {
    if (tunnels[i] != NULL) {
      if (tunnels[i]->count() > 0) {
        if (tunnels[i]->read(fieldId, fieldValue)) {
          return i;
        }
      }
    }
  }
  return -1;
}
void JsonDDTunnelMultiplexer::release() {
  for (int i = 0; i < tunnelCount; i++) {
    if (tunnels[i] != NULL) {
      tunnels[i]->release();
    }
  }
}
void JsonDDTunnelMultiplexer::reconnect() {
  for (int i = 0; i < tunnelCount; i++) {
    if (tunnels[i] != NULL) {
      tunnels[i]->reconnect();
    }
  }
}
//#endif


void DumbDisplay::initialize(DDInputOutput* pIO, uint16_t sendBufferSize, long idleTimeout/*, bool enableDoubleClick*/) {
  _SetIO(pIO, sendBufferSize, idleTimeout);
  //_DDIO = pIO;
  //_EnableDoubleClick = enableDoubleClick;
}
void DumbDisplay::connect() {
  _Connect();
}
bool DumbDisplay::connected() const {
  return _Connected;
}
int DumbDisplay::getConnectVersion() const {
  return _ConnectVersion;
}
int DumbDisplay::getCompatibilityVersion() const {
  return _DDCompatibility;
}
// bool DumbDisplay::checkReconnecting() const {
//   _Yield();
//   //_ValidateConnection();
//   //return false;
//   return _ConnectedIOProxy != NULL &&_ConnectedIOProxy->isReconnecting();
// }
void DumbDisplay::configPinFrame(int xUnitCount, int yUnitCount, bool autoControlLayerVisible) {
  _Connect();
  if (_DDCompatibility >= 7) {
      _sendCommand3("", "CFGPF", String(xUnitCount), String(yUnitCount), TO_BOOL(autoControlLayerVisible));
  } else {
    _sendCommand2("", "CFGPF", String(xUnitCount), String(yUnitCount));
  }
}
void DumbDisplay::configAutoPin(const String& layoutSpec, bool autoControlLayerVisible) {
  _Connect();
  if (true) {
    if (layoutSpec.c_str() == NULL) {
      __SendErrorComment("invalid autopin config");
      return;
    }
  }
  if (_DDCompatibility >= 7) {
    _sendCommand2("", "CFGAP", layoutSpec, TO_BOOL(autoControlLayerVisible));
  } else {
    _sendCommand1("", "CFGAP", layoutSpec);
  }
}
// void DumbDisplay::configAutoPinEx(const String& layoutSpec, const String& remainingLayoutSpec) {
//   _Connect();
//   if (true) {
//     if (layoutSpec.c_str() == NULL) {
//       __SendErrorComment("invalid autopin config");
//       return;
//     }
//   }
//   _sendCommand2("", "CFGAP", layoutSpec, remainingLayoutSpec);
// }
void DumbDisplay::addRemainingAutoPinConfig(const String& remainingLayoutSpec) {
  _Connect();
  _sendCommand1("", "ADDRESTAP", remainingLayoutSpec);
}
void DumbDisplay::setFeedbackSingleClickOnly(bool singleClickOnly) {
  _Connect();
  _sendCommand1("", "SETFBSCO", TO_BOOL(singleClickOnly));
}
// void DumbDisplay::enableFeedbackDoubleClick(bool enable) {
//   //_EnableDoubleClick = enable;
//   _Connect();
//   _sendCommand1("", "ENBFBDBLCLK", TO_BOOL(enable));
// }
MbDDLayer* DumbDisplay::createMicrobitLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("mb"), String(width), String(height));
  MbDDLayer* pLayer = new MbDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
TurtleDDLayer* DumbDisplay::createTurtleLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("turtle"), String(width), String(height));
  TurtleDDLayer* pLayer = new TurtleDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
LedGridDDLayer* DumbDisplay::createLedGridLayer(int colCount, int rowCount, int subColCount, int subRowCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("ledgrid"), String(colCount), String(rowCount), String(subColCount), String(subRowCount));
  LedGridDDLayer* pLayer = new LedGridDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
LedGridDDLayerHandle DumbDisplay::createLedGridLayerHandle(int colCount, int rowCount, int subColCount, int subRowCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("ledgrid"), String(colCount), String(rowCount), String(subColCount), String(subRowCount));
  LedGridDDLayerHandle handle;
  handle._h = lid;
  return handle;
}
LcdDDLayer* DumbDisplay::createLcdLayer(int colCount, int rowCount, int charHeight, const String& fontName) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("lcd"), String(colCount), String(rowCount), String(charHeight), fontName);
  LcdDDLayer* pLayer = new LcdDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
LcdDDLayerHandle DumbDisplay::createLcdLayerHandle(int colCount, int rowCount, int charHeight, const String& fontName) {
  int lid = _AllocLid();
  _sendCommand5(String(lid), "SU", String("lcd"), String(colCount), String(rowCount), String(charHeight), fontName);
  LcdDDLayerHandle handle;
  handle._h = lid;
  return handle;
}
SelectionDDLayer* DumbDisplay::createSelectionLayer(int colCount, int rowCount,
                                                    int horiSelectionCount, int vertSelectionCount,
                                                    int charHeight, const String& fontName,
                                                    bool canDrawDots, float selectionBorderSizeCharHeightFactor) {
  int lid = _AllocLid();
  String layerId = String(lid);
  if (_DDCompatibility >= 9) {
    _sendCommand9(layerId, "SU", String("selection"), String(colCount), String(rowCount), String(horiSelectionCount), String(vertSelectionCount), String(charHeight), fontName, TO_BOOL(canDrawDots), TO_NUM(selectionBorderSizeCharHeightFactor));
  } else {
    _sendCommand8(layerId, "SU", String("selection"), String(colCount), String(rowCount), String(horiSelectionCount), String(vertSelectionCount), String(charHeight), fontName, TO_NUM(selectionBorderSizeCharHeightFactor));
  }
  SelectionDDLayer* pLayer = new SelectionDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
SelectionListDDLayer* DumbDisplay::createSelectionListLayer(int colCount, int rowCount, 
                                                            int horiSelectionCount, int vertSelectionCount,
                                                            int charHeight, const String& fontName,
                                                            bool canDrawDots, float selectionBorderSizeCharHeightFactor) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand9(layerId, "SU", String("selectionlist"), String(colCount), String(rowCount), String(horiSelectionCount), String(vertSelectionCount), String(charHeight), fontName, TO_BOOL(canDrawDots), TO_NUM(selectionBorderSizeCharHeightFactor));
  SelectionListDDLayer* pLayer = new SelectionListDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
#ifdef SUPPORT_CONTAINER
GraphicalDDLayer* DumbDisplay::setRootLayer(int width, int height, const String& containedAlignment) {
  _Connect();
  _sendCommand3("", "ROOT", String(width), String(height), containedAlignment);
  if (__RootLayer != NULL) {
    delete __RootLayer;
  }
  __RootLayer = new GraphicalDDLayer(CONTAINER_LAYER_ID);
  return __RootLayer;
}
#endif
GraphicalDDLayer* DumbDisplay::createGraphicalLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("graphical"), String(width), String(height));
  GraphicalDDLayer* pLayer = new GraphicalDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
GraphicalDDLayerHandle DumbDisplay::createGraphicalLayerHandle(int width, int height) {
  int lid = _AllocLid();
  _sendCommand3(String(lid), "SU", String("graphical"), String(width), String(height));
  GraphicalDDLayerHandle handle;
  handle._h = lid;
  return handle;
}SevenSegmentRowDDLayer* DumbDisplay::create7SegmentRowLayer(int digitCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand2(layerId, "SU", String("7segrow"), String(digitCount));
  SevenSegmentRowDDLayer* pLayer = new SevenSegmentRowDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
// @param stickValueDivider the divider of the stick value; 1 by default
JoystickDDLayer* DumbDisplay::createJoystickLayer(int maxStickValue, const String& directions, float stickSizeFactor/*, int stickValueDivider*/) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("joystick"), String(maxStickValue),  directions, TO_NUM(stickSizeFactor));
  //_sendCommand5(layerId, "SU", String("joystick"), String(maxStickValue),  directions, TO_NUM(stickSizeFactor), String(stickValueDivider));
  JoystickDDLayer* pLayer = new JoystickDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
PlotterDDLayer* DumbDisplay::createPlotterLayer(int width, int height, int pixelsPerSecond) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("plotter"), String(width), String(height), String(pixelsPerSecond));
  PlotterDDLayer* pLayer = new PlotterDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
PlotterDDLayer* DumbDisplay::createFixedRatePlotterLayer(int width, int height, int pixelsPerScale) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("fixedrateplotter"), String(width), String(height), String(pixelsPerScale));
  PlotterDDLayer* pLayer = new PlotterDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
TomTomMapDDLayer* DumbDisplay::createTomTomMapLayer(const String& mapKey, int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("tomtommap"), mapKey, String(width), String(height));
  TomTomMapDDLayer* pLayer = new TomTomMapDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
TerminalDDLayer* DumbDisplay::createTerminalLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("terminal"), String(width), String(height));
  TerminalDDLayer* pLayer = new TerminalDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}

WebViewDDLayer* DumbDisplay::createWebViewLayer(int width, int height, const String& jsObjectName ) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("webview"), String(width), String(height), jsObjectName);
  WebViewDDLayer* pLayer = new WebViewDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}

DumbDisplayWindowDDLayer* DumbDisplay::createDumbDisplayWindowLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("dumbdisplaywin"), String(width), String(height));
  DumbDisplayWindowDDLayer* pLayer = new DumbDisplayWindowDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;  
}

RtspClientDDLayer* DumbDisplay::createRtspClient(int width, int height) {

  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("rtspclient"), String(width), String(height));
  RtspClientDDLayer* pLayer = new RtspClientDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;    
}




void DumbDisplay::pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand5(pLayer->getLayerId(), "PIN", String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand6("", "PINAP", layoutSpec, String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::resetPinLayers() {
  _sendCommand0("", "RESETPIN");
}
void DumbDisplay::deleteLayer(DDLayer *pLayer) {
  _sendCommand0(pLayer->getLayerId(), "DEL");
  delete pLayer;  // will call _PreDeleteLayer(pLayer)
}
void DumbDisplay::deleteLayer(DDLayerHandle layerHandle) {
  _sendCommand0(String(layerHandle._h), "DEL");
}
void DumbDisplay::reorderLayer(DDLayer *pLayer, const String& how) {
  _sendCommand1(pLayer->getLayerId(), "REORD", how);
}
void DumbDisplay:: walkLayers(void (*walker)(DDLayer *)) {
  for (int i = 0; i < _NextLid; i++) {
    DDObject* pObject = _DDLayerArray[i];
    if (pObject != NULL) {
      if (pObject->objectType == DD_OBJECT_TYPE_LAYER) {
        walker((DDLayer*) pObject);
      }
    }
  }
}
void DumbDisplay::recordLayerSetupCommands() {
  _Connect();
  _sendCommand0("", C_RECC);
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WODDIO)->setKeepBuffering(true);
  }
#endif
}
void DumbDisplay::playbackLayerSetupCommands(const String& layerSetupPersistId) {
  _sendCommand2("", C_SAVEC, layerSetupPersistId, TO_BOOL(true));
  _sendCommand0("", C_PLAYC);
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WODDIO)->setKeepBuffering(false);
    ((DDWriteOnyIO*) _WODDIO)->flush();
  }
#endif
#ifdef SUPPORT_RECONNECT
  _ConnectedIOProxy->setReconnectRCId(layerSetupPersistId);
#endif
}
void DumbDisplay::recordLayerCommands() {
  _Connect();
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WODDIO)->setKeepBuffering(true);
  }
#endif
  _sendCommand0("", C_RECC);
}
void DumbDisplay::playbackLayerCommands() {
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WODDIO)->setKeepBuffering(false);
  }
#endif
  _sendCommand0("", C_PLAYC);
}
void DumbDisplay::stopRecordLayerCommands(const String& saveId, bool persistSave) {
  if (_DDCompatibility >= 11 && saveId != "") {
    _sendCommand2("", "STOPC", saveId, TO_BOOL(persistSave));
  } else {
    _sendCommand0("", "STOPC");
  }
}
void DumbDisplay::saveLayerCommands(const String& id, bool persist, bool stopAfterSave) {
  if (_DDCompatibility >= 11 && stopAfterSave) {
    _sendCommand3("", C_SAVEC, id, TO_BOOL(persist), TO_BOOL(stopAfterSave));
  } else {
    _sendCommand2("", C_SAVEC, id, TO_BOOL(persist));
  }
}
void DumbDisplay::loadLayerCommands(const String& id) {
  _sendCommand1("", "LOADC", id);
}
void DumbDisplay::capture(const String& imageFileName, int width, int height) {
  _sendCommand3("", C_CAPTURE, imageFileName, String(width), String (height));
}
void DumbDisplay::backgroundColor(const String& color) {
  _Connect();
  _sendCommand1("", "BGC", color);
}
void DumbDisplay::sendNoOp() {
    _sendCommand0("", C_KAL);
}
void DumbDisplay::writeComment(const String& comment) {
  _Connect();
  //_sendCommand0("", ("// " + comment).c_str());
  if (true) {
    int idx = comment.indexOf('\n');
    if (idx != -1) {
        String com1 = comment.substring(0, idx);
        String com2 = comment.substring(idx + 1);
        _sendCommand0("", ("// " + com1).c_str());
        writeComment(com2);
    } else {
      _sendCommand0("", ("// " + comment).c_str());
    }  
  } else {
    _sendCommand0("", ("// " + comment).c_str());
  }
}
void DumbDisplay::tone(uint32_t freq, uint32_t duration) {
  if (false) {  // false since 20240810
    _Connect();
  }
  _sendCommand2("", C_TONE, TO_C_INT(freq), TO_C_INT(duration));
}
void DumbDisplay::notone() {
  if (false) {  // false since 20240810
    _Connect();
  }
  _sendCommand0("", C_NOTONE);
}
void DumbDisplay::saveSound8(const String& soundName, const int8_t *bytes, int sampleCount, int sampleRate, int numChannels) {
  int byteCount = sampleCount;
  _sendCommand5("", C_SAVESND, soundName, String(sampleRate), String(8), String(numChannels), TO_EDIAN());
  _sendByteArrayAfterCommand((uint8_t*) bytes, byteCount);
}
int DumbDisplay::saveSoundChunked8(const String& soundName/*, const int8_t *bytes, int sampleCount*/, int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  uint8_t noData[0];
  _sendCommand6("", C_SAVESND, soundName, String(sampleRate), String(8), String(numChannels), TO_EDIAN(), bytesId);
  _sendByteArrayAfterCommandChunked(bytesId, noData, 0);
  return bid;
}
int DumbDisplay::streamSound8(int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  _sendCommand5("", C_STREAMSND, String(sampleRate), String(8), String(numChannels), TO_EDIAN(), bytesId);
  return bid;
} 
void DumbDisplay::saveSound16(const String& soundName, const int16_t *data, int sampleCount, int sampleRate, int numChannels) {
  int byteCount = 2 * sampleCount;
  _sendCommand5("", C_SAVESND, soundName, String(sampleRate), String(16), String(numChannels), TO_EDIAN());
  _sendByteArrayAfterCommand((uint8_t*) data, byteCount);
}
int DumbDisplay::saveSoundChunked16(const String& soundName/*, const int16_t *data, int sampleCount*/, int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  uint8_t noData[0];
  _sendCommand6("", C_SAVESND, soundName, String(sampleRate), String(16), String(numChannels), TO_EDIAN(), bytesId);
  _sendByteArrayAfterCommandChunked(bytesId, noData, 0);
  return bid;
}
void DumbDisplay::cacheSound8(const String& soundName, const int8_t *bytes, int sampleCount, int sampleRate, int numChannels) {
  int byteCount = sampleCount;
  _sendCommand5("", C_CACHESND, soundName, String(sampleRate), String(8), String(numChannels), TO_EDIAN());
  _sendByteArrayAfterCommand((uint8_t*) bytes, byteCount);
}
int DumbDisplay::cacheSoundChunked8(const String& soundName/*, const int8_t *bytes, int sampleCount*/, int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  uint8_t noData[0];
  _sendCommand6("", C_CACHESND, soundName, String(sampleRate), String(8), String(numChannels), TO_EDIAN(), bytesId);
  _sendByteArrayAfterCommandChunked(bytesId, noData, 0);
  return bid;
}
void DumbDisplay::sendSoundChunk8(int chunkId, const int8_t *bytes, int sampleCount, bool isFinal) {
  String bytesId = String(chunkId);
  int byteCount = sampleCount;
  _sendByteArrayAfterCommandChunked(bytesId, (uint8_t*) bytes, byteCount, isFinal);
}
void DumbDisplay::cacheSound16(const String& soundName, const int16_t *data, int sampleCount, int sampleRate, int numChannels) {
  int byteCount = 2 * sampleCount;
  _sendCommand5("", C_CACHESND, soundName, String(sampleRate), String(16), String(numChannels), TO_EDIAN());
  _sendByteArrayAfterCommand((uint8_t*) data, byteCount);
}
int DumbDisplay::cacheSoundChunked16(const String& soundName/*, const int16_t *data, int sampleCount*/, int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  uint8_t noData[0];
  _sendCommand6("", C_CACHESND, soundName, String(sampleRate), String(16), String(numChannels), TO_EDIAN(), bytesId);
  _sendByteArrayAfterCommandChunked(bytesId, noData, 0);
  return bid;
}
int DumbDisplay::streamSound16(int sampleRate, int numChannels) {
  int bid = _AllocBytesId();
  String bytesId = String(bid);
  //int byteCount = 2 * sampleCount;
  _sendCommand5("", C_STREAMSND, String(sampleRate), String(16), String(numChannels), TO_EDIAN(), bytesId);
  return bid;
}
void DumbDisplay::sendSoundChunk16(int chunkId, const int16_t *data, int sampleCount, bool isFinal) {
  String bytesId = String(chunkId);
  int byteCount = 2 * sampleCount;
  _sendByteArrayAfterCommandChunked(bytesId, (uint8_t*) data, byteCount, isFinal);
}
void DumbDisplay::saveCachedSound(const String& soundName) {
  _sendCommand1("", "SAVECACHEDSND", soundName);
}
void DumbDisplay::saveCachedSoundAsH(const String& soundName) {
  _sendCommand2("", "SAVECACHEDSNDASH", soundName, TO_EDIAN());
}
void DumbDisplay::playSound(const String& soundName) {
  _sendCommand1("", C_PLAYSND, soundName);
}
void DumbDisplay::stopSound() {
  _sendCommand0("", C_STOPSND);
}
void DumbDisplay::saveImage(const String& imageName, const uint8_t *bytes, int byteCount) {
  _sendCommand1("", C_SAVEIMG, imageName);
  _sendByteArrayAfterCommand((uint8_t*) bytes, byteCount);
}
void DumbDisplay::savePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color, char compressMethod) {
  //int byteCount = width * height / 8; 
  int byteCount = (width + 7) * height / 8; 
  _sendCommand4("", C_SAVEPIXIMG, imageName, String(width), String(height), color);
  _sendByteArrayAfterCommand(bytes, byteCount, compressMethod);
}
void DumbDisplay::savePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options, char compressMethod) {
  int byteCount = 2 * width * height; 
  _sendCommand5("", C_SAVEPIXIMG16, imageName, String(width), String(height), TO_EDIAN(), options);
  _sendByteArrayAfterCommand((uint8_t*) data, byteCount, compressMethod);
}
void DumbDisplay::savePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options, char compressMethod) {
  int byteCount = width * height; 
  _sendCommand4("", C_SAVEPIXIMGGS, imageName, String(width), String(height), options);
  _sendByteArrayAfterCommand(data, byteCount, compressMethod);
}
void DumbDisplay::stitchImages(const String& imageNames, const String& asImageName) {
  _sendCommand2("", "STITCHIMGS", imageNames, asImageName);
}
void DumbDisplay::alert(const String& message, const String& title) {
  _sendCommand2("", C_ALERT, title, message);
}
void DumbDisplay::debugOnly(int i) {
  _sendCommand2("", "DEBUGONLY", String(i), TO_C_INT(i));
  uint8_t bytes[i];
  for (int j = 0; j < i; j++) {
    bytes[j] = (uint8_t) (j % 256);
    // if (i <= 10) {
    //   bytes[j] = (uint8_t) (128 + (j % 128));
    // } else {
    //   bytes[j] = (uint8_t) (j % 256);
    // }
  }
  _sendByteArrayAfterCommand(bytes, i);
}
BasicDDTunnel* DumbDisplay::createBasicTunnel(const String& endPoint, bool connectNow, int8_t bufferSize) {
#ifdef SUPPORT_TUNNEL
  int tid = _AllocTid();
  String tunnelId = String(tid);
  // if (connectNow) {
  //   _sendSpecialCommand("lt", tunnelId, "connect", "ddbasic@" + endPoint);
  // }
  BasicDDTunnel* pTunnel = new BasicDDTunnel("ddbasic", tid, "", endPoint/*, connectNow*/, bufferSize);
  _PostCreateTunnel(pTunnel, connectNow);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}
JsonDDTunnel* DumbDisplay::createJsonTunnel(const String& endPoint, bool connectNow, int8_t bufferSize) {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  // if (connectNow) {
  //   _sendSpecialCommand("lt", tunnelId, "connect", "ddsimplejson@" + endPoint);
  // }
  JsonDDTunnel* pTunnel = new JsonDDTunnel("ddsimplejson", tid, "", endPoint/*, connectNow*/, bufferSize);
  _PostCreateTunnel(pTunnel, connectNow);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}
JsonDDTunnel* DumbDisplay::createFilteredJsonTunnel(const String& endPoint, const String& fieldNames, bool connectNow, int8_t bufferSize) {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  JsonDDTunnel* pTunnel = new JsonDDTunnel("ddsimplejson", tid, fieldNames, endPoint/*, connectNow*/, bufferSize);
  _PostCreateTunnel(pTunnel, connectNow);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}
ImageDownloadDDTunnel* DumbDisplay::createImageDownloadTunnel(const String& endPoint, const String& imageName, bool redownload) {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  String params = imageName;
  if (!redownload) {
    params.concat(",NRDL");
    //params = params + ",NRDL";
  }
#ifdef DEBUG_MISSING_ENDPOINT_C  
_sendCommand0("", ("// CreateEP -- " + endPoint).c_str());
#endif
   ImageDownloadDDTunnel* pTunnel = new ImageDownloadDDTunnel(tid, params, endPoint/*, true*/, 1);
  _PostCreateTunnel(pTunnel, true);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}
BasicDDTunnel* DumbDisplay::createDateTimeServiceTunnel() {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  BasicDDTunnel* pTunnel = new BasicDDTunnel("datetimeservice", tid, "", ""/*, false*/, 1);
  _PostCreateTunnel(pTunnel, false);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}
BasicDDTunnel* DumbDisplay::createGeneralServiceTunnel() {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  BasicDDTunnel* pTunnel = new BasicDDTunnel("generalservice", tid, "", ""/*, false*/, 1);
  _PostCreateTunnel(pTunnel, false);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}

GpsServiceDDTunnel* DumbDisplay::createGpsServiceTunnel() {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  GpsServiceDDTunnel* pTunnel = new GpsServiceDDTunnel("gpsservice", tid, "", ""/*, false*/, 1);
  _PostCreateTunnel(pTunnel, false);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}

ObjectDetectDemoServiceDDTunnel* DumbDisplay::createObjectDetectDemoServiceTunnel(int scaleToWidth, int scaleToHeight, int maxNumObjs) {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  String params;
  if (scaleToWidth > 0 && scaleToHeight > 0) {
    params = String(scaleToWidth) + "," + String(scaleToHeight) + ",";
  }
  params = params + "mno=" + String(maxNumObjs);
  ObjectDetectDemoServiceDDTunnel* pTunnel = new ObjectDetectDemoServiceDDTunnel("objectdetectdemo", tid, params, ""/*, false*/, DD_TUNNEL_DEF_BUFFER_SIZE);
  _PostCreateTunnel(pTunnel, false);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}

ImageRetrieverDDTunnel* DumbDisplay::createImageRetrieverTunnel() {
#ifdef SUPPORT_TUNNEL	
  int tid = _AllocTid();
  String tunnelId = String(tid);
  ImageRetrieverDDTunnel* pTunnel = new ImageRetrieverDDTunnel("imageretriever", tid, TO_EDIAN(), ""/*, false*/, DD_TUNNEL_DEF_BUFFER_SIZE);
  _PostCreateTunnel(pTunnel, false);
  return pTunnel;
#else
  _sendTunnelDisabledComment();
  return NULL;
#endif
}

void DumbDisplay::deleteTunnel(DDTunnel *pTunnel) {
#ifdef SUPPORT_TUNNEL	
  pTunnel->release();
  delete pTunnel;  // will call _PreDeleteTunnel
#endif
}


// bool DumbDisplay::canLogToSerial() {
//   return _CanLogToSerial();
// }

// #ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
// void DumbDisplay::optionNoCompression(bool noCompression) {
//   _NoEncodeInt = noCompression;
// }
//#endif
#ifndef DD_NO_IDLE_CALLBACK
void DumbDisplay::setIdleCallback(DDIdleCallback idleCallback) {
#ifdef SUPPORT_IDLE_CALLBACK
  _IdleCallback = idleCallback;
#endif
}
#endif

#ifndef DD_NO_CONNECT_VERSION_CHANGED_CALLBACK
void DumbDisplay::setConnectVersionChangedCallback(DDConnectVersionChangedCallback connectVersionChangedCallback) {
#ifdef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
  _ConnectVersionChangedCallback = connectVersionChangedCallback;
#endif
}
#endif

// void DumbDisplay::delay(unsigned long ms) {
//   _Delay(ms);
// }
bool DumbDisplay::canPrintToSerial() {
  return _CanLogToSerial();
}
void DumbDisplay::logToSerial(const String& logLine, bool force) {
  if (_CanLogToSerial()) {
#ifdef DDIO_USE_DD_SERIAL
    if (_The_DD_Serial != NULL) {
      _The_DD_Serial->print(logLine);
      _The_DD_Serial->print("\n");
    } else {
      Serial.println(logLine);
    }
#else
    Serial.println(logLine);
#endif    
  } else {
    if (true) { 
      if (force) {
        Serial.println(logLine);
      }
    } else {
      if (_Connected) {
        writeComment(logLine);
      } else if (force) {
        Serial.println(logLine);
      }
    }
  }
}
void DumbDisplay::log(const String& logLine, bool isError) {
  if (_CanLogToSerial()) {
#ifdef DDIO_USE_DD_SERIAL
    if (_The_DD_Serial != NULL) {
      _The_DD_Serial->print(logLine);
      _The_DD_Serial->print("\n");
    } else {
      Serial.println(logLine);
    }
#else
    Serial.println(logLine);
#endif
  }
  if (_Connected) {
    __SendComment(logLine, isError);
  }  
}


#ifndef DD_NO_PASSIVE_CONNECT
bool DumbDisplay::connectPassive(DDConnectPassiveStatus* pStatus) {
#ifdef SUPPORT_PASSIVE
  bool connected = _Connect(true);
  if (pStatus != NULL) {
    pStatus->connected = connected;
    pStatus->connecting = false;
    pStatus->reconnecting = false;
  }
  if (!connected && pStatus != NULL) {
    pStatus->connecting = _C_state.step >= _C_HANDSHAKE/*_C_HANDSHAKE*/; 
  }
  if (connected && pStatus != NULL) {
    _Yield();
    pStatus->reconnecting = _ConnectedIOProxy != NULL &&_ConnectedIOProxy->isReconnecting();
  }
// Serial.print("$");
//Serial.print(_C_state.step);
  return connected;
#else
  return false;
#endif  
}
#endif
// void DumbDisplay::savePassiveConnectState(DDSavedConnectPassiveState& state) {
//   state.initialized = 12345;
//   state.step = _C_state.step;
//   state.startMillis =_C_state.startMillis;
//   state.lastCallMillis = _C_state.lastCallMillis;
//   state.firstCall = _C_state.firstCall;
//   state.hsStartMillis = _C_state.hsStartMillis;
//   state.hsNextMillis = _C_state.hsNextMillis;
// }
// void DumbDisplay::restorePassiveConnectState(DDSavedConnectPassiveState& state) {
//   if (state.initialized == 12345) {
//     if (true) {
//       state.step = 0;
//     }
//     _C_state.step = state.step;
//     _C_state.startMillis = state.startMillis;
//     _C_state.lastCallMillis = state.lastCallMillis;
//     _C_state.firstCall = state.firstCall;
//     _C_state.hsStartMillis = state.hsStartMillis;
//     _C_state.hsNextMillis = state.hsNextMillis;
//     // if (state.step >= _C_IOPROXY_SET) {
//     //   __C_SetupIOProxy();
//     // }
// // Serial.print("-----");
// // Serial.println(state.step);
//   }
// }
// bool DumbDisplay::connectPassive(bool* pReconnecting) {
// #ifdef SUPPORT_PASSIVE
//   bool connected = _Connect(true);
//   if (pReconnecting != NULL) {
//     _Yield();
//     *pReconnecting = _ConnectedIOProxy != NULL &&_ConnectedIOProxy->isReconnecting();
//   }
//   return connected;
// #else
//   return false;
// #endif  
// }
#ifndef DD_NO_PASSIVE_CONNECT
void DumbDisplay::masterReset() {
#ifdef SUPPORT_MASTER_RESET
  bool reconnecting = _ConnectedIOProxy != NULL && _ConnectedIOProxy->isReconnecting();
  //bool canLogToSerial = !_DDIO->isSerial();
  bool canLogToSerial = _CanLogToSerial();

#ifdef MASTER_RESET_KEEP_CONNECTED
  bool canKeepConnected = _Connected && !reconnecting && _DDCompatibility >= 9;
#endif

  if (canLogToSerial) {
    Serial.println();
#ifdef MASTER_RESET_KEEP_CONNECTED
    if (canKeepConnected) {
      Serial.println("***** Keep Connection Master Reset (START) *****");
    } else {
      Serial.println("***** Master Reset (START) *****");
    }
#else
    Serial.println("***** Master Reset (START) *****");
#endif    
    if (reconnecting) {
      Serial.println("- during reconnecting");
    }
  }

#ifdef MASTER_RESET_KEEP_CONNECTED
  if (canKeepConnected) {
    if (canLogToSerial) Serial.println(". still connected ... send command to DD app to master reset");
    _sendCommand0("", "MASTERRESET");
  } else {
      if (canLogToSerial) Serial.println(". send command to DD app to disconnect");
      _sendCommand0("", "DISCONNECT");
  }
#else  
    // try the best to delete objects tracked
    if (_Connected && !reconnecting) {
      if (canLogToSerial) Serial.println(". send command to DD app to disconnect");
      _sendCommand0("", "DISCONNECT");
    }
#endif

#ifdef SUPPORT_CONTAINER
  if (__RootLayer != NULL) {
    //delete __RootLayer;  // TODO: seems deleting __RootLayer will cause hang for Pico ... ref: Arduino Clock
    __RootLayer = NULL;
  }
#endif
  if (_NextLid > 0) {
    if (canLogToSerial) Serial.println(". cleanup layers / tunnels");
    for (int i = 0; i < _NextLid; i++) {
      DDObject* pObject = _DDLayerArray[i];
      if (pObject != NULL) {
        delete pObject;
      }
    }
#ifdef USE_MALLOC_FOR_LAYER_ARRAY
    free(_DDLayerArray);
#else     
    delete _DDLayerArray;
#endif    
    _DDLayerArray = NULL;
#ifdef SUPPORT_TUNNEL
    _MaxDDLayerCount = 0;
#endif
  }
  _NextLid = 0;
  _NextImgId = 0;  // allocated image not tracked
  _NextBytesId = 0;
  //_DDIO = NULL;
// #ifdef SUPPORT_USE_WOIO
//   if (_WODDIO != NULL) {
// 	  delete _WODDIO;
//     _WODDIO = NULL;
//   }  
// #endif

#ifdef MASTER_RESET_KEEP_CONNECTED
  if (canLogToSerial) Serial.println(String(". reset states ... canKeepConnected=") + String(canKeepConnected));
  if (!canKeepConnected) {
    if (_ConnectedIOProxy != NULL) {
      delete _ConnectedIOProxy;
      _ConnectedIOProxy = NULL;
    }
    _Connected = false;
    _ConnectVersion = 0;
    _C_state.step = 0;
  }/* else {
    _C_state.step = _C_MASTER_RESET;
  }*/
  _SendingCommand = false;
  _HandlingFeedback = false;
#else
  if (canLogToSerial) Serial.println(". reset states");
  if (_ConnectedIOProxy != NULL) {
    delete _ConnectedIOProxy;
    _ConnectedIOProxy = NULL;
  }
  _SendingCommand = false;
  _HandlingFeedback = false;
  _Connected = false;
  _ConnectVersion = 0;
  _C_state.step = 0;
#endif  

  if (canLogToSerial) {
#ifdef MASTER_RESET_KEEP_CONNECTED
    if (canKeepConnected) {
      Serial.println("***** Keep Connection Master Reset (END) *****");
    } else {
      Serial.println("***** Master Reset (END) *****");
    }
#else
    Serial.println("***** Master Reset (END) *****");
#endif    
    Serial.println();
  }
#endif
}
#endif

//void DumbDisplay::debugSetup(int debugLedPin/*, bool enableEchoFeedback*/) {
#ifndef DD_NO_DEBUG_INTERFACE
void DumbDisplay::debugSetup(DDDebugInterface *debugInterface) {
#ifdef SUPPORT_DEBUG_INTERFACE
  _DebugInterface = debugInterface;
#endif
// #ifdef DEBUG_WITH_LED
//   if (debugLedPin != -1) {
//      pinMode(debugLedPin, OUTPUT);
//    }
//   _DebugLedPin = debugLedPin;
// #endif  
#ifdef DEBUG_ECHO_FEEDBACK
  _DebugEnableEchoFeedback = true;//enableEchoFeedback;
#endif
}
#endif
void DDLayer::debugOnly(int i) {
  _sendCommand2(layerId, "debugonly", String(i), TO_C_INT(i));
  // byte bytes[i];
  // for (int j = 0; j < i; j++) {
  //   bytes[j] = j;
  // }
  // _sendByteArrayAfterCommand(bytes, i);
}

// void DDLogToSerial(const String& logLine) {
//    _LogToSerial(logLine);
// }
bool DDConnected() {
  return _Connected;
}
void DDDelay(unsigned long ms) {
#ifdef HANDLE_FEEDBACK_DURING_DELAY
  _Delay(ms);
#else
  delay(ms);
#endif
}
void DDYield() {
  _Yield();
}

void DDDebugOnly(int32_t i) {
  Serial.print(i);
  Serial.print(" ==> ");
  Serial.println(TO_C_INT(i));
}



