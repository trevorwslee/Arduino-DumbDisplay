import serial
import socket

HOST = '' # empty ==> all accepted
PORT = 10201        # Port to listen on (non-privileged ports are > 1023)


class DDBridge:
    def __init__(self):
        self.line_list = []
    def insertSourceLine(self, source_line):
        self._insertLine('>', source_line)
    def insertTargetLine(self, target_line):
        self._insertLine('<', target_line)
    def insertLogLine(self, log_line):
        self._insertLine('=', log_line)
    def _insertLine(self, transDir, line):
        self.line_list.append(transDir + line)
        #self._transportLine()
    def transportLine(self):
        while True:
            if len(self.line_list) == 0:
                break
            line = self.line_list.pop()
            transDir = line[0]
            line = line[1:]
            self._sendLine(line, transDir)
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
        except (serial.SerialException, AttributeError) as err:
            self.ser = None
            self.error = err
    def _serialServe(self):
        ser_line = ""
        while True:
            for b in self.ser.read():
                c = chr(b)
                if c == '\n':
                    self.bridge.insertSourceLine(ser_line)
                    ser_line = ""
                else:
                    ser_line = ser_line + c


class WifiTarget:
    def __init__(self, bridge):
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
            self.sock = s
            self.sock.bind((HOST, PORT))
            if self.bridge != None:
                self.bridge.insertLogLine("!!!!! For WiFi, listening on " + get_ip() + ':' + str(PORT))
            self.sock.listen()
            conn, addr = self.sock.accept() # block and wait
            self.conn = conn
            with conn:
                if self.bridge != None:
                    self.bridge.insertLogLine('Connected by ' + str(addr))
                while True:
                    data = conn.recv(1024) # blocking
                    if not data: # data is b''
                        break
                    line = data.decode('UTF8').rstrip()
                    if line != '':
                        if self.bridge != None:
                            self.bridge.insertTargetLine(line)

if __name__ == "__main__":
    print("Plase run DDWifiBridge.py instead!!!")