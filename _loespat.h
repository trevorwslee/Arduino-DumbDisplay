#ifndef DEF_TIMEOUT_MS
  #define DEF_TIMEOUT_MS 10000
#endif
#ifndef RECEIVE_BUFFER_LEN
  #define RECEIVE_BUFFER_LEN 128
#endif


// #define DEBUG_ESP_AT
// #define FORCE_DEBUG_NO_SILENT


//#define DEBUG_ACTIVE_BUFFER         true

// *** if ACTIVE_RECEIVE_BUFFER_LEN is defined, assume a single client
#if defined(DEBUG_ACTIVE_BUFFER)
  #define ACTIVE_RECEIVE_BUFFER_LEN   64
#else
  #define ACTIVE_RECEIVE_BUFFER_LEN   RECEIVE_BUFFER_LEN
#endif


#define CHECK_STATE_FALLBACK
#define CHECK_SERVER_FALLBACK
#define CHECK_CLIENT_FALLBACK

// *** if SHARED_RECEIVE_RESPONSE, assume called in serial
#define SHARED_RECEIVE_RESPONSE

// NO echo AT seems cause more unexpected AT responses, and therefore more error-prone
//#define NO_AT_ECHO
//#define YIELD_WHEN_LOOP

namespace LOEspAt {

#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
  char ReceiveBuffer[ACTIVE_RECEIVE_BUFFER_LEN + 1];
  uint16_t ReceiveBufferStartIdx = 0;
  uint16_t ReceiveBufferEndIdx= 0;
  void _Concat(String& data, char* p, int len) {
    char c = *(p + len);
    *(p + len) = 0;
    data.concat(p);
    *(p + len) = c;
  }
  bool RetrieveReceivedData(String& data) {
    if (ReceiveBufferStartIdx == ReceiveBufferEndIdx) {
      return false;
    }
    data = "";
    if (ReceiveBufferStartIdx > ReceiveBufferEndIdx) {
      int part_len = ACTIVE_RECEIVE_BUFFER_LEN - ReceiveBufferStartIdx;
      _Concat(data, ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
      _Concat(data, ReceiveBuffer, ReceiveBufferEndIdx);  
      // data.concat(ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
      // data.concat(ReceiveBuffer, ReceiveBufferEndIdx);  
    } else {
      int part_len = ReceiveBufferEndIdx - ReceiveBufferStartIdx;
      _Concat(data, ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
      //data.concat(ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
    }
    if (true) {
      ReceiveBufferStartIdx = ReceiveBufferEndIdx;
    } else {
      ReceiveBufferStartIdx = 0;
      ReceiveBufferEndIdx = 0;
    }
    return true;
  }
  void AppendDataToBeReceived(String& data) {
    int data_len = data.length();
    if ((ReceiveBufferEndIdx + data_len) <= ACTIVE_RECEIVE_BUFFER_LEN) {
      strncpy(ReceiveBuffer + ReceiveBufferEndIdx, data.c_str(), data_len);
      ReceiveBufferEndIdx += data_len;
      // Serial.print("11111 ");
      // Serial.print("ReceiveBufferEndIdx=");
      // Serial.println(ReceiveBufferEndIdx);
    } else {
      int part1_len = ACTIVE_RECEIVE_BUFFER_LEN - ReceiveBufferEndIdx;
      int part2_len = data_len - part1_len;
      if (part2_len >= ReceiveBufferStartIdx) {
        part2_len = ReceiveBufferStartIdx - 1;  // may cause data lost
      }
      strncpy(ReceiveBuffer + ReceiveBufferEndIdx, data.c_str(), part1_len);
      strncpy(ReceiveBuffer, data.c_str() + part1_len, part2_len);
      ReceiveBufferEndIdx = part2_len;
      // Serial.print("22222 ");
      // Serial.print("ReceiveBufferEndIdx=");
      // Serial.println(ReceiveBufferEndIdx);
    }
    // Serial.print(ACTIVE_RECEIVE_BUFFER_LEN);
    // Serial.print(" : ");      
    // Serial.print("ReceiveBufferStartIdx=");
    // Serial.print(ReceiveBufferStartIdx);
    // Serial.print(" / ");      
    // Serial.print("ReceiveBufferEndIdx=");
    // Serial.print(ReceiveBufferEndIdx);
    // Serial.println();      
  }
#endif  



  const long DefAtTimeout = DEF_TIMEOUT_MS;
  const bool DefSendReceiveDataSilent = true;

  class AtResposeInterpreter {
    public:
        virtual bool intepret(int response_idx, String& response) { return false; }
  };
  class AtResposeInterpreter1: public AtResposeInterpreter {
    public:
      AtResposeInterpreter1(int want_response_idx) {
        this->want_response_idx = want_response_idx;
      }
      void reset() {
        this->response = "";
      }
      virtual bool intepret(int response_idx, String& response) {
        if (response_idx == want_response_idx) {
          this->response = response;
        }
        return false;
      }
    public:
      int want_response_idx;
      String response;
  };
  class AtResposeInterpreter2: public AtResposeInterpreter {
    public:
      AtResposeInterpreter2(int want_response_idx) {
        this->want_response_idx = want_response_idx;
      }
      void reset() {
        this->response1 = "";
        this->response2 = "";
      }
      virtual bool intepret(int response_idx, String& response) {
        if (response_idx == want_response_idx) {
          this->response1 = response;
        }
        if (response_idx == (want_response_idx + 1)) {
          this->response2 = response;
        }
        return false;
      }
    public:
      int want_response_idx;
      String response1;
      String response2;
  };
  class ReceiveAtDataInterpreter: public AtResposeInterpreter {
    public:
      ReceiveAtDataInterpreter(String& data): data(data) {
        this->total_len = -1;
        this->data = "";
      }
      virtual bool intepret(int response_idx, String& response) {
// Serial.print("{");        
// Serial.print(response);        
// Serial.print(":");        
// Serial.print(total_len);
// Serial.print("!");        
// Serial.print(data);        
// Serial.print("}");        
        if (response_idx > 0) {
          if (total_len == -1) {
            if (response.startsWith("+CIPRECVDATA:")) {
              int idx = response.indexOf(',');
              total_len = response.substring(13, idx).toInt();
              data = response.substring(idx + 1);
              data += "\n";
            }
          } else {
            if ((total_len - data.length()) > 0) {
              data += response;
              data += "\n";
            }
          }
          if (total_len != -1) {
            return (total_len - data.length()) > 0;
          }
        }
        return false;
      }
    private:
      int total_len;
      String& data;
  };
  String ReceiveResponse;
  bool _ReceiveAtResponse(bool for_receive_data, const char* at_command, AtResposeInterpreter *response_interpreter, long timeout_ms, bool silent) {
#if defined(FORCE_DEBUG_NO_SILENT)
      silent = false;
#endif
      long start_ms = millis();
      bool ok = false;
#if defined(NO_AT_ECHO)    
      int response_idx = 1;
#else
      int response_idx = at_command != NULL ? 0 : 1;
#endif
      while (true) {
#if defined(SHARED_RECEIVE_RESPONSE)
        ReceiveResponse = "";
        while (true) {
          long diff = millis() - start_ms;
          if (diff > timeout_ms) {
#ifdef DEBUG_ESP_AT
            if (!silent) {
              Serial.println("TIMEOUT_READ");
            }  
#endif
            ok = false;
            return false;
          }  
#if defined(YIELD_WHEN_LOOP)
          yield();
#endif
          if (ESP_SERIAL.available()) {
            char c = ESP_SERIAL.read();
            // Serial.print("[");
            // Serial.print(c);
            // Serial.print("]");
            if (c == '\n') {
              break;
            }
            ReceiveResponse += c;
          }
        }
        String& response = ReceiveResponse;
#else
        String response = ESP_SERIAL.readStringUntil('\n');
#endif
#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
        if (response.startsWith("+IPD,")) {
          int idx1 = response.indexOf(',', 5);
          int idx2 = idx1 != -1 ? response.indexOf(':', idx1) : -1;
#ifdef DEBUG_ESP_AT
          Serial.print("$");
          Serial.print("IPD"/*response*/);
          Serial.print(".");
          Serial.print(idx1);
          Serial.print(".");
          Serial.print(idx2);
          Serial.print(".");
#endif
#if defined(SHARED_RECEIVE_RESPONSE)
          int len = -1;  
          if (idx1 != -1 && idx2 != -1) {
            len = response.substring(idx1 + 1, idx2).toInt();
            //String data = response.substring(idx2 + 1) + "\n"; 
            String& data = response;
            data.remove(0, idx2 + 1);
            data += "\n";
#else
          if (idx1 != -1 && idx2 != -1) {
            String data = response.substring(idx2 + 1) + "\n"; 
            int len = response.substring(idx1 + 1, idx2).toInt();
#endif
#ifdef DEBUG_ESP_AT
            Serial.print("(");
            Serial.print(len);
            Serial.print("):");
            Serial.print(data);
            Serial.print(":");
#endif
            while ((len - data.length()) > 0) {
              long diff = millis() - start_ms;
              if (diff > timeout_ms) {
#ifdef DEBUG_ESP_AT
                if (!silent) {
                  Serial.println("TIMEOUT_RECV");
                }  
#endif
                ok = false;
                return false;
              }  
#if defined(YIELD_WHEN_LOOP)
              yield();
              if (!ESP_SERIAL.available()) {
                continue;
              }
#endif
              int c = ESP_SERIAL.read();
              if (c >= 0) {
                data += (char) c;
              }
            }
            AppendDataToBeReceived(data);
#ifdef DEBUG_ESP_AT
            Serial.print("=");
            Serial.print(data.length());
            Serial.print("$");
#endif
            if (for_receive_data) {
              return true;
            }
          }
          continue;
        }
#endif        
        if (for_receive_data) {
#ifdef DEBUG_ESP_AT
          Serial.print("UNEXPECTED AT response [");
          Serial.print(response);
          Serial.println("]");
#endif        
          return false;  // not expected
        }
        int len = response.length();
        while (len > 0) {
          if (response.charAt(0) == '\r') {
#if defined(SHARED_RECEIVE_RESPONSE)
            response.remove(0, 1);
#else
            response = response.substring(1);
#endif            
            len -= 1;
          } else if (response.charAt(len - 1) == '\r') {
#if defined(SHARED_RECEIVE_RESPONSE)
            response.remove(len - 1, 1);
#else
            response = response.substring(0, len - 1);
#endif
            len -= 1;
          } else {
            break;
          }
        }
        // if (len > 0 && response.charAt(len - 1) == '\r') {
        //   response = response.substring(0, len - 1);
        //   len -= 1;
        // }
        if (true) {
          if (response_idx == 0 && len > 0 && (len < 2 || strcmp(response.c_str(), at_command) != 0/*!response.startsWith("AT")*/)) {
#ifdef DEBUG_ESP_AT
            if (!silent) {
              Serial.print("<x");
              Serial.print(response.c_str());
              //Serial.print(strlen(response.c_str()));
              //Serial.print("/");
              //Serial.print(at_command);
              //Serial.print(strlen(at_command));
              Serial.print("x>");
            }
#endif          
            response = "";
            len = 0;
          }
        }
        // if (check_once_only) {
        //   break;
        // }
        if (len/*response.length()*/ == 0) {
          long diff = millis() - start_ms;
          if (diff > timeout_ms) {
#ifdef DEBUG_ESP_AT
            if (!silent) {
              Serial.println("TIMEOUT_1");
            }  
#endif
            ok = false;
            return false;
          }  
          continue;
        }
#ifdef DEBUG_ESP_AT
        if (!silent) {
          Serial.print("[");
          Serial.print(response);
          Serial.print("]");
        }
#endif
        if (response_interpreter != NULL) {
          bool expecting_next = response_interpreter->intepret(response_idx, response);
          if (expecting_next) {
            response_idx++;
            continue;
          }
        }
        response_idx++;
        if (response == "OK" || response == "SEND OK" || response == "SET OK") {
          ok = true;
          break;
        }
        if (response == "ERROR" || response == "SEND FAIL") {
          ok = false;
          break;
        }
        long diff_ms = millis() - start_ms;
        if (diff_ms > timeout_ms) {
#ifdef DEBUG_ESP_AT
          if (!silent) {
            Serial.println("TIMEOUT");
          }  
#endif
          ok = false;
          break;
        }
      }
#ifdef DEBUG_ESP_AT
      if (!silent) {
        Serial.println();
      }
#endif
      return ok;
  }
  inline bool ReceiveAtResponse(const char* at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return _ReceiveAtResponse(false, at_command, response_interpreter, timeout_ms, silent);
  }
  inline bool ReceiveAtResponse(String& at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return _ReceiveAtResponse(false, at_command.c_str(), response_interpreter, timeout_ms, silent);
  }
  inline bool SendAtCommand(const char* at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
#ifdef DEBUG_ESP_AT
      if (!silent) {
        Serial.print("{");
        Serial.print(at_command);
        Serial.print("}");
      }
#endif
      if (true) {
        ESP_SERIAL.print(at_command);
        ESP_SERIAL.print("\r\n");
      } else {
        ESP_SERIAL.println(at_command);
      }
// #if defined(YIELD_WHEN_LOOP)
//       delay(100);
// #endif
      return ReceiveAtResponse(at_command, response_interpreter, timeout_ms, silent);
  }
  inline bool SendAtCommand(String& at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return SendAtCommand(at_command.c_str(), response_interpreter, timeout_ms, silent);
  }
  bool SendDataToClient(int link_id, const char* data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    int data_len = strlen(data);
    String at_command = String("AT+CIPSEND=") + String(link_id) + String(",") + String(data_len);
    bool ok = false;
    if (SendAtCommand(at_command, NULL, timeout_ms, silent)) {
      ESP_SERIAL.print(data);
      ok = ReceiveAtResponse(NULL, NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendDataToClient(int link_id, const uint8_t* data, int data_len, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    String at_command = String("AT+CIPSEND=") + String(link_id) + String(",") + String(data_len);
    bool ok = false;
    if (SendAtCommand(at_command, NULL, timeout_ms, silent)) {
      ESP_SERIAL.write(data, data_len);
      ok = ReceiveAtResponse(NULL, NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendDataToClient(int link_id, uint8_t data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    String at_command = String("AT+CIPSEND=") + String(link_id) + String(",1");
    bool ok = false;
    if (SendAtCommand(at_command, NULL, timeout_ms, silent)) {
      ESP_SERIAL.write(data);
      ok = ReceiveAtResponse(at_command, NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendDataToClient(int link_id, const String& data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    return SendDataToClient(link_id, data.c_str(), timeout_ms, silent);
  }
  bool ReceiveDataFromClient(int link_id, String& data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
    if (ESP_SERIAL.available()) {
      _ReceiveAtResponse(true, NULL, NULL, timeout_ms, silent);
    }
    return RetrieveReceivedData(data);
#else
   ReceiveAtDataInterpreter interpreter(data);
    if (SendAtCommand(String("AT+CIPRECVDATA=") + String(link_id) + String(",102400"), &interpreter, timeout_ms, silent)) {
      return data.length() > 0;
    }
    return false;
#endif    
  }

  inline bool CheckAt() {
    return SendAtCommand("AT");
  }

  bool InitAt() {
#if defined(ACTIVE_RECEIVE_BUFFER_LEN) && defined(DEBUG_ACTIVE_BUFFER)
    // ***** debugging code
    String src_data = "0123456789abcdef";
    for (int i = 0; i < 100; i++) {
      String data;
      bool retrieved = RetrieveReceivedData(data);
      if (retrieved) {
        Serial.print("- [");
        Serial.print(data);
        Serial.println({"]"});
      }
      AppendDataToBeReceived(src_data);
      AppendDataToBeReceived(src_data);
      AppendDataToBeReceived(src_data);
    }
    for (int i = 0; i < 10; i++) {
      Serial.println("... check debug data ...");
      delay(3000);
    }
#endif

#if defined(NO_AT_ECHO)    
    SendAtCommand("ATE0");
#else
    SendAtCommand("ATE1");
#endif    
    if (true) {
      // disconnect AP
      //SendAtCommand("AT+CWQAP");
      // no auto reconnect
      SendAtCommand("AT+CWAUTOCONN=0");
      //SendAtCommand("AT+CWRECONNCFG=0,0");
    }
    return CheckAt();
  }

  // IMPORTANT: afterward, please set baud rate of ESP_SERIAL
  void SetBaudRate(uint32_t baud_rate) {
    if (true) {
      String at_command = String("AT+UART_CUR=") + String(baud_rate) + String(",8,1,0,0");
      ESP_SERIAL.print(at_command);
      ESP_SERIAL.print("\r\n");
    } else {
      SendAtCommand(String("AT+UART_CUR=") + String(baud_rate) + String(",8,1,0,0"));
    }
  }

#if defined(SHARED_RECEIVE_RESPONSE)
    AtResposeInterpreter1 AtResponseInterpreter1(1);
    AtResposeInterpreter2 AtResponseInterpreter2(1);
#endif

  //  0: no AP
  //  1: AP checked
  // -1: failed to check
  int CheckAP() {
#if defined(SHARED_RECEIVE_RESPONSE)
    AtResposeInterpreter1& at_response_interpreter = AtResponseInterpreter1;
    at_response_interpreter.reset();
#else
    AtResposeInterpreter1 at_response_interpreter(1);
#endif    
    if (SendAtCommand("AT+CWJAP?", &at_response_interpreter)) {
      // No AP or +CWJAP:"TrevorWireless","c0:c9:e3:ac:92:f3",7,-40
      if (at_response_interpreter.response.startsWith("+CWJAP:")) {
        return 1;
      }
      return 0;
    }
    return -1;
  }

  // seems not for ESP8266
  //  0: ESP32 station has not started any Wi-Fi connection.
  //  1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
  //  2: ESP32 station has connected to an AP, and got an IPv4 address.
  //  3: ESP32 station is in Wi-Fi connecting or reconnecting state.
  //  4: ESP32 station is in Wi-Fi disconnected state.
  // -1: filed to check
  int CheckState() {
#if defined(SHARED_RECEIVE_RESPONSE)
    AtResposeInterpreter1& at_response_interpreter = AtResponseInterpreter1;
    at_response_interpreter.reset();
#else
    AtResposeInterpreter1 at_response_interpreter(1);
#endif    
    if (SendAtCommand("AT+CWSTATE?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CWSTATE:")) {
        int state = at_response_interpreter.response.substring(9, at_response_interpreter.response.indexOf(',')).toInt();
        return state;
      }
    } else {
#if defined(CHECK_STATE_FALLBACK)
      int ap_state = CheckAP();
      if (ap_state == 0) {
        return 0;
      } else if (ap_state == 1) {
        return 2;
      }
#endif
    }
    return -1;
  }



  //  0: not running
  //  1: running
  // -1: filed to check
  int CheckServerState() {
#if defined(SHARED_RECEIVE_RESPONSE)
    AtResposeInterpreter1& at_response_interpreter = AtResponseInterpreter1;
    at_response_interpreter.reset();
#else
    AtResposeInterpreter1 at_response_interpreter(1);
#endif    
    if (SendAtCommand("AT+CIPSERVER?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CIPSERVER:")) {
        int state = at_response_interpreter.response.substring(11, at_response_interpreter.response.indexOf(',')).toInt();
        return state;
      }
    } else {
#if defined(CHECK_SERVER_FALLBACK)   
      return 1;  // assume server is running   
#endif
    }
    return -1;
  }

  // inline bool Restore() {
  //   return SendAtCommand("AT+RESTORE");
  //   // bool ok = false;
  //   // if (SendAtCommand("AT")) {
  //   //   if (SendAtCommand("AT+RESTORE")) {
  //   //     ok = true;
  //   //   }
  //   // }
  //   // return ok;
  // }
  inline bool Reset() {
    return SendAtCommand("AT+RST");
    // bool ok = false;
    // if (SendAtCommand("AT")) {
    //   if (SendAtCommand("AT+RST")) {
    //     ok = true;
    //   }
    // }
    // return ok;
  }

  inline bool SetStationMode() {
    return SendAtCommand("AT+CWMODE=1");
  }


  bool ConnectAP(const char* ssid, const char* password, String& ip) {
    bool connected = false;
    if (SendAtCommand(String("AT+CWJAP=\"") + ssid + String("\",\"") + password + String("\""))) {
      connected = CheckAP() == 1;
#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
      SendAtCommand("AT+CIPRECVMODE=0");
#else
      connected = connected && SendAtCommand("AT+CIPRECVMODE=1");
#endif
    }
    // if (SendAtCommand("AT")) {
    //   if (SendAtCommand(String("AT+CWJAP=\"") + ssid + String("\",\"") + password + String("\""))) {
    //     connected = SendAtCommand("AT+CIPRECVMODE=1");
    //   }
    // }
    if (connected) {
#if defined(SHARED_RECEIVE_RESPONSE)
      AtResposeInterpreter1& at_response_interpreter = AtResponseInterpreter1;
      at_response_interpreter.reset();
#else
      AtResposeInterpreter1 at_response_interpreter(1);
#endif      
      if (SendAtCommand("AT+CIPSTA?", &at_response_interpreter)) {
        int idx1 = at_response_interpreter.response.indexOf(":ip:\"");
        int idx2 = at_response_interpreter.response.lastIndexOf("\"");
        if (idx1 != -1 && idx2 != -1) {
          ip = at_response_interpreter.response.substring(idx1 + 5, idx2);
        }
      } else {
        connected = false;
      }
    }
    return connected;
  }

  inline bool DisconnectAP() {
    return SendAtCommand("AT+CWQAP");
  }

  bool StartServer(uint16_t port) {
    bool started = false;
    if (SendAtCommand("AT+CIPMUX=1")) {  // *** must accept multiple connections to start a server; and as a result, cannot use pass through send data mode (AT+CIPMODE=1) 
      if (SendAtCommand(String("AT+CIPSERVER=1,") + String(port))) {
        started = true;
      }
    }
    return started;
  }

  int CheckForClientConnection() {
    int link_id = -1;
#if defined(SHARED_RECEIVE_RESPONSE)
    AtResposeInterpreter1& at_response_interpreter = AtResponseInterpreter1;
    at_response_interpreter.reset();
#else
    AtResposeInterpreter1 at_response_interpreter(1);
#endif    
    if (SendAtCommand("AT+CIPSTATE?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CIPSTATE:")) {
        int idx = at_response_interpreter.response.indexOf(",");
        if (idx != -1) {
          link_id = at_response_interpreter.response.substring(10, idx).toInt();
        }
      }
    } else {
#if defined(CHECK_CLIENT_FALLBACK)
#if defined(SHARED_RECEIVE_RESPONSE)
      AtResposeInterpreter2& at_response_interpreter_2 = AtResponseInterpreter2;
      at_response_interpreter_2.reset();
#else
      AtResposeInterpreter2 at_response_interpreter_2(1);
#endif      
      if (SendAtCommand("AT+CIPSTATUS", &at_response_interpreter_2)) {
        // 1. STATUS:3
        // 2. +CIPSTATUS:0,"TCP","192.168.0.98",38774,10201,1
        if (at_response_interpreter_2.response1 == "STATUS:3") {
          if (at_response_interpreter_2.response2.startsWith("+CIPSTATUS:")) {
            int idx = at_response_interpreter_2.response2.indexOf(",");
// Serial.print("***");
// Serial.print(idx);            
            if (idx != -1) {
              link_id = at_response_interpreter_2.response2.substring(11, idx).toInt();
            }
          }
        }
      }
#endif
    }
    return link_id;
  }

  bool DisconnectClient(int link_id) {
    return SendAtCommand(String("+CIPCLOSE=") + String(link_id));
  }

};

