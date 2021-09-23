#include "Arduino.h"

#include "dumbdisplay.h"


#define HAND_SHAKE_GAP 1000

#define ENABLE_FEEDBACK
#define STORE_LAYERS
#define HANDLE_FEEDBACK_DURING_DELAY

#define FEEDBACK_BUFFER_SIZE 4

#define READ_BUFFER_USE_BUFFER

#define TO_BOOL(val) (val ? "1" : "0")

//#define DD_DEBUG_HS
//#define DD_DEBUG_SEND_COMMAND
//#define DEBUG_ECHO_COMMAND
//#define DEBUG_RECEIVE_FEEDBACK
//#define DEBUG_ECHO_FEEDBACK

#define VALIDATE_CONNECTION

#define DEBUG_WITH_LED


// not flush seems to be a bit better for Serial (lost data)
#define FLUSH_AFTER_SENT_COMMAND false
#define YIELD_AFTER_SEND_COMMAND false


DDSerial* _The_DD_Serial = NULL;

namespace DDImpl {


class IOProxy {
  public: 
    IOProxy(DDInputOutput *pIO) {
      this->pIO = pIO;
    }
    bool available();
    const String& get();
    void clear();
    void print(const String &s);
    void print(const char *p);
    void keepAlive();
    void validConnection();
  private:
    DDInputOutput *pIO;
    bool fromSerial;
    String data;  
};

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
void IOProxy::keepAlive() {
  pIO->keepAlive();
}
void IOProxy::validConnection() {
  pIO->validConnection();
}



//volatile bool _Preconneced = false;
volatile bool _Connected = false;
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
  _IO->preConnect();
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
    while (true) {
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
      long now = millis();
      if (now > nextTime) {
#ifdef DEBUG_WITH_LED
        if (debugLedPin != -1) {
          debugLedOn = !debugLedOn;
          digitalWrite(debugLedPin, debugLedOn ? HIGH : LOW);
        }
#endif
//Serial.println((_ConnectedFromSerial ? "SERIAL" : "NON-SERIAL"));
        ioProxy.print(">init>:Arduino-c1\n");
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
  int lid = _LayerIdToLid(pTunnel->getTunnelId());
  _DDLayerArray[lid] = pTunnel;
#endif
}
void _PreDeleteTunnel(DDTunnel* pTunnel) {
#ifdef STORE_LAYERS  
  int lid = _LayerIdToLid(pTunnel->getTunnelId());
  _DDLayerArray[lid] = NULL;
#endif
}
#endif

#ifdef VALIDATE_CONNECTION
long _LastValidateConnectionMillis = 0;
#endif
String* _ReadFeedback(String& buffer) {
  if (_ConnectedIOProxy == NULL || !_ConnectedIOProxy->available()) {
    return NULL;
  }
#ifdef VALIDATE_CONNECTION
    long now = millis();
    long diff = now - _LastValidateConnectionMillis;
    if (diff >= 5000) {
      _ConnectedIOProxy->validConnection();
      _LastValidateConnectionMillis = now;
    }
#endif
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
      if (*(pFeedback->c_str()) == '<') {
        if (pFeedback->length() == 1) {
          // keep alive
          _ConnectedIOProxy->keepAlive();
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
//Serial.println(String("// ") + (final ? "F" : "."));
//Serial.println("LT++++" + data + " - " + String(final));
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
      int x = -1;
      int y = -1;
      char* token = strtok(buf, ".");
      if (token != NULL) {
        lid = _LayerIdToLid(token);
        token = strtok(NULL, ":");
      }
      if (token != NULL) {
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        x = atoi(token);
        token = strtok(NULL, ",");
      }
      if (token != NULL) {
        y = atoi(token);
        ok = true;
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
            handler(pLayer, CLICK, x, y);
            //_SendCommand("", ("// feedback (" + String(lid) + ") -- " + *pFeedback).c_str());
          } else {
            DDFeedbackManager *pFeedbackManager = pLayer->getFeedbackManager();
            if (pFeedbackManager != NULL) {
              pFeedbackManager->pushFeedback(x, y);
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
  }
}

inline void _Delay(unsigned long ms) {
//Serial.println("// {");
#ifdef ENABLE_FEEDBACK
  unsigned long delayMillis = ms;
	unsigned long startMillis = millis();
  while (true) {
    _HandleFeedback();
    unsigned long remainMillis = delayMillis - (millis() - startMillis);
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
//Serial.println("// }");
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





}



//*************/
/** EXPORTED **/
//*************/



// class FeedbackManager {
//   public: 
//     FeedbackManager(int bufferSize = 16) {
//       this->feedbackValid = false;
//       this->feedbackArray = new DDFeedback[bufferSize];
//       // this->xArray = new int[bufferSize];
//       // this->yArray = new int[bufferSize];
//       this->arraySize = bufferSize;
//       this->nextArrayIdx = 0;
//     }
//     ~FeedbackManager() {
//       delete feedbackArray;
//       // delete this->xArray;
//       // delete this->yArray;
//     }
//     // bool checkFeedback() {
//     //   bool checkResult = feedbackValid;
//     //   feedbackValid = false;
//     //   return checkResult;
//     // }
//     const DDFeedback* getFeedback() {
//       if (!feedbackValid) return NULL;
//       const DDFeedback* pFeedback = &feedbackArray[nextArrayIdx];
//       feedbackValid = false;
//       return pFeedback;
//       //return feedbackArray[nextArrayIdx];
//       // DDFeedback feedback;
//       // feedback.x = xArray[nextArrayIdx];
//       // feedback.y = yArray[nextArrayIdx];
//       // feedbackValid = false;
//       // return feedback;
//     }
//     void pushFeedback(int x, int y) {
//       feedbackArray[nextArrayIdx].x = x;
//       feedbackArray[nextArrayIdx].y = y;
//       // xArray[nextArrayIdx] = x;
//       // yArray[nextArrayIdx] = y;
//       nextArrayIdx  = (nextArrayIdx + 1) % arraySize;
//       feedbackValid = true;
//     }
//   private:
//     bool feedbackValid;
//     DDFeedback* feedbackArray;
//     // int *xArray;
//     // int *yArray;
//     int arraySize;
//     int nextArrayIdx;
// };


using namespace DDImpl;


DDFeedbackManager::DDFeedbackManager(int bufferSize) {
  this->feedbackArray = new DDFeedback[bufferSize];
  this->arraySize = bufferSize;
  this->nextArrayIdx = 0;
  this->validArrayIdx = 0;
}
DDFeedbackManager::~DDFeedbackManager() {
  delete feedbackArray;
}
const DDFeedback* DDFeedbackManager::getFeedback() {
  if (nextArrayIdx == validArrayIdx) return NULL;
  const DDFeedback* pFeedback = &feedbackArray[validArrayIdx];
  validArrayIdx = (validArrayIdx + 1) % arraySize;
  return pFeedback;
}
void DDFeedbackManager::pushFeedback(int x, int y) {
  feedbackArray[nextArrayIdx].x = x;
  feedbackArray[nextArrayIdx].y = y;
  nextArrayIdx = (nextArrayIdx + 1) % arraySize;
  if (nextArrayIdx == validArrayIdx)
    validArrayIdx = (validArrayIdx + 1) % arraySize;
}


DDLayer::DDLayer(int layerId) {
  this->layerId = String(layerId);
  this->pFeedbackManager = NULL;
  this->feedbackHandler = NULL;
}
DDLayer::~DDLayer() {
  _PreDeleteLayer(this);
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
} 
void DDLayer::visibility(bool visible) {
  _sendCommand1(layerId, "visible", TO_BOOL(visible));
}
void DDLayer::opacity(int opacity) {
  _sendCommand1(layerId, "opacity", String(opacity));
}
void DDLayer::border(float size, const String& color, const String& shape) {
  _sendCommand3(layerId, "border", String(size), color, shape);
}
void DDLayer::noBorder() {
  _sendCommand0(layerId, "border");
}
void DDLayer::padding(float left, float top, float right, float bottom) {
  _sendCommand4(layerId, "padding", String(left), String(top), String(right), String(bottom));
}
void DDLayer::noPadding() {
  _sendCommand0(layerId, "padding");
}
void DDLayer::clear() {
  _sendCommand0(layerId, "clear");
}
// void DDLayer::backgroundColor(long color) {
//   _sendCommand1(layerId, "bgcolor", HEX_COLOR(color));
// }
void DDLayer::backgroundColor(const String& color) {
  _sendCommand1(layerId, "bgcolor", color);
}
void DDLayer::noBackgroundColor() {
  _sendCommand0(layerId, "nobgcolor");
}
void DDLayer::flash() {
  _sendCommand0(layerId, "flash");
}
void DDLayer::flashArea(int x, int y) {
  _sendCommand2(layerId, "flasharea", String(x), String(y));
}
void DDLayer::writeComment(const String& comment) {
  _sendCommand0("", ("// " + layerId + ": " + comment).c_str());
}
void DDLayer::enableFeedback(const String& autoFeedbackMethod) {
  _sendCommand2(layerId, "feedback", TO_BOOL(true), autoFeedbackMethod);
  feedbackHandler = NULL;
  if (pFeedbackManager != NULL)
    delete pFeedbackManager;
  pFeedbackManager = new DDFeedbackManager(FEEDBACK_BUFFER_SIZE + 1);  // need 1 more slot
}
void DDLayer::disableFeedback() {
  _sendCommand1(layerId, "feedback", TO_BOOL(false));
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
  _sendCommand2(layerId, "feedback", TO_BOOL(enable), autoFeedbackMethod);
  feedbackHandler = handler;
  if (pFeedbackManager != NULL) {
    delete pFeedbackManager;
    pFeedbackManager = NULL;
  }
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
  _sendCommand1(layerId, "pensize", String(size));
}
// void TurtleDDLayer::penColor(long color) {
//   _sendCommand1(layerId, "pencolor", HEX_COLOR(color));
// }
void TurtleDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, "pencolor", color);
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
  _sendCommand1(layerId, "ptextsize", String(size));
}
void TurtleDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, "ptextfont", fontName, String(size));
}
void TurtleDDLayer::dot(int size, const String& color) {
  _sendCommand2(layerId, "dot", String(size), color);
}
void TurtleDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? "ccircle" : "circle", String(radius));
}
void TurtleDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "coval" : "oval", String(width), String(height));
}
void TurtleDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "crect" : "rect", String(width), String(height));
}
void TurtleDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, "trisas", String(side1), String(angle), String(side2));
}
void TurtleDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, "trisas", String(side), String(angle));
}
void TurtleDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, "poly", String(side), String(vertexCount));
}
void TurtleDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? "cpolyin" : "cpoly", String(radius), String(vertexCount));
}
void TurtleDDLayer::write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? "drawtext" : "write", text);
}
void LedGridDDLayer::turnOn(int x, int y) {
  _sendCommand2(layerId, "ledon", String(x), String(y));
}
void LedGridDDLayer::turnOff(int x, int y) {
  _sendCommand2(layerId, "ledoff", String(x), String(y));
}
void LedGridDDLayer::toggle(int x, int y) {
  _sendCommand2(layerId, "ledtoggle", String(x), String(y));
}
void LedGridDDLayer::horizontalBar(int count, bool rightToLeft) {
  _sendCommand2(layerId, "ledhoribar", String(count), TO_BOOL(rightToLeft));
}
void LedGridDDLayer::verticalBar(int count, bool bottomToTop) {
  _sendCommand2(layerId, "ledvertbar", String(count), TO_BOOL(bottomToTop));
}
// void LedGridDDLayer::onColor(long color) {
//   _sendCommand1(layerId, "ledoncolor", HEX_COLOR(color));
// }
void LedGridDDLayer::onColor(const String& color) {
  _sendCommand1(layerId, "ledoncolor", color);
}
// void LedGridDDLayer::offColor(long color) {
//   _sendCommand1(layerId, "ledoffcolor", HEX_COLOR(color));
// }
void LedGridDDLayer::offColor(const String& color) {
  _sendCommand1(layerId, "ledoffcolor", color);
}
void LedGridDDLayer::noOffColor() {
  _sendCommand0(layerId, "ledoffcolor");
}


