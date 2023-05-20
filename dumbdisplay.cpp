#include "Arduino.h"

#include "dumbdisplay.h"


#define HAND_SHAKE_GAP 1000
#define VALIDATE_GAP 2000

#define ENABLE_FEEDBACK
#define STORE_LAYERS
#define HANDLE_FEEDBACK_DURING_DELAY

#define FEEDBACK_BUFFER_SIZE 4

#define READ_BUFFER_USE_BUFFER

#define MORE_KEEP_ALIVE

#define SUPPORT_ENCODE_OPER

// #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
//   #define PGM_READ_BYTERS
// #endif

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#define TL_BUFFER_DATA_LEN 24
#else
#define TL_BUFFER_DATA_LEN 128
#define SUPPORT_USE_WOIO
#endif


#define TO_BOOL(val) (val ? "1" : "0")
#define TO_EDIAN() String(DDCheckEndian())


//#define DD_DEBUG_HS
//#define DD_DEBUG_SEND_COMMAND
//#define DEBUG_ECHO_COMMAND
//#define DEBUG_RECEIVE_FEEDBACK
//#define DEBUG_ECHO_FEEDBACK
//#define DEBUG_VALIDATE_CONNECTION
//#define DEBUG_TUNNEL_RESPONSE
//#define DEBUG_SHOW_FEEDBACK


//#define SUPPORT_LONG_PRESS_FEEDBACK

#define SUPPORT_IDLE_CALLBACK
#define SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK

#define SUPPORT_TUNNEL
#define TUNNEL_TIMEOUT_MILLIS 30000


#define VALIDATE_CONNECTION
#define DEBUG_WITH_LED

#define SUPPORT_RECONNECT
#define RECONNECT_NO_KEEP_ALIVE_MILLIS 5000

//#define SHOW_KEEP_ALIVE
//#define DEBUG_RECONNECT_WITH_COMMENT
//#define RECONNECTED_RESET_KEEP_ALIVE


// not flush seems to be a bit better for Serial (lost data)
#define FLUSH_AFTER_SENT_COMMAND false
#define YIELD_AFTER_SEND_COMMAND false
#define YIELD_AFTER_HANDLE_FEEDBACK true

// see nobody.trevorlee.dumbdisplay.DDActivity#ddSourceCompatibility
#define DD_SID "Arduino-c7"


#include "_dd_commands.h"


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
        for (int i = 0; i < size; i++) {
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


DDSerial* _The_DD_Serial = NULL;

namespace DDImpl {


class IOProxy {
  public: 
    IOProxy(DDInputOutput *pIO) {
      this->pIO = pIO;
#ifdef SUPPORT_RECONNECT      
      this->lastKeepAliveMillis = 0;
      this->reconnectEnabled = false;
#endif
    }
    bool available();
    const String& get();
    void clear();
    void print(const String &s);
    void print(const char *p);
    void write(uint8_t b);
    void keepAlive();
    void validConnection();
    void setReconnectRCId(const String& rcId) {
#ifdef SUPPORT_RECONNECT      
      this->reconnectRCId = rcId;
      this->reconnectEnabled = true;
      this->reconnectKeepAliveMillis = 0;
#endif
    }
  private:
    DDInputOutput *pIO;
    bool fromSerial;
    String data;  
#ifdef SUPPORT_RECONNECT      
    unsigned long lastKeepAliveMillis;
    bool reconnectEnabled;
    String reconnectRCId;
    long reconnectKeepAliveMillis;
#endif
};


//volatile bool _EnableDoubleClick = false;
volatile bool _Connected = false;
volatile int _ConnectVersion = 0;

#ifdef SUPPORT_IDLE_CALLBACK
volatile DDIdleCallback _IdleCallback = NULL; 
#endif

#ifdef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
volatile DDConnectVersionChangedCallback _ConnectVersionChangedCallback = NULL; 
#endif

bool IOProxy::available() {
  bool done = false;
  while (!done && pIO->available()) {
    char c =  pIO->read();
    if (c == '\n') {
      done = true;
    } else {
      data += c;
//         data = data + c;
    }
  }
  return done;
}
const String& IOProxy::get() {
  return data;
}
void IOProxy::clear() {
  //data.remove(0, data.length());
  data = "";
}
void IOProxy::print(const String &s) {
  pIO->print(s);
}
void IOProxy::print(const char *p) {
  pIO->print(p);
}
void IOProxy::write(uint8_t b) {
  pIO->write(b);
}
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
#if defined (SUPPORT_IDLE_CALLBACK) || defined (SUPPORT_RECONNECT)
  bool needReconnect = false;
  if (this->lastKeepAliveMillis > 0) {
    long now = millis();
    long notKeptAliveMillis = now - this->lastKeepAliveMillis; 
    if (notKeptAliveMillis > RECONNECT_NO_KEEP_ALIVE_MILLIS) {
      needReconnect = true;
#ifdef SUPPORT_IDLE_CALLBACK      
      if (_IdleCallback != NULL) {
        long idleForMillis = notKeptAliveMillis - RECONNECT_NO_KEEP_ALIVE_MILLIS;
        _IdleCallback(idleForMillis);
      }
#endif      
    }
#ifdef SUPPORT_RECONNECT
    if (this->reconnectEnabled && needReconnect) {
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
    }
#endif
  }
#endif  
}



//volatile bool _Preconneced = false;
//volatile bool _Connected = false;
volatile int _DDCompatibility = 0;
volatile int _NextLid = 0;
volatile int _NextImgId = 0;
volatile int _NextBytesId = 0;

#ifdef SUPPORT_TUNNEL
#define DD_LAYER_INC   2
DDObject** _DDLayerArray = NULL;
int _MaxDDLayerCount = 0;
#else
DDLayer** _DDLayerArray = NULL;
#endif

DDInputOutput* volatile _IO = NULL;

#ifdef SUPPORT_USE_WOIO
DDInputOutput* volatile _WOIO = NULL;
volatile uint16_t _SendBufferSize = 0;//DD_DEF_SEND_BUFFER_SIZE;
#else
#define _WOIO _IO
#endif


inline void _SetIO(DDInputOutput* io, uint16_t sendBufferSize) {
  _IO = io;
#ifdef SUPPORT_USE_WOIO  
  if (_WOIO != NULL) delete _WOIO;
  if (sendBufferSize > 0 && io->canUseBuffer()) {
    _SendBufferSize = sendBufferSize;
    _WOIO = new DDWriteOnyIO(io, sendBufferSize);
  } else {
    _SendBufferSize = 0;
    _WOIO = io;
  }
#endif
}



IOProxy* volatile _ConnectedIOProxy = NULL;
volatile bool _ConnectedFromSerial = false; 

#ifdef DEBUG_WITH_LED
volatile int _DebugLedPin = -1;
#endif
#ifdef DEBUG_ECHO_FEEDBACK 
volatile bool _DebugEnableEchoFeedback = false;
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



volatile bool _SendingCommand = false;
volatile bool _HandlingFeedback = false;


// void _Preconnect() {
//   if (_Preconneced) {
//     return;
//   }
// #ifdef DEBUG_WITH_LED
//   int debugLedPin = _DebugLedPin;  
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, HIGH);
//   }
// #endif
//   _IO->preConnect();
// #ifdef DEBUG_WITH_LED
//   if (debugLedPin != -1) {
//     digitalWrite(debugLedPin, LOW);
//   }
// #endif
// }
void _Connect() {
  if (_Connected)
    return;
// #ifdef SUPPORT_TUNNEL
//   _Preconnect(); 
// #else    
//   _IO->preConnect();
// #endif
#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;  
  bool debugLedOn;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
    debugLedOn = true;
  }
