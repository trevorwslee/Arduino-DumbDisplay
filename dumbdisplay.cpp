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

#define TO_BOOL(val) (val ? "1" : "0")


//#define DD_DEBUG_HS
//#define DD_DEBUG_SEND_COMMAND
//#define DEBUG_ECHO_COMMAND
//#define DEBUG_RECEIVE_FEEDBACK
//#define DEBUG_ECHO_FEEDBACK
//#define DEBUG_VALIDATE_CONNECTION
//#define DEBUG_TUNNEL_RESPONSE


#define SUPPORT_LONG_PRESS_FEEDBACK

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

//#define DD_SID "Arduino-c1"
#define DD_SID "Arduino-c2"


#include "_dd_commands.h"


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

#ifdef SUPPORT_TUNNEL
DDObject** _DDLayerArray = NULL;
#else
DDLayer** _DDLayerArray = NULL;
#endif

DDInputOutput* volatile _IO = NULL;
IOProxy* volatile _ConnectedIOProxy = NULL;
volatile bool _ConnectedFromSerial = false; 

#ifdef DEBUG_WITH_LED
volatile int _DebugLedPin = -1;
#endif
#ifdef DEBUG_ECHO_FEEDBACK 
volatile bool _DebugEnableEchoFeedback = false;
#endif
#ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
volatile bool _NoEncodeInt = false;
#endif


#define IS_FLOAT_ZERO(f) ((((f)<0?-(f):(f)) - 0.0) < 0.001)
#define IS_FLOAT_WHOLE(f) IS_FLOAT_ZERO((f) - (int) (f))


#ifdef DD_CONDENSE_COMMAND
#ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
#define TO_C_INT(i) (_NoEncodeInt ? String(i) : DDIntEncoder(i).encoded())
#else
#define TO_C_INT(i) (DDIntEncoder(i).encoded())
#endif
#define TO_NUM(num) IS_FLOAT_WHOLE(num) ? String((int) num) : String(num) 
#else
#define TO_C_INT(i) String(i)
#define TO_NUM(num) String(num) 
#endif
#define TO_C_NUM(num) IS_FLOAT_WHOLE(num) ? TO_C_INT((int) num) : String(num) 



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
            _IO = pSIO;
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