void LcdDDLayer::print(const String& text) {
  _sendCommand1(layerId, "print", text);
}
void LcdDDLayer::home() {
  _sendCommand0(layerId, "home");
}
void LcdDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, "setcursor", String(x), String(y));
}
void LcdDDLayer::cursor() {
  _sendCommand1(layerId, "cursor", TO_BOOL(true));
}
void LcdDDLayer::noCursor() {
  _sendCommand1(layerId, "cursor", TO_BOOL(false));
}
void LcdDDLayer::autoscroll() {
  _sendCommand1(layerId, "autoscroll", TO_BOOL(true));
}
void LcdDDLayer::noAutoscroll() {
  _sendCommand1(layerId, "autoscroll", TO_BOOL(false));
}
void LcdDDLayer::display() {
  _sendCommand1(layerId, "display", TO_BOOL(true));
}
void LcdDDLayer::noDisplay() {
  _sendCommand1(layerId, "display", TO_BOOL(false));
}
void LcdDDLayer::scrollDisplayLeft() {
  _sendCommand0(layerId, "scrollleft");
}
void LcdDDLayer::scrollDisplayRight() {
  _sendCommand0(layerId, "scrollright");
}
void LcdDDLayer::writeLine(const String& text, int y, const String& align) {
  _sendCommand3(layerId, "writeline", String(y), align, text);
}
void LcdDDLayer::writeCenteredLine(const String& text, int y) {
  _sendCommand3(layerId, "writeline", String(y), "C", text);
} 
void LcdDDLayer::pixelColor(const String &color) {
  _sendCommand1(layerId, "pixelcolor", color);
}
void LcdDDLayer::bgPixelColor(const String &color) {
  _sendCommand1(layerId, "bgpixelcolor", color);
}
void LcdDDLayer::noBgPixelColor() {
  _sendCommand0(layerId, "bgpixelcolor");
}