#endif
  {
    long startMillis = millis();
    long lastCallMillis = startMillis;
    bool firstCall = true;
    while (true) {
      YIELD();
      if (_IO->preConnect(firstCall)) {
        break;
      }
#ifdef SUPPORT_IDLE_CALLBACK
        if (_IdleCallback != NULL) {
          long now = millis();
          if ((now - lastCallMillis) >= HAND_SHAKE_GAP) {
            long idleForMillis = now - startMillis;
            _IdleCallback(idleForMillis);
            lastCallMillis = now;
          }
        }
#endif      
      firstCall = false;
    }
  }
  if (!_IO->isSerial()) {
    Serial.println("**********");
#ifdef SUPPORT_USE_WOIO
    Serial.print("* _SendBufferSize=");
    Serial.println(_SendBufferSize);
#endif
    //Serial.print("* _EnableDoubleClick=");
    //Serial.println(_EnableDoubleClick ? "yes" : "no");
    Serial.println("**********");
    Serial.flush();
  }
  {
    long nextTime = 0;
    IOProxy ioProxy(_IO);
    IOProxy* pSerialIOProxy = NULL;
    DDInputOutput *pSIO = NULL;
    if (_IO->isBackupBySerial()) {
      //pSIO = new DDInputOutput(_IO);
      pSIO = _IO->newForSerialConnection();
      pSerialIOProxy = new IOProxy(pSIO);
    }
    long startMillis = millis();
    while (true) {
      YIELD();
      long now = millis();
      if (now > nextTime) {
#ifdef DEBUG_WITH_LED
        if (debugLedPin != -1) {
          debugLedOn = !debugLedOn;
          digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
        }
#endif
        ioProxy.print("ddhello\n");
        if (pSerialIOProxy != NULL) 
          pSerialIOProxy->print("ddhello\n");
#ifdef DD_DEBUG_HS          
        Serial.println("handshake:ddhello");
#endif        
#ifdef SUPPORT_IDLE_CALLBACK
        if (_IdleCallback != NULL) {
          long idleForMillis = now - startMillis;
          _IdleCallback(idleForMillis);
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
            _SetIO(pSIO, DD_DEF_SEND_BUFFER_SIZE);
            //_IO = pSIO;
            pSIO = NULL;
          }
          _ConnectedIOProxy = new IOProxy(_IO);
//          _ConnectedFromSerial = fromSerial;
          _ConnectedFromSerial = _IO->isSerial();
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
    IOProxy ioProxy(_IO);
    while (true) {
      YIELD();
      long now = millis();
      if (now > nextTime) {
#ifdef DEBUG_WITH_LED
        if (debugLedPin != -1) {
          debugLedOn = !debugLedOn;
          digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
        }
#endif
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
//  _ConnectedIOProxy = new IOProxy(_IO);
  _DDCompatibility = compatibility;
  if (false) {
    // ignore any input in 1000ms window
    delay(1000);
    while (_IO->available()) {
      _IO->read();
    }
  }
  if (true) {       
    _IO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
    //_IO->flush();
    if (false) {
      // *** debug code
      for (int i = 0; i < 10; i++) {
        delay(500);
        _IO->print("// connected to DD c" + String(compatibility) + "\n"/*.c_str()*/);
      }
    }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** CONNECTED");
#endif        
  }
#ifdef DEBUG_WITH_LED
    if (debugLedPin != -1) {
      digitalWrite(debugLedPin, LOW);
    }
#endif
    // if (false) {
    //   // *** debug code
    //   _IO->print("// connection to DD made\n");
    //    _sendCommand0("", "// *** connection made ***");
    // }
#ifdef DD_DEBUG_HS          
    Serial.println("// *** DONE MAKE CONNECTION");
#endif        
}


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
#ifdef SUPPORT_TUNNEL
  if (DD_LAYER_INC > 0) {
    if (lid >= _MaxDDLayerCount) {
      if (true) {
        int oriLayerCount = _MaxDDLayerCount;
        _MaxDDLayerCount = lid + DD_LAYER_INC;
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
      } else {
        _MaxDDLayerCount = lid + DD_LAYER_INC;
        DDObject** oriLayerArray = _DDLayerArray;
        DDObject** layerArray = (DDObject**) malloc(_MaxDDLayerCount * sizeof(DDObject*));
        if (oriLayerArray != NULL) {
          memcpy(layerArray, oriLayerArray, (_MaxDDLayerCount - DD_LAYER_INC) * sizeof(DDObject*));
          free(oriLayerArray);
        }
        _DDLayerArray = layerArray;
      }
    }
  } else {
    DDObject** oriLayerArray = _DDLayerArray;
    DDObject** layerArray = (DDObject**) malloc((lid + 1) * sizeof(DDObject*));
    if (oriLayerArray != NULL) {
      memcpy(layerArray, oriLayerArray, lid * sizeof(DDObject*));
      free(oriLayerArray);
    }
    _DDLayerArray = layerArray;
  }
#else  
  DDLayer** oriLayerArray = _DDLayerArray;
  DDLayer** layerArray = (DDLayer**) malloc((lid + 1) * sizeof(DDLayer*));
  if (oriLayerArray != NULL) {
    memcpy(layerArray, oriLayerArray, lid * sizeof(DDLayer*));
    free(oriLayerArray);
  }
  _DDLayerArray = layerArray;
#endif  
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
int _AllocTid() {
  return _AllocLid();
}
void _PostCreateTunnel(DDTunnel* pTunnel) {
#ifdef STORE_LAYERS  
  int8_t lid = _LayerIdToLid(pTunnel->getTunnelId());
  _DDLayerArray[lid] = pTunnel;
#endif
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
String* _ReadFeedback(String& buffer) {
#ifdef VALIDATE_CONNECTION
    long now = millis();
    long diff = now - _LastValidateConnectionMillis;
    if (diff >= VALIDATE_GAP/*2000*//*5000*/) {
      _ConnectedIOProxy->validConnection();
      _LastValidateConnectionMillis = now;
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
    buffer = data;
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
  if (layerId != "") {
    _WOIO->print(layerId/*.c_str()*/);
    _WOIO->print(".");
  }
#ifdef SUPPORT_ENCODE_OPER
  if (_DDCompatibility >= 3 && layerId != "" && command[0] == '#') {
    char encoded[3];
    encoded[0] = 14 + ((command[1] > '9') ? ((command[1] - 'a') + 10) : (command[1] - '0'));
    encoded[1] = 14 + ((command[2] > '9') ? ((command[2] - 'a') + 10) : (command[2] - '0'));
    encoded[2] = 0;
    _WOIO->print(encoded);
  } else { 
    _WOIO->print(command);
    if (pParam1 != NULL) {
      _WOIO->print(":");
    }
  }   
#else
  _WOIO->print(command);
  #ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" ...");
  #endif        
  if (pParam1 != NULL) {
  #ifdef DD_DEBUG_SEND_COMMAND          
    Serial.print(" [1|");
    Serial.print(*pParam1);
    Serial.print(" |]");
  #endif        
    _WOIO->print(":");
  }
#endif
  if (pParam1 != NULL) {
    _WOIO->print(*pParam1/*pParam1->c_str()*/);
    if (pParam2 != NULL) {
      _WOIO->print(",");
      _WOIO->print(*pParam2/*pParam2->c_str()*/);
      if (pParam3 != NULL) {
        _WOIO->print(",");
        _WOIO->print(*pParam3/*pParam3->c_str()*/);
        if (pParam4 != NULL) {
          _WOIO->print(",");
          _WOIO->print(*pParam4/*pParam4->c_str()*/);
          if (pParam5 != NULL) {
            _WOIO->print(",");
            _WOIO->print(*pParam5/*pParam5->c_str()*/);
            if (pParam6 != NULL) {
              _WOIO->print(",");
              _WOIO->print(*pParam6/*pParam6->c_str()*/);
              if (pParam7 != NULL) {
                _WOIO->print(",");
                _WOIO->print(*pParam7/*pParam7->c_str()*/);
                if (pParam8 != NULL) {
                  _WOIO->print(",");
                  _WOIO->print(*pParam8/*pParam8->c_str()*/);
                  if (pParam9 != NULL) {
                    _WOIO->print(",");
                    _WOIO->print(*pParam9/*pParam9->c_str()*/);
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
  _WOIO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WOIO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
#ifdef DEBUG_ECHO_COMMAND
  _IO->print("// ");
  _IO->print(command);
  _IO->print("\n");
  _IO->flush();
#endif  
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.println(command);
#endif        
}  
void _HandleFeedback();
void _SendCommand(const String& layerId, const char* command, const String* pParam1 = NULL, const String* pParam2 = NULL, const String* pParam3 = NULL, const String* pParam4 = NULL, const String* pParam5 = NULL, const String* pParam6 = NULL, const String* pParam7 = NULL, const String* pParam8 = NULL, const String* pParam9 = NULL) {
  bool alreadySendingCommand = _SendingCommand;  // not very accurate
  _SendingCommand = true;

#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
  }
#endif   

  if (command != NULL) {
    __SendCommand(layerId, command, pParam1, pParam2, pParam3, pParam4, pParam5, pParam6, pParam7, pParam8, pParam9);
  }

#ifdef ENABLE_FEEDBACK
  if (!alreadySendingCommand) {
    _HandleFeedback();
  }
#endif

#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, LOW);
  }  
#endif

  _SendingCommand = false;
}
void __SendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
//Serial.println("//&&" + specialData);
  _WOIO->print("%%>");
  _WOIO->print(specialType);
  _WOIO->print(".");
  if (specialId != "") {
    _WOIO->print(specialId);
    if (specialCommand != NULL) {
      _WOIO->print(":");
      _WOIO->print(specialCommand);
    }
    _WOIO->print(">");
  }
    if (specialData != "") {
      _WOIO->print(specialData);
    }
  _WOIO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _WOIO->flush();
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
            _IO->write(0);
            _IO->write(zeroCount);
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
            _IO->write(b);
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
  _IO->print("|bytes|>");
  if (_DDCompatibility >= 5 && bytesNature != NULL) {
//Serial.print("*** BYTES NATURE: ");
//Serial.println(bytesNature);    
    _IO->print(bytesNature);
    _IO->print("#");
  }
  _IO->print(String(byteCount));
  if (compressedByteCount != -1) {
    _IO->print("@0>");
    _IO->print(String(compressedByteCount));
  }
  _IO->print(":");
  if (true) {
    if (compressedByteCount != -1) {
      //__CountZeroCompressedBytes(bytes, byteCount, true);
      if (readFromProgramSpace) {
        uint8_t dummy;
        __FillZeroCompressedBytes(bytes, byteCount, &dummy, true);
      } else {
        uint8_t *compressedBytes = new uint8_t[compressedByteCount];
        __FillZeroCompressedBytes(bytes, byteCount, compressedBytes, false);
        _IO->write(compressedBytes, compressedByteCount);
        delete compressedBytes;
      }
    } else {
      if (readFromProgramSpace) {
        for (int i = 0; i < byteCount; i++) {
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
          _IO->write(pgm_read_byte(bytes[i]));
#else          
          _IO->write(bytes[i]);
#endif          
        }
      } else {
        _IO->write(bytes, byteCount);
      }
    }
  } else {
    for (int i = 0; i < byteCount; i++) {
      uint8_t b = bytes[i];
      _IO->write(b);
    }
  }
  if (FLUSH_AFTER_SENT_COMMAND) {
    _IO->flush();
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
#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
  }
  __SendSpecialCommand(specialType, specialId, specialCommand, specialData);
#endif   
  if (!alreadySendingCommand) {
    _HandleFeedback();
  }
#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, LOW);
  }  
#endif
  _SendingCommand = false;
}


bool _CanLogToSerial() {
  if (!_ConnectedFromSerial || !_Connected) {
    return true;
  } else {
    return false;
  }
}
// inline void _LogToSerial(const String& logLine) {
//   if (!_ConnectedFromSerial || !_Connected) {
//     Serial.println(logLine);  // in case not connected ... hmm ... assume ... Serial.begin() called
//   } else {
//     _SendCommand("", ("// " + logLine).c_str());
//   }
// }

#ifdef READ_BUFFER_USE_BUFFER
String _ReadFeedbackBuffer;
#endif
void _HandleFeedback() {
  if (!_HandlingFeedback) {
    _HandlingFeedback = true;
#ifdef READ_BUFFER_USE_BUFFER
    String* pFeedback = _ReadFeedback(_ReadFeedbackBuffer);
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
          // keep alive wheneven received someting
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
//Serial.println("LT-[" + *pFeedback + "]");
          if (pFeedback->startsWith("<lt.")) {
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
//Serial.println(String("// ") + (final ? "F" : "."));
Serial.println("LT++++" + data + " - final:" + String(final));
#endif
                pTunnel->handleInput(data, final);
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
      DDFeedbackType type = CLICK;
      int16_t x = -1;
      int16_t y = -1;
      char* pText = NULL;      
      char* token = strtok(buf, ".");
      if (token != NULL) {
        lid = _LayerIdToLid(token);
        token = strtok(NULL, ":");
      }
      if (token != NULL) {
        if (strcmp(token, "longpress") == 0) {
          type = LONGPRESS;
        } else if (strcmp(token, "doubleclick") == 0) {
          type = DOUBLECLICK;
        } else if (strcmp(token, "move") == 0) {
          type = MOVE;
        }
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        x = atoi(token);
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        y = atoi(token);
        ok = true;
        token = strtok(NULL, "");  // want the rest
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
#ifdef SUPPORT_TUNNEL
inline void _sendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
  _SendSpecialCommand(specialType, specialId, specialCommand, specialData);
}
void _setLTBufferCommand(const String& data) {
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
}
#endif
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


DDFeedbackManager::DDFeedbackManager(int8_t bufferSize) {
  this->nextArrayIdx = 0;
  this->validArrayIdx = 0;
}
const DDFeedback* DDFeedbackManager::getFeedback() {
  if (nextArrayIdx == validArrayIdx) return NULL;
  const DDFeedback* pFeedback = &feedbackArray[validArrayIdx];
  validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
  return pFeedback;
}
void DDFeedbackManager::pushFeedback(DDFeedbackType type, int16_t x, int16_t y, const char* pText) {
  feedbackArray[nextArrayIdx].type = type;
  feedbackArray[nextArrayIdx].x = x;
  feedbackArray[nextArrayIdx].y = y;
  feedbackArray[nextArrayIdx].text = pText != NULL ? pText : "";
  nextArrayIdx = (nextArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
  if (nextArrayIdx == validArrayIdx)
    validArrayIdx = (validArrayIdx + 1) % DD_FEEDBACK_BUFFER_SIZE/*arraySize*/;
}


DDLayer::DDLayer(int8_t layerId)/*: DDObject(DD_OBJECT_TYPE_LAYER)*/ {
  this->objectType = DD_OBJECT_TYPE_LAYER;
  this->layerId = String(layerId);
  this->pFeedbackManager = NULL;
  this->feedbackHandler = NULL;
}
DDLayer::~DDLayer() {
  _PreDeleteLayer(this);
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
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
void DDLayer::backgroundColor(const String& color) {
  _sendCommand1(layerId, C_bgcolor, color);
}
void DDLayer::noBackgroundColor() {
  _sendCommand0(layerId, C_nobgcolor);
}
// void DDLayer::reorder(bool bringUp) {
//   _sendCommand1(layerId, C_reorder);
// }
void DDLayer::flash() {
  _sendCommand0(layerId, C_flash);
}
void DDLayer::flashArea(int x, int y) {
  _sendCommand2(layerId, C_flasharea, String(x), String(y));
}
// void DDLayer::writeComment(const String& comment) {
//   _sendCommand0("", ("// " + layerId + ": " + comment).c_str());
// }
void DDLayer::_enableFeedback() {
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
  pFeedbackManager = new DDFeedbackManager(FEEDBACK_BUFFER_SIZE + 1);  // need 1 more slot
}
void DDLayer::enableFeedback(const String& autoFeedbackMethod) {
  _sendCommand2(layerId, C_feedback, TO_BOOL(true), autoFeedbackMethod);
  feedbackHandler = NULL;
  _enableFeedback();
  // if (pFeedbackManager != NULL)
  //   delete pFeedbackManager;
  // pFeedbackManager = new DDFeedbackManager(FEEDBACK_BUFFER_SIZE + 1);  // need 1 more slot
}
void DDLayer::disableFeedback() {
  _sendCommand1(layerId, C_feedback, TO_BOOL(false));
  feedbackHandler = NULL;
  if (pFeedbackManager != NULL) {
    delete pFeedbackManager;
    pFeedbackManager = NULL;
  }
}
const DDFeedback* DDLayer::getFeedback() {
  //yield();
  _HandleFeedback();
  return pFeedbackManager != NULL ? pFeedbackManager->getFeedback() : NULL;
}
void DDLayer::setFeedbackHandler(DDFeedbackHandler handler, const String& autoFeedbackMethod) {
  bool enable = handler != NULL;
  _sendCommand2(layerId, C_feedback, TO_BOOL(enable), autoFeedbackMethod);
  feedbackHandler = handler;
  if (pFeedbackManager != NULL) {
    delete pFeedbackManager;
    pFeedbackManager = NULL;
  }
}
void DDLayer::debugOnly(int i) {
  _sendCommand2(layerId, "debugonly", String(i), TO_C_INT(i));
  // byte bytes[i];
  // for (int j = 0; j < i; j++) {
  //   bytes[j] = j;
  // }
  // _sendByteArrayAfterCommand(bytes, i);
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
void LedGridDDLayer::bitwise(unsigned long bits, int y) {
  _sendCommand2(layerId, C_bitwise, String(y), String(bits));
}
void LedGridDDLayer::bitwise2(unsigned long bits_0, unsigned long bits_1, int y) {
  _sendCommand3(layerId, C_bitwise, String(y), String(bits_0), String(bits_1));
}
void LedGridDDLayer::bitwise3(unsigned long bits_0, unsigned long bits_1, unsigned long bits_2, int y) {
  _sendCommand4(layerId, C_bitwise, String(y), String(bits_0), String(bits_1), String(bits_2));
}
void LedGridDDLayer::bitwise4(unsigned long bits_0, unsigned long bits_1, unsigned long bits_2, unsigned long bits_3, int y) {
  _sendCommand5(layerId, C_bitwise, String(y), String(bits_0), String(bits_1), String(bits_2), String(bits_3));
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
void LcdDDLayer::writeCenteredLine(const String& text, int y) {
  _sendCommand3(layerId, C_writeline, String(y), "C", text);
} 
void LcdDDLayer::pixelColor(const String &color) {
  _sendCommand1(layerId, C_pixelcolor, color);
}
void LcdDDLayer::bgPixelColor(const String &color) {
  _sendCommand1(layerId, C_bgpixelcolor, color);
}
void LcdDDLayer::noBgPixelColor() {
  _sendCommand0(layerId, C_bgpixelcolor);
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
void GraphicalDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, C_textfont, fontName, String(size));
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
void GraphicalDDLayer::drawPixel(int x, int y, const String& color) {
  _sendCommand3(layerId, C_drawpixel, TO_C_INT(x), TO_C_INT(y), color);
}
void GraphicalDDLayer::drawLine(int x1, int y1, int x2, int y2, const String& color) {
  _sendCommand5(layerId, C_drawline, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), color);
}
void GraphicalDDLayer::drawRect(int x, int y, int w, int h, const String& color, bool filled) {
  _sendCommand6(layerId, c_drawrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
}
void GraphicalDDLayer::drawOval(int x, int y, int w, int h, const String& color, bool filled) {
  _sendCommand6(layerId, c_drawoval, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), color, TO_BOOL(filled));
}
void GraphicalDDLayer::drawArc(int x, int y, int w, int h, int startAngle, int sweepAngle, bool useCenter, const String& color, bool filled) {
  _sendCommand9(layerId, c_drawarc, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(startAngle), TO_C_INT(sweepAngle), TO_BOOL(useCenter), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillRect(int x, int y, int w, int h, const String& color) {
//   _sendCommand6(layerId, "drawrect", String(x), String(y), String(w), String(h), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawCircle(int x, int y, int r, const String& color, bool filled) {
  _sendCommand5(layerId, C_drawcircle, TO_C_INT(x), TO_C_INT(y), TO_C_INT(r), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillCircle(int x, int y, int r, const String& color) {
//   _sendCommand5(layerId, "drawcircle", String(x), String(y), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color, bool filled) {
  //_sendCommand8(layerId, C_drawtriangle, String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(filled));
  _sendCommand8(layerId, C_drawtriangle, TO_C_INT(x1), TO_C_INT(y1), TO_C_INT(x2), TO_C_INT(y2), TO_C_INT(x3), TO_C_INT(y3), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color) {
//   _sendCommand8(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawRoundRect(int x, int y, int w, int h, int r, const String& color, bool filled) {
  _sendCommand7(layerId, C_drawroundrect, TO_C_INT(x), TO_C_INT(y), TO_C_INT(w), TO_C_INT(h), TO_C_INT(r), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillRoundRect(int x, int y, int w, int h, int r, const String& color) {
//   _sendCommand7(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::forward(int distance) {
  _sendCommand1(layerId, C_fd, String(distance));
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
void GraphicalDDLayer::loadImageFileCropped(const String& imageFileName, int x, int y, int w, int h, const String& asImageFileName) {
  _sendCommand6(layerId, C_loadimagefilecropped, imageFileName, String(x), String(y), String(w), String(h), asImageFileName);
}
void GraphicalDDLayer::cacheImage(const String& imageName, const uint8_t *bytes, int byteCount, char compressMethod) {
  _sendCommand2("", C_CACHEIMG, layerId, imageName);
  _sendByteArrayAfterCommand(bytes, byteCount, compressMethod);
}
void GraphicalDDLayer::cachePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color, char compressMethod) {
  int byteCount = width * height / 8; 
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
void GraphicalDDLayer::saveCachedImageFile(const String& imageName) {
  _sendCommand2("", C_SAVECACHEDIMG, layerId, imageName);
}
void GraphicalDDLayer::saveCachedImageFiles(const String& stitchAsImageName) {
  _sendCommand2("", C_SAVECACHEDIMGS, layerId, stitchAsImageName);
}
void GraphicalDDLayer::unloadImageFile(const String& imageFileName) {
  _sendCommand1(layerId, C_unloadimagefile, imageFileName);
}
void GraphicalDDLayer::drawImageFile(const String& imageFileName, int x, int y, int w, int h) {
  if (w == 0 && h == 0) {
    _sendCommand3(layerId, C_drawimagefile, imageFileName, String(x), String(y));
  } else {
    _sendCommand5(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h));
  }
}
void GraphicalDDLayer::drawImageFileFit(const String& imageFileName, int x, int y, int w, int h, const String& align) {
  _sendCommand6(layerId, C_drawimagefilefit, imageFileName, String(x), String(y), String(w), String(h), align);
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
void SevenSegmentRowDDLayer::showHexNumber(int number) {
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
void JoystickDDLayer::moveToPos(int x, int y, bool sendFeedback) {
  _sendCommand3(layerId, C_movetopos, TO_C_INT(x), TO_C_INT(y), TO_BOOL(sendFeedback));
}
void JoystickDDLayer::moveToCenter(bool sendFeedback) {
  _sendCommand1(layerId, C_movetocenter, TO_BOOL(sendFeedback));
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

#ifdef SUPPORT_TUNNEL
DDTunnel::DDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint, bool connectNow/*, int8_t bufferSize*/):
  /*DDObject(DD_OBJECT_TYPE_TUNNEL), */type(type), tunnelId(String(tunnelId)), params(params), endPoint(endPoint) {
    this->objectType = DD_OBJECT_TYPE_TUNNEL;
  // this->arraySize = bufferSize;
  // this->dataArray = new String[bufferSize];
  // this->nextArrayIdx = 0;
  // this->validArrayIdx = 0;
//  this->done = false;
  this->done = true;
  if (connectNow) {
    reconnect();
  }
}
DDTunnel::~DDTunnel() {
  _PreDeleteTunnel(this);
  //delete this->dataArray;
} 
void DDTunnel::reconnect() {
  if (endPoint != "") {
    //nextArrayIdx = 0;
    //validArrayIdx = 0;
    done = false;
    //for (int i = 0; i < arraySize; i++) {
      //dataArray[i] = "";
    //}
    if (_DDCompatibility >= 4) {
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
  }
}
void DDTunnel::release() {
  if (!done) {
    _sendSpecialCommand("lt", this->tunnelId, "disconnect", "");
  }
  done = true;
}
// int DDTunnel::_count() {
//   return (arraySize + validArrayIdx - nextArrayIdx) % arraySize;
// }
bool DDTunnel::_eof() {
  //yield();
  _HandleFeedback();
#ifdef TUNNEL_TIMEOUT_MILLIS
    if (done) {
#ifdef DEBUG_TUNNEL_RESPONSE
Serial.println("_EOF: DONE");
#endif                
      return true;
    }
    long diff = millis() - connectMillis;
    if (diff > TUNNEL_TIMEOUT_MILLIS) {
      return true;
    }
    return false;
#else
    return /*nextArrayIdx == validArrayIdx && */done;
#endif    
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
void DDTunnel::handleInput(const String& data, bool final) {
//if (final) Serial.println("//final:" + data);
  // if (!final || data != "") {
  //   dataArray[nextArrayIdx] = data;
  //   nextArrayIdx  = (nextArrayIdx + 1) % arraySize;
  //   if (nextArrayIdx == validArrayIdx)
  //     validArrayIdx = (validArrayIdx + 1) % arraySize;
  // }
  if (final)
    this->done = true;
//Serial.println(String("// ") + (final ? "f" : "."));
}
DDBufferedTunnel::DDBufferedTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint, bool connectNow, int8_t bufferSize):
  DDTunnel(type, tunnelId, params, endPoint, connectNow/*, bufferSize*/) {
  bufferSize = bufferSize + 1;  // need one more
  this->arraySize = bufferSize;
  this->dataArray = new String[bufferSize];
  this->nextArrayIdx = 0;
  this->validArrayIdx = 0;
  //this->done = false;
}
DDBufferedTunnel::~DDBufferedTunnel() {
  delete this->dataArray;
} 
void DDBufferedTunnel::reconnect() {
  nextArrayIdx = 0;
  validArrayIdx = 0;
  //done = false;
  for (int i = 0; i < arraySize; i++) {
    dataArray[i] = "";
  }
  //_sendSpecialCommand("lt", tunnelId, "reconnect", endPoint);
  this->DDTunnel::reconnect();
}
void DDBufferedTunnel::release() {
  // if (!done) {
  //   _sendSpecialCommand("lt", this->tunnelId, "disconnect", "");
  // }
  // done = true;
  this->DDTunnel::release();
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
bool DDBufferedTunnel::_eof() {
  if (true) {
      _HandleFeedback();
  }
  if (!this->DDTunnel::_eof()) {
    return false;
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
bool DDBufferedTunnel::_readLine(String &buffer) {
  if (nextArrayIdx == validArrayIdx) {
    buffer = "";
    return false;
  } else {
    buffer = dataArray[validArrayIdx];
    dataArray[validArrayIdx] = "";
    validArrayIdx = (validArrayIdx + 1) % arraySize;
    return true;
  }
}

// void DDTunnel::_writeLine(const String& data) {
// //Serial.println("//--");
//   _sendSpecialCommand("lt", tunnelId, NULL, data);
// }
void DDBufferedTunnel::handleInput(const String& data, bool final) {
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
    nextArrayIdx  = (nextArrayIdx + 1) % arraySize;
    if (nextArrayIdx == validArrayIdx)
      validArrayIdx = (validArrayIdx + 1) % arraySize;
  }
  this->DDTunnel::handleInput(data, final);
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
String BasicDDTunnel::readLine() {
  String buffer;
  _readLine(buffer);
  return buffer;
}
bool BasicDDTunnel::read(String& fieldId, String& fieldValue) {
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
void SimpleToolDDTunnel::reconnect() {
  this->result = 0;
  this->DDBufferedTunnel::reconnect();
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
    } else if (eof()) {
      // not quite expected
#ifdef DEBUG_TUNNEL_RESPONSE      
Serial.println("XXX EOF???");
#endif
      this->result = -1;
    }
  }
  return this->result;
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

void ObjectDetetDemoServiceDDTunnel::reconnectForObjectDetect(const String& imageName) {
  reconnectTo("detect?imageName=" + imageName);
}
void ObjectDetetDemoServiceDDTunnel::reconnectForObjectDetectFrom(GraphicalDDLayer* pGraphicalLayer, const String& imageName) {
  reconnectTo("detectfrom?layerId=" + pGraphicalLayer->getLayerId() + "?imageName=" + imageName);
}


bool ObjectDetetDemoServiceDDTunnel::readObjectDetectResult(DDObjectDetectDemoResult& objectDetectResult) {
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
#endif


void DumbDisplay::initialize(DDInputOutput* pIO, uint16_t sendBufferSize/*, boolean enableDoubleClick*/) {
  _SetIO(pIO, sendBufferSize);
  //_IO = pIO;
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
void DumbDisplay::configPinFrame(int xUnitCount, int yUnitCount) {
  _Connect();
  if (xUnitCount != 100 || yUnitCount != 100) {
    _sendCommand2("", "CFGPF", String(xUnitCount), String(yUnitCount));
  }
}
void DumbDisplay::configAutoPin(const String& layoutSpec) {
  _Connect();
  _sendCommand1("", "CFGAP", layoutSpec);
}
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
LcdDDLayer* DumbDisplay::createLcdLayer(int colCount, int rowCount, int charHeight, const String& fontName) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand5(layerId, "SU", String("lcd"), String(colCount), String(rowCount), String(charHeight), fontName);
  LcdDDLayer* pLayer = new LcdDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
GraphicalDDLayer* DumbDisplay::createGraphicalLayer(int width, int height) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand3(layerId, "SU", String("graphical"), String(width), String(height));
  GraphicalDDLayer* pLayer = new GraphicalDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
SevenSegmentRowDDLayer* DumbDisplay::create7SegmentRowLayer(int digitCount) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand2(layerId, "SU", String("7segrow"), String(digitCount));
  SevenSegmentRowDDLayer* pLayer = new SevenSegmentRowDDLayer(lid);
  _PostCreateLayer(pLayer);
  return pLayer;
}
JoystickDDLayer* DumbDisplay::createJoystickLayer(const String& directions, int maxStickValue, float stickLookScaleFactor) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("joystick"), directions, String(maxStickValue),  TO_NUM(stickLookScaleFactor));
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

void DumbDisplay::pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand5(pLayer->getLayerId(), "PIN", String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand6("", "PINAP", layoutSpec, String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::deleteLayer(DDLayer *pLayer) {
  _sendCommand0(pLayer->getLayerId(), "DEL");
  delete pLayer;  // will call _PreDeleteLayer(pLayer)
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
    ((DDWriteOnyIO*) _WOIO)->setKeepBuffering(true);
  }
#endif
}
void DumbDisplay::playbackLayerSetupCommands(const String& layerSetupPersistId) {
  _sendCommand2("", C_SAVEC, layerSetupPersistId, TO_BOOL(true));
  _sendCommand0("", C_PLAYC);
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WOIO)->setKeepBuffering(false);
    ((DDWriteOnyIO*) _WOIO)->flush();
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
    ((DDWriteOnyIO*) _WOIO)->setKeepBuffering(true);
  }
#endif
  _sendCommand0("", C_RECC);
}
void DumbDisplay::playbackLayerCommands() {
#ifdef SUPPORT_USE_WOIO
  if (_SendBufferSize > 0) {
    ((DDWriteOnyIO*) _WOIO)->setKeepBuffering(false);
  }
#endif
  _sendCommand0("", C_PLAYC);
}
void DumbDisplay::stopRecordLayerCommands() {
  _sendCommand0("", "STOPC");
}
void DumbDisplay::saveLayerCommands(const String& id, bool persist) {
  _sendCommand2("", C_SAVEC, id, TO_BOOL(persist));
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
  _Connect();
  _sendCommand2("", C_TONE, TO_C_INT(freq), TO_C_INT(duration));
}
void DumbDisplay::notone() {
  _Connect();
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
  int byteCount = width * height / 8; 
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

#ifdef SUPPORT_TUNNEL
BasicDDTunnel* DumbDisplay::createBasicTunnel(const String& endPoint, bool connectNow, int8_t bufferSize) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  // if (connectNow) {
  //   _sendSpecialCommand("lt", tunnelId, "connect", "ddbasic@" + endPoint);
  // }
  BasicDDTunnel* pTunnel = new BasicDDTunnel("ddbasic", tid, "", endPoint, connectNow, bufferSize);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
JsonDDTunnel* DumbDisplay::createJsonTunnel(const String& endPoint, bool connectNow, int8_t bufferSize) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  // if (connectNow) {
  //   _sendSpecialCommand("lt", tunnelId, "connect", "ddsimplejson@" + endPoint);
  // }
  JsonDDTunnel* pTunnel = new JsonDDTunnel("ddsimplejson", tid, "", endPoint, connectNow, bufferSize);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
JsonDDTunnel* DumbDisplay::createFilteredJsonTunnel(const String& endPoint, const String& fieldNames, bool connectNow, int8_t bufferSize) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  JsonDDTunnel* pTunnel = new JsonDDTunnel("ddsimplejson", tid, fieldNames, endPoint, connectNow, bufferSize);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
SimpleToolDDTunnel* DumbDisplay::createImageDownloadTunnel(const String& endPoint, const String& imageName, boolean redownload) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  String params = imageName;
  if (!redownload) {
    params = params + ",NRDL";
  }
  SimpleToolDDTunnel* pTunnel = new SimpleToolDDTunnel("dddownloadimage", tid, params, endPoint, true, 1);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
BasicDDTunnel* DumbDisplay::createDateTimeServiceTunnel() {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  BasicDDTunnel* pTunnel = new BasicDDTunnel("datetimeservice", tid, "", "", false, 1);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}

GpsServiceDDTunnel* DumbDisplay::createGpsServiceTunnel() {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  GpsServiceDDTunnel* pTunnel = new GpsServiceDDTunnel("gpsservice", tid, "", "", false, 1);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}

ObjectDetetDemoServiceDDTunnel* DumbDisplay::createObjectDetectDemoServiceTunnel(int scaleToWidth, int scaleToHeight) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  String params;
  if (scaleToWidth > 0 && scaleToHeight > 0) {
    params = String(scaleToWidth) + "," + String(scaleToHeight);
  }
  ObjectDetetDemoServiceDDTunnel* pTunnel = new ObjectDetetDemoServiceDDTunnel("objectdetectdemo", tid, params, "", false, DD_TUNNEL_DEF_BUFFER_SIZE);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
void DumbDisplay::deleteTunnel(DDTunnel *pTunnel) {
  pTunnel->release();
#ifndef ESP32  
  delete pTunnel;  // problem with ESP32 ... for now, just don't delete
#endif  
}
#endif


bool DumbDisplay::canLogToSerial() {
  return _CanLogToSerial();
}

void DumbDisplay::debugSetup(int debugLedPin/*, bool enableEchoFeedback*/) {
#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
     pinMode(debugLedPin, OUTPUT);
   }
  _DebugLedPin = debugLedPin;
#endif  
#ifdef DEBUG_ECHO_FEEDBACK
  _DebugEnableEchoFeedback = true;//enableEchoFeedback;
#endif
}
// #ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
// void DumbDisplay::optionNoCompression(bool noCompression) {
//   _NoEncodeInt = noCompression;
// }
//#endif
void DumbDisplay::setIdleCallback(DDIdleCallback idleCallback) {
#ifdef SUPPORT_IDLE_CALLBACK
  _IdleCallback = idleCallback;
#endif
}
void DumbDisplay::setConnectVersionChangedCalback(DDConnectVersionChangedCallback connectVersionChangedCallback) {
#ifdef SUPPORT_CONNECT_VERSION_CHANGED_CALLBACK
  _ConnectVersionChangedCallback = connectVersionChangedCallback;
#endif
}


// void DumbDisplay::delay(unsigned long ms) {
//   _Delay(ms);
// }
void DumbDisplay::logToSerial(const String& logLine) {
  if (canLogToSerial()) {
    if (_The_DD_Serial != NULL) {
      _The_DD_Serial->print(logLine);
      _The_DD_Serial->print("\n");
    }
  } else {
    writeComment(logLine);
  }
}



// void DDLogToSerial(const String& logLine) {
//    _LogToSerial(logLine);
// }
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



