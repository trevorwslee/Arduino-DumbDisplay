
// #define DEBUG_ESP_AT
// #define FORCE_DEBUG_NO_SILENT

#define NO_AT_ECHO

// seems SUPPORT_CHECK_STATE not for ESP8266 
//#define SUPPORT_CHECK_STATE


#define DEBUG_ACTIVE_BUFFER         false
#define ACTIVE_RECEIVE_BUFFER_LEN   (DEBUG_ACTIVE_BUFFER ? 64 : 5 * 1025)

#define CHECK_CLIENT_FALLBACK

namespace LOEspAt {

#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
  char ReceiveBuffer[ACTIVE_RECEIVE_BUFFER_LEN];
  int ReceiveBufferStartIdx = 0;
  int ReceiveBufferEndIdx= 0;
  bool RetrieveReceivedData(String& data) {
    if (ReceiveBufferStartIdx == ReceiveBufferEndIdx) {
      return false;
    }
    data = "";
    if (ReceiveBufferStartIdx > ReceiveBufferEndIdx) {
      int part_len = ACTIVE_RECEIVE_BUFFER_LEN - ReceiveBufferStartIdx;
      data.concat(ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
      data.concat(ReceiveBuffer, ReceiveBufferEndIdx);  
    } else {
      int part_len = ReceiveBufferEndIdx - ReceiveBufferStartIdx;
      data.concat(ReceiveBuffer + ReceiveBufferStartIdx, part_len);  
    }
    if (true) {
      ReceiveBufferStartIdx = ReceiveBufferEndIdx;
    } else {
      ReceiveBufferStartIdx = 0;
      ReceiveBufferEndIdx = 0;
    }
    return true;
  }
  void AppendDataToBeReceived(String data) {
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



  const long DefAtTimeout = 15000;
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
  bool _ReceiveAtResponse(const char* at_command, AtResposeInterpreter *response_interpreter, long timeout_ms, bool silent) {
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
        String response;
        response = ESP_SERIAL.readStringUntil('\n');
        int len = response.length();
        if (len > 0 && response.charAt(len - 1) == '\r') {
          response = response.substring(0, len - 1);
          len -= 1;
        }
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
          String data = response.substring(idx2 + 1) + "\n"; 
          if (idx1 != -1 && idx2 != -1) {
            int len = response.substring(idx1 + 1, idx2).toInt();
#ifdef DEBUG_ESP_AT
            Serial.print("(");
            Serial.print(len);
            Serial.print("):");
#endif
            while ((len - data.length()) > 0) {
              long diff = millis() - start_ms;
              if (diff > timeout_ms) {
#ifdef DEBUG_ESP_AT
                if (!silent) {
                  Serial.println("TIMEOUT3");
                }  
#endif
                ok = false;
                return false;
              }  
              if (ESP_SERIAL.available()) {
                int c = ESP_SERIAL.read();
                if (c >= 0) {
                  data += (char) c;
                }
              }
            }
            AppendDataToBeReceived(data);
          }
#ifdef DEBUG_ESP_AT
          Serial.print("=");
          Serial.print(data.length());
          Serial.print("$");
#endif
        }
#endif        
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
        if (len/*response.length()*/ == 0) {
          long diff = millis() - start_ms;
          if (diff > timeout_ms) {
#ifdef DEBUG_ESP_AT
            if (!silent) {
              Serial.println("TIMEOUT1");
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
            Serial.println("TIMEOUT2");
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
    return _ReceiveAtResponse(at_command, response_interpreter, timeout_ms, silent);
  }
  inline bool ReceiveAtResponse(String& at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return _ReceiveAtResponse(at_command.c_str(), response_interpreter, timeout_ms, silent);
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
#if defined(ACTIVE_RECEIVE_BUFFER_LEN)
    if (DEBUG_ACTIVE_BUFFER) {
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
    }
#endif

#if defined(NO_AT_ECHO)    
    SendAtCommand("ATE0");
#else
    SendAtCommand("ATE1");
#endif    
    // if (true) {
    //   SendAtCommand("AT+CWQAP");
    //   // no auto reconnect
    //   SendAtCommand("AT+CWRECONNCFG=0,1");
    // }
    return CheckAt();
  }

#if defined(SUPPORT_CHECK_STATE)
  //  0: ESP32 station has not started any Wi-Fi connection.
  //  1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
  //  2: ESP32 station has connected to an AP, and got an IPv4 address.
  //  3: ESP32 station is in Wi-Fi connecting or reconnecting state.
  //  4: ESP32 station is in Wi-Fi disconnected state.
  // -1: filed to check
  int CheckState() {
    AtResposeInterpreter1 at_response_interpreter(1);
    if (SendAtCommand("AT+CWSTATE?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CWSTATE:")) {
        int state = at_response_interpreter.response.substring(9, at_response_interpreter.response.indexOf(',')).toInt();
        return state;
      }
    }
    return -1;
  }
#endif  

  //  0: no AP
  //  1: AP checked
  // -1: failed to check
  int CheckAP() {
    AtResposeInterpreter1 at_response_interpreter(1);
    if (SendAtCommand("AT+CWJAP?", &at_response_interpreter)) {
      // No AP or +CWJAP:"TrevorWireless","c0:c9:e3:ac:92:f3",7,-40
      if (at_response_interpreter.response.startsWith("+CWJAP:")) {
        return 1;
      }
      return 0;
    }
    return -1;
  }


  //  0: not running
  //  1: running
  // -1: filed to check
  int CheckServerState() {
    AtResposeInterpreter1 at_response_interpreter(1);
    if (SendAtCommand("AT+CIPSERVER?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CIPSERVER:")) {
        int state = at_response_interpreter.response.substring(11, at_response_interpreter.response.indexOf(',')).toInt();
        return state;
      }
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
      AtResposeInterpreter1 at_response_interpreter(1);
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

  bool StartServer(int port) {
    bool started = false;
    if (SendAtCommand("AT+CIPMUX=1")) {
      if (SendAtCommand(String("AT+CIPSERVER=1,") + String(port))) {
        started = true;
      }
    }
    return started;
  }

  int CheckForClientConnection() {
    int link_id = -1;
    AtResposeInterpreter1 at_response_interpreter(1);
    if (SendAtCommand("AT+CIPSTATE?", &at_response_interpreter)) {
      if (at_response_interpreter.response.startsWith("+CIPSTATE:")) {
        int idx = at_response_interpreter.response.indexOf(",");
        if (idx != -1) {
          link_id = at_response_interpreter.response.substring(10, idx).toInt();
        }
      }
    } else {
#if defined(CHECK_CLIENT_FALLBACK)
      AtResposeInterpreter2 at_response2_interpreter(1);
      if (SendAtCommand("AT+CIPSTATUS", &at_response2_interpreter)) {
        // 1. STATUS:3
        // 2. +CIPSTATUS:0,"TCP","192.168.0.98",38774,10201,1
        if (at_response2_interpreter.response1 == "STATUS:3") {
          if (at_response2_interpreter.response2.startsWith("+CIPSTATUS:")) {
            int idx = at_response2_interpreter.response2.indexOf(",");
// Serial.print("***");
// Serial.print(idx);            
            if (idx != -1) {
              link_id = at_response2_interpreter.response2.substring(11, idx).toInt();
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

