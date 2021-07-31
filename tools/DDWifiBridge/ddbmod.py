
import socket
import threading

class DDBridge:
    def __init__(self):
        self.lock = threading.Lock()
        self.line_list = []
    def insertSourceLine(self, source_line):
        self._insertLine('>', source_line)
    def insertTargetLine(self, target_line):
        self._insertLine('<', target_line)
    def insertLogLine(self, log_line):
        self._insertLine('=', log_line)
    def transportLine(self):
        while True:
            line = self._popLine()
            if line == None:
                break
            transDir = line[0]
            line = line[1:]
            self._sendLine(line, transDir)
    def _insertLine(self, transDir, line):
        self.lock.acquire()
        self.line_list.append(transDir + line)
        self.lock.release()
    def _popLine(self):
        line = None
        self.lock.acquire()
        if len(self.line_list) > 0:
            line = self.line_list.pop()
        self.lock.release()    
        return line    
    def _sendLine(self, line, transDir):
        raise Exception("should have been overridden")


def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

class SerialSource:
    def __init__(self, ser, bridge):
        self.ser = ser
        self.error = None
        self.bridge = bridge
    def serialServe(self):
        try:
            self._serialServe()
        #except (serial.SerialException, Exception) as err:
        except Exception as err:
            self.ser = None
            self.error = err
    def _serialServe(self):
        ser_line = ""
        while True:
            # c = self.ser.read().decode()
            # if c == '\n':
            #     self.bridge.insertSourceLine(ser_line)
            #     ser_line = ""
            # else:
            #     ser_line = ser_line + c
            for b in self.ser.read():
                c = chr(b)
                if c == '\n':
                    self.bridge.insertSourceLine(ser_line)
                    ser_line = ""
                else:
                    ser_line = ser_line + c

class WifiTarget:
    def __init__(self, bridge, host, port):
        #self.ip = ip#get_ip()
        self.host = host
        self.port = port
        self.bridge = bridge
        self.sock = None
        self.conn = None
    def serve(self):
        while True:
            try:
                self._serveOnce()
            except OSError as err:
                if self.bridge != None:
                    self.bridge.insertLogLine("!!!!! WiFi connection lost")
                self.stop()
                break
    def stop(self):
        if self.conn != None:
            self.conn.close()
            self.conn = None
        if self.sock != None:
            self.sock.close()
            self.sock = None
        self.bridge = None
    def forward(self, line):
        if self.conn != None:
            data = bytes(line + "\n", 'UTF8')
            try:
                self.conn.sendall(data)
            except:
                if self.bridge != None:
                    self.bridge.insertLogLine("!!!!! WiFi connection lost")
                self.stop()
    def _serveOnce(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            #host = '' # empty ==> all accepted
            host = self.host
            port = self.port        # Port to listen on (non-privileged ports are > 1023)
            self.sock = s
            self.sock.bind((host, port))
            if self.bridge != None:
                #self.bridge.insertLogLine("!!!!! For WiFi, listening on " + host + ':' + str(port))
                self.bridge.insertLogLine("!!!!! For WiFi, listening on {0}:{1}".format(host, str(port)))
            self.sock.listen()
            conn, addr = self.sock.accept() # block and wait
            self.conn = conn
            with conn:
                if self.bridge != None:
                    #self.bridge.insertLogLine('Connected by ' + str(addr))
                    self.bridge.insertLogLine("!!!!! WiFi connected by {0}".format(str(addr)))
                while True:
                    data = conn.recv(1024) # blocking
                    if not data: # data is b''
                        break
                    line = data.decode('UTF8').rstrip() # right strip to strip \n
                    if line != '':
                        if self.bridge != None:
                            self.bridge.insertTargetLine(line)

if __name__ == "__main__":
    print("Plase run DDWifiBridge.py instead!!!")