int _AllocImgId() {
  int imgId = _NextImgId++;
  return imgId;
}
int _AllocLid() {
  _Connect();
  int lid = _NextLid++;
#ifdef STORE_LAYERS  
#ifdef SUPPORT_TUNNEL
  DDObject** oriLayerArray = _DDLayerArray;
  DDObject** layerArray = (DDObject**) malloc((lid + 1) * sizeof(DDObject*));
  if (oriLayerArray != NULL) {
    memcpy(layerArray, oriLayerArray, lid * sizeof(DDObject*));
    free(oriLayerArray);
  }
  _DDLayerArray = layerArray;
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



void __SendCommand(const String& layerId, const char* command, const String* pParam1, const String* pParam2, const String* pParam3, const String* pParam4, const String* pParam5, const String* pParam6, const String* pParam7, const String* pParam8) {
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print("// *** sent");
#endif        
  if (layerId != "") {
    _IO->print(layerId/*.c_str()*/);
    _IO->print(".");
  }
  _IO->print(command);
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" ...");
#endif        
  if (pParam1 != NULL) {
#ifdef DD_DEBUG_SEND_COMMAND          
  Serial.print(" [1|");
  Serial.print(*pParam1);
  Serial.print(" |]");
#endif        
    _IO->print(":");
    _IO->print(*pParam1/*pParam1->c_str()*/);
    if (pParam2 != NULL) {
      _IO->print(",");
      _IO->print(*pParam2/*pParam2->c_str()*/);
      if (pParam3 != NULL) {
        _IO->print(",");
        _IO->print(*pParam3/*pParam3->c_str()*/);
        if (pParam4 != NULL) {
          _IO->print(",");
          _IO->print(*pParam4/*pParam4->c_str()*/);
          if (pParam5 != NULL) {
            _IO->print(",");
            _IO->print(*pParam5/*pParam5->c_str()*/);
            if (pParam6 != NULL) {
              _IO->print(",");
              _IO->print(*pParam6/*pParam6->c_str()*/);
              if (pParam7 != NULL) {
                _IO->print(",");
                _IO->print(*pParam7/*pParam7->c_str()*/);
                if (pParam8 != NULL) {
                  _IO->print(",");
                  _IO->print(*pParam8/*pParam8->c_str()*/);
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
  _IO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _IO->flush();
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
void _SendCommand(const String& layerId, const char* command, const String* pParam1 = NULL, const String* pParam2 = NULL, const String* pParam3 = NULL, const String* pParam4 = NULL, const String* pParam5 = NULL, const String* pParam6 = NULL, const String* pParam7 = NULL, const String* pParam8 = NULL) {
  bool alreadySendingCommand = _SendingCommand;  // not very accurate
  _SendingCommand = true;

#ifdef DEBUG_WITH_LED
  int debugLedPin = _DebugLedPin;
  if (debugLedPin != -1) {
    digitalWrite(debugLedPin, HIGH);
  }
#endif   

  if (command != NULL) {
    __SendCommand(layerId, command, pParam1, pParam2, pParam3, pParam4, pParam5, pParam6, pParam7, pParam8);
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
  _IO->print("%%>");
  _IO->print(specialType);
  _IO->print(".");
  _IO->print(specialId);
  if (specialCommand != NULL) {
    _IO->print(":");
    _IO->print(specialCommand);
  }
  _IO->print(">");
  if (specialData != "") {
    _IO->print(specialData);
  }
  _IO->print("\n");
  if (FLUSH_AFTER_SENT_COMMAND) {
    _IO->flush();
  }
  if (YIELD_AFTER_SEND_COMMAND) {
    yield();
  }
}
void __SendByteArrayPortion(const uint8_t *bytes, int byteCount) {
  _IO->print("|bytes|>");
  _IO->print(String(byteCount));
  _IO->print(":");
  if (true) {
    _IO->write(bytes, byteCount);
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
//Serial.println("LT-command" + data);
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
#ifdef SUPPORT_LONG_PRESS_FEEDBACK        
        if (strcmp(token, "longpress") == 0) {
          type = LONGPRESS;
        } else if (strcmp(token, "doubleclick") == 0) {
          type = DOUBLECLICK;
        }
#endif       
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        x = atoi(token);
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        y = atoi(token);
        ok = true;
        token = strtok(NULL, ",");
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
#ifdef SUPPORT_TUNNEL
inline void _sendSpecialCommand(const char* specialType, const String& specialId, const char* specialCommand, const String& specialData) {
  _SendSpecialCommand(specialType, specialId, specialCommand, specialData);
}
#endif
void _sendByteArrayAfterCommand(const uint8_t *bytes, int byteCount) {
  __SendByteArrayPortion(bytes, byteCount);
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


DDLayer::DDLayer(int8_t layerId): DDObject(DD_OBJECT_TYPE_LAYER) {
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
void DDLayer::enableFeedback(const String& autoFeedbackMethod) {
  _sendCommand2(layerId, C_feedback, TO_BOOL(true), autoFeedbackMethod);
  feedbackHandler = NULL;
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
  pFeedbackManager = new DDFeedbackManager(FEEDBACK_BUFFER_SIZE + 1);  // need 1 more slot
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
  _sendCommand1(layerId, "fillcolor", color);
}
void TurtleDDLayer::noFillColor() {
  _sendCommand0(layerId, "nofillcolor");
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
void GraphicalDDLayer::loadImageFile(const String& imageFileName, int w, int h) {
  _sendCommand3(layerId, C_loadimagefile, imageFileName, String(w), String(h));
}
void GraphicalDDLayer::cacheImage(const String& imageName, const uint8_t *bytes, int byteCount) {
  _sendCommand2("", C_CACHEIMG, layerId, imageName);
  _sendByteArrayAfterCommand(bytes, byteCount);
}
void GraphicalDDLayer::unloadImageFile(const String& imageFileName) {
  _sendCommand1(layerId, C_unloadimagefile, imageFileName);
}
void GraphicalDDLayer::drawImageFile(const String& imageFileName, int x, int y, int w, int h) {
  _sendCommand5(layerId, C_drawimagefile, imageFileName, String(x), String(y), String(w), String(h));
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
void SevenSegmentRowDDLayer::showFormatted(const String& formatted) {
  _sendCommand1(layerId, C_showformatted, formatted);
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
  DDObject(DD_OBJECT_TYPE_TUNNEL), type(type), tunnelId(String(tunnelId)), params(params), endPoint(endPoint) {
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
    String data;
    data.concat(type);
    if (params.length() > 0) {
      data.concat(":");
      data.concat(params);
    }
    data.concat("@");
    data.concat(endPoint);
    _sendSpecialCommand("lt", tunnelId, "reconnect", data/*type + "@" + endPoint*/);
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
#ifdef DEBUG_TUNNEL_RESPONSE                
Serial.print("COUNT: ");
Serial.println(count);
#endif
  return count;
}
bool DDBufferedTunnel::_eof() {
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
bool JsonDDTunnel::read(String& fieldId, String& fieldValue) {
  fieldId = "";
  if (!_readLine(fieldValue)) {
    return false;
  }
  int idx = fieldValue.indexOf(":");
  if (idx != -1) {
    fieldId = fieldValue.substring(0, idx);
    fieldValue = fieldValue.substring(idx + 1);
  }
  return true;
  // String buffer;
  // if (!_readLine(buffer)) {
  //   return false;
  // }
  // int idx = buffer.indexOf(":");
  // if (idx != -1) {
  //   fieldId = buffer.substring(0, idx);
  //   fieldValue = buffer.substring(idx + 1);
  // } else {
  //   fieldId = "";
  //   fieldValue = buffer;
  // }
  // return true;
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



// DumbDisplay::DumbDisplay(DDInputOutput* pIO, DDSerialProxy* pDDSerialProxy) {
//   if (pIO->isSerial() || pIO->isBackupBySerial()) {
//     //_The_DD_Serial = new DDSerial();
//     _The_DD_Serial = pDDSerialProxy;
//   }
//   _IO = pIO;
// }
void DumbDisplay::initialize(DDInputOutput* pIO) {
  _IO = pIO;
}
void DumbDisplay::connect() {
  _Connect();
}
bool DumbDisplay::connected() {
  return _Connected;
}
int DumbDisplay::getConnectVersion() {
  return _ConnectVersion;
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
PlotterDDLayer* DumbDisplay::createPlotterLayer(int width, int height, int pixelsPerSecond) {
  int lid = _AllocLid();
  String layerId = String(lid);
  _sendCommand4(layerId, "SU", String("plotterview"), String(width), String(height), String(pixelsPerSecond));
  PlotterDDLayer* pLayer = new PlotterDDLayer(lid);
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
}
void DumbDisplay::playbackLayerSetupCommands(const String& persist_id) {
  _sendCommand2("", C_SAVEC, persist_id, TO_BOOL(true));
  _sendCommand0("", C_PLAYC);
#ifdef SUPPORT_RECONNECT
  _ConnectedIOProxy->setReconnectRCId(persist_id);
#endif
}
void DumbDisplay::recordLayerCommands() {
  _Connect();
  _sendCommand0("", C_RECC);
}
void DumbDisplay::stopRecordLayerCommands() {
  _sendCommand0("", "STOPC");
}
void DumbDisplay::playbackLayerCommands() {
  _sendCommand0("", C_PLAYC);
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
  _sendCommand0("", ("// " + comment).c_str());
  // if (true) {
  //   int idx = comment.indexOf('\n');
  //   if (idx != -1) {
  //       String com1 = comment.substring(0, idx);
  //       String com2 = comment.substring(idx + 1);
  //       _sendCommand0("", ("// " + com1).c_str());
  //       writeComment(com2);
  //   } else {
  //     _sendCommand0("", ("// " + comment).c_str());
  //   }  
  // } else {
  //   _sendCommand0("", ("// " + comment).c_str());
  // }
}
void DumbDisplay::tone(uint32_t freq, uint32_t duration) {
  _Connect();
  _sendCommand2("", C_TONE, TO_C_INT(freq), TO_C_INT(duration));
}
void DumbDisplay::saveImage(const String& imageName, const uint8_t *bytes, int byteCount) {
  _sendCommand1("", C_SAVEIMG, imageName);
  _sendByteArrayAfterCommand(bytes, byteCount);
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
SimpleToolDDTunnel* DumbDisplay::createImageDownloadTunnel(const String& endPoint, const String& imageName) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  SimpleToolDDTunnel* pTunnel = new SimpleToolDDTunnel("dddownloadimage", tid, imageName, endPoint, true, 1);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}

void DumbDisplay::deleteTunnel(DDTunnel *pTunnel) {
  pTunnel->release();
  delete pTunnel;
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
#ifdef DD_CAN_TURN_OFF_CONDENSE_COMMAND
void DumbDisplay::optionNoCompression(bool noCompression) {
  _NoEncodeInt = noCompression;
}
#endif
void DumbDisplay::setIdleCalback(DDIdleCallback idleCallback) {
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