void GraphicalDDLayer::setCursor(int x, int y) {
  _sendCommand2(layerId, "setcursor", String(x), String(y));
}
void GraphicalDDLayer::moveCursorBy(int byX, int byY) {
  _sendCommand2(layerId, "movecursorby", String(byX), String(byY));
}
// void GraphicalDDLayer::setTextColor(const String& color) {
//   _sendCommand1(layerId, "textcolor", color);
// }
void GraphicalDDLayer::setTextColor(const String& color, const String& bgColor) {
  _sendCommand2(layerId, "textcolor", color, bgColor);
}
void GraphicalDDLayer::setTextSize(int size) {
  _sendCommand1(layerId, "textsize", String(size));
}
void GraphicalDDLayer::setTextFont(const String& fontName, int size) {
  _sendCommand2(layerId, "textfont", fontName, String(size));
}
void GraphicalDDLayer::setTextWrap(bool wrapOn) {
  _sendCommand1(layerId, "settextwrap", TO_BOOL(wrapOn));
}
void GraphicalDDLayer::fillScreen(const String& color) {
  _sendCommand1(layerId, "fillscreen", color);
}
void GraphicalDDLayer::print(const String& text) {
  _sendCommand1(layerId, "print", text);
}
void GraphicalDDLayer::println(const String& text) {
  _sendCommand1(layerId, "println", text);
}
void GraphicalDDLayer::drawChar(int x, int y, char c, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, "drawchar", String(x), String(y), color, bgColor, String(size), String(c));
}
void GraphicalDDLayer::drawStr(int x, int y, const String& string, const String& color, const String& bgColor, int size) {
  _sendCommand6(layerId, "drawstr", String(x), String(y), color, bgColor, String(size), string);
}
void GraphicalDDLayer::drawPixel(int x, int y, const String& color) {
  _sendCommand3(layerId, "drawpixel", String(x), String(y), color);
}
void GraphicalDDLayer::drawLine(int x1, int y1, int x2, int y2, const String& color) {
  _sendCommand5(layerId, "drawline", String(x1), String(y1), String(x2), String(y2), color);
}
void GraphicalDDLayer::drawRect(int x, int y, int w, int h, const String& color, bool filled) {
  _sendCommand6(layerId, "drawrect", String(x), String(y), String(w), String(h), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillRect(int x, int y, int w, int h, const String& color) {
//   _sendCommand6(layerId, "drawrect", String(x), String(y), String(w), String(h), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawCircle(int x, int y, int r, const String& color, bool filled) {
  _sendCommand5(layerId, "drawcircle", String(x), String(y), String(r), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillCircle(int x, int y, int r, const String& color) {
//   _sendCommand5(layerId, "drawcircle", String(x), String(y), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color, bool filled) {
  _sendCommand8(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color) {
//   _sendCommand8(layerId, "drawtriangle", String(x1), String(y1), String(x2), String(y2), String(x3), String(y3), color, TO_BOOL(true));
// }
void GraphicalDDLayer::drawRoundRect(int x, int y, int w, int h, int r, const String& color, bool filled) {
  _sendCommand7(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color, TO_BOOL(filled));
}
// void GraphicalDDLayer::fillRoundRect(int x, int y, int w, int h, int r, const String& color) {
//   _sendCommand7(layerId, "drawroundrect", String(x), String(y), String(w), String(h), String(r), color, TO_BOOL(true));
// }
void GraphicalDDLayer::forward(int distance) {
  _sendCommand1(layerId, "fd", String(distance));
}
void GraphicalDDLayer::leftTurn(int angle) {
  _sendCommand1(layerId, "lt", String(angle));
}
void GraphicalDDLayer::rightTurn(int angle) {
  _sendCommand1(layerId, "rt", String(angle));
}
void GraphicalDDLayer::setHeading(int angle) {
  _sendCommand1(layerId, "seth", String(angle));
}
void GraphicalDDLayer::penSize(int size) {
  _sendCommand1(layerId, "pensize", String(size));
}
void GraphicalDDLayer::penColor(const String& color) {
  _sendCommand1(layerId, "pencolor", color);
}
void GraphicalDDLayer::fillColor(const String& color) {
  _sendCommand1(layerId, "fillcolor", color);
}
void GraphicalDDLayer::noFillColor() {
  _sendCommand0(layerId, "nofillcolor");
}
void GraphicalDDLayer::circle(int radius, bool centered) {
  _sendCommand1(layerId, centered ? "ccircle" : "circle", String(radius));
}
void GraphicalDDLayer::oval(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "coval" : "oval", String(width), String(height));
}
void GraphicalDDLayer::rectangle(int width, int height, bool centered) {
  _sendCommand2(layerId, centered ? "crect" : "rect", String(width), String(height));
}
void GraphicalDDLayer::triangle(int side1, int angle, int side2) {
  _sendCommand3(layerId, "trisas", String(side1), String(angle), String(side2));
}
void GraphicalDDLayer::isoscelesTriangle(int side, int angle) {
  _sendCommand2(layerId, "trisas", String(side), String(angle));
}
void GraphicalDDLayer::polygon(int side, int vertexCount) {
  _sendCommand2(layerId, "poly", String(side), String(vertexCount));
}
void GraphicalDDLayer::centeredPolygon(int radius, int vertexCount, bool inside) {
  _sendCommand2(layerId, inside ? "cpolyin" : "cpoly", String(radius), String(vertexCount));
}
void GraphicalDDLayer:: write(const String& text, bool draw) {
  _sendCommand1(layerId, draw ? "drawtext" : "write", text);
}

void SevenSegmentRowDDLayer::segmentColor(const String& color) {
  _sendCommand1(layerId, "segcolor", color);
}
void SevenSegmentRowDDLayer::turnOn(const String& segments, int digitIdx) {
  _sendCommand2(layerId, "segon", segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::turnOff(const String& segments, int digitIdx) {
  _sendCommand2(layerId, "segoff", segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::setOn(const String& segments, int digitIdx) {
  _sendCommand2(layerId, "setsegon", segments, String(digitIdx));
}
void SevenSegmentRowDDLayer::showNumber(float number, const String& padding) {
  _sendCommand2(layerId, "shownumber", String(number, 5), padding);
}
void SevenSegmentRowDDLayer::showHexNumber(int number) {
  _sendCommand1(layerId, "showhex", String(number));
}
void SevenSegmentRowDDLayer::showFormatted(const String& formatted) {
  _sendCommand1(layerId, "showformatted", formatted);
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
DDTunnel::DDTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize):
  type(type), endPoint(endPoint), tunnelId(String(tunnelId)) {
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
  //nextArrayIdx = 0;
  //validArrayIdx = 0;
  done = false;
  //for (int i = 0; i < arraySize; i++) {
    //dataArray[i] = "";
  //}
  _sendSpecialCommand("lt", tunnelId, "reconnect", type + "@" + endPoint);
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
  return /*nextArrayIdx == validArrayIdx && */done;
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
DDBufferedTunnel::DDBufferedTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize):
  DDTunnel(type, tunnelId, endPoint, connectNow, bufferSize) {
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
  return (arraySize + validArrayIdx - nextArrayIdx) % arraySize;
}
bool DDBufferedTunnel::_eof() {
  return nextArrayIdx == validArrayIdx && this->DDTunnel::_eof();
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
JsonDDTunnelMultiplexer::JsonDDTunnelMultiplexer(JsonDDTunnel** tunnels, int tunnelCount) {
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
void DumbDisplay::pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align) {
  _sendCommand5(pLayer->getLayerId(), "PIN", String(uLeft), String(uTop), String(uWidth), String(uHeight), align);
}
void DumbDisplay::deleteLayer(DDLayer *pLayer) {
  _sendCommand0(pLayer->getLayerId(), "DEL");
  //_PreDeleteLayer(pLayer);
  delete pLayer;
}
void DumbDisplay::recordLayerCommands() {
  _sendCommand0("", "RECC");
}
void DumbDisplay::stopRecordLayerCommands() {
  _sendCommand0("", "STOPC");
}
void DumbDisplay::playbackLayerCommands() {
  _sendCommand0("", "PLAYC");
}
void DumbDisplay::saveLayerCommands(const String& id, bool persist) {
  _sendCommand2("", "SAVEC", id, TO_BOOL(persist));
}
void DumbDisplay::loadLayerCommands(const String& id) {
  _sendCommand1("", "LOADC", id);
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


BasicDDTunnel* DumbDisplay::createBasicTunnel(const String& endPoint, bool connectNow, int bufferSize) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  // if (connectNow) {
  //   _sendSpecialCommand("lt", tunnelId, "connect", "ddbasic@" + endPoint);
  // }
  BasicDDTunnel* pTunnel = new BasicDDTunnel("ddbasic", tid, endPoint, connectNow, bufferSize);
  _PostCreateTunnel(pTunnel);
  return pTunnel;
}
JsonDDTunnel* DumbDisplay::createJsonTunnel(const String& endPoint, bool connectNow, int bufferSize) {
  int tid = _AllocTid();
  String tunnelId = String(tid);
  if (connectNow) {
    _sendSpecialCommand("lt", tunnelId, "connect", "ddsimplejson@" + endPoint);
  }
  JsonDDTunnel* pTunnel = new JsonDDTunnel("ddsimplejson", tid, endPoint, connectNow, bufferSize);
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

void DumbDisplay::debugSetup(int debugLedPin, bool enableEchoFeedback) {
#ifdef DEBUG_WITH_LED
  if (debugLedPin != -1) {
     pinMode(debugLedPin, OUTPUT);
   }
  _DebugLedPin = debugLedPin;
#endif  
#ifdef DEBUG_ECHO_FEEDBACK
  _DebugEnableEchoFeedback = enableEchoFeedback;
#endif
}
// void DumbDisplay::delay(unsigned long ms) {
//   _Delay(ms);
// }


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



