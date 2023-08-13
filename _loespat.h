
#define DEBUG_ESP_AT
#define FORCE_DEBUG_NO_SILENT


namespace LOEspAt {

  const long DefAtTimeout = 1000;
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
  class ReceiveAtDataInterpreter: public AtResposeInterpreter {
    public:
      ReceiveAtDataInterpreter(String& data): data(data) {
        this->total_len = 0;
        this->data = "";
      }
      virtual bool intepret(int response_idx, String& response) {
        if (response_idx > 0) {
          if (response_idx == 1) {
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
          return (total_len - data.length()) > 0;
        }
        return false;
      }
    public:
      int total_len;
      String& data;
  };
  bool _ReceiveAtResponse(AtResposeInterpreter *response_interpreter, long timeout_ms, bool silent) {
#if defined(FORCE_DEBUG_NO_SILENT)
      silent = false;
#endif
      long start_ms = millis();
      bool ok = false;
      int response_idx = 0;
      while (true) {
        String response;
        response = ESP_SERIAL.readStringUntil('\n');
        int len = response.length();
        if (len > 0 && response.charAt(len - 1) == '\r') {
          response = response.substring(0, len - 1);
          len -= 1;
        }
        if (true) {
          if (response_idx == 0 && len > 0 && (len < 2 || !response.startsWith("AT"))) {
#ifdef DEBUG_ESP_AT
            if (!silent) {
              Serial.print("<x");
              Serial.print(response);
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
              Serial.println("TO");
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
            Serial.println("TO");
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
  inline bool ReceiveAtResponse(AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return _ReceiveAtResponse(response_interpreter, timeout_ms, silent);
  }
  inline bool SendAtCommand(const char* at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
      ESP_SERIAL.println(at_command);
      return ReceiveAtResponse(response_interpreter, timeout_ms, silent);
  }
  inline bool SendAtCommand(String& at_command, AtResposeInterpreter *response_interpreter = NULL, long timeout_ms = DefAtTimeout, bool silent = false) {
    return SendAtCommand(at_command.c_str(), response_interpreter, timeout_ms, silent);
  }
  bool SendAtData(int link_id, const char* data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    int data_len = strlen(data);
    bool ok = false;
    if (SendAtCommand(String("AT+CIPSEND=") + String(link_id) + String(",") + String(data_len), NULL, timeout_ms, silent)) {
      ESP_SERIAL.print(data);
      ok = ReceiveAtResponse(NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendAtData(int link_id, const uint8_t* data, int data_len, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    bool ok = false;
    if (SendAtCommand(String("AT+CIPSEND=") + String(link_id) + String(",") + String(data_len), NULL, timeout_ms, silent)) {
      ESP_SERIAL.write(data, data_len);
      ok = ReceiveAtResponse(NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendAtData(int link_id, uint8_t data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    bool ok = false;
    if (SendAtCommand(String("AT+CIPSEND=") + String(link_id) + String(",1"), NULL, timeout_ms, silent)) {
      ESP_SERIAL.write(data);
      ok = ReceiveAtResponse(NULL, timeout_ms, silent);
    }
    return ok;
  }
  bool SendAtData(int link_id, const String& data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    return SendAtData(link_id, data.c_str(), timeout_ms, silent);
  }
  bool ReceiveAtData(int link_id, String& data, long timeout_ms = DefAtTimeout, bool silent = DefSendReceiveDataSilent) {
    ReceiveAtDataInterpreter interpreter(data);
    if (SendAtCommand(String("AT+CIPRECVDATA=") + String(link_id) + String(",102400"), &interpreter, timeout_ms, silent)) {
      return data.length() > 0;
    }
    return false;
  }

  // 0: ESP32 station has not started any Wi-Fi connection.
  // 1: ESP32 station has connected to an AP, but does not get an IPv4 address yet.
  // 2: ESP32 station has connected to an AP, and got an IPv4 address.
  // 3: ESP32 station is in Wi-Fi connecting or reconnecting state.
  // 4: ESP32 station is in Wi-Fi disconnected state.
  // -1 ... not checked
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

  bool ConnectAP(const char* ssid, const char* password, String& ip) {
    bool connected = false;
    if (SendAtCommand("AT")) {
      if (SendAtCommand(String("AT+CWJAP=\"") + ssid + String("\",\"") + password + String("\""))) {
        connected = SendAtCommand("AT+CIPRECVMODE=1");
      }
    }
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
    }
    return link_id;
  }

  bool DisconnectClient(int link_id) {
    return SendAtCommand(String("+CIPCLOSE=") + String(link_id));
  }

};

