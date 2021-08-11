
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
                return False
            transDir = line[0]
            line = line[1:]
            self._sendLine(line, transDir)
            return True
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

class SimpleDDBridge(DDBridge):
    def __init__(self, ser, target):
        super().__init__()
        self.ser = ser
        self.target = target
    def _sendLine(self, line, transDir):
        if line != None:
            if transDir == '>':
                if self.target != None:
                    self.target.forward(line)
            elif transDir == '<':
                if self.ser != None:
                    data = (line + '\n').encode()
                    self.ser.write(data)

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

class Tunnel:
    def __init__(self, ser, tunnel_id,host, port):
        self.bridge = SimpleDDBridge(ser, self)
        self.tunnel_id = tunnel_id
        self.host = host
        self.port = port
        self.sock = None
        self.closed = False
    def insertSourceLine(self, source_line):
        if self.bridge != None:
            self.bridge.insertSourceLine(source_line)
    def close(self):
        self._close(None)
    def serve(self):
        # while True:
        #     try:
        #         self._serveOnce()
        #     except OSError as err:
        #         print(f"failed to 'serve' end-point {self.host}:{self.port} ... {err}")
        #         self._onError()
        #         break
        try:
            self._serveOnce()
        except OSError as err:
            print(f"failed to 'serve' end-point {self.host}:{self.port} ... {err}")
            self._close("OS error: {0}".format(err))
    def forward(self, line):
        if self.sock != None:
            data = bytes(line + "\n", 'UTF8')
            try:
                self.sock.sendall(data)
            except:
                pass
                #self._close()
    def _serveOnce(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            self.sock = s
            self.sock.connect((self.host, self.port))
            received_any = False
            rest = ""
            while True:
                idx = rest.find('\n')
                if idx != -1:
                    d = rest
                    rest = ''
                else:
                    r = self.sock.recv(1024) # blocking
                    if not r: # data is b''
                        break
                    received_any = True
                    d = r.decode('UTF8')
                idx = d.find('\n')
                if idx != -1:
                    line = rest + d[0: idx]
                    rest = d[idx + 1:]
                    if self.bridge != None:
                        #print("----:" + line)
                        self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + "<" + line)
                else:
                    rest = rest + d
            if received_any:
                if self.bridge != None:  # actually, no line-end
                    #print("FINAL:" + rest)
                    self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + ":final<" + rest)
                self._close(None)
            # while True:
            #     data = self.sock.recv(1024)
            #     if data: # data is b''
            #         received_any = True
            #         line = data.decode('UTF8').rstrip() # right strip to strip \n
            #         if self.bridge != None:
            #             for data in line.split('\n'):
            #                 #print(f"-{self.tunnel_id}:{data}")
            #                 self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + "<" + data)
            #     else:
            #         if received_any:
            #             self._close("no data")
            #             break
    def _close(self, error_msg):
        if error_msg != None and self.bridge != None:
            self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + ":error<" + error_msg)
        if self.sock != None:
            self.sock.close()
            self.sock = None
        self.closed = True
        #self.bridge = None

class SerialSource:
    def __init__(self, ser, bridge):
        self.ser = ser
        self.error = None
        self.bridge = bridge
        self.tunnels = {}
    def timeSlice(self, bridge):
        bridge.transportLine()
        tunnels = set(self.tunnels.values())
        for tunnel in tunnels:
            if not tunnel.bridge.transportLine() and tunnel.closed:
                del self.tunnels[tunnel.tunnel_id]
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
                    insert_it = True
                    if ser_line.startswith("%%>lt"):
                        insert_it = False
                        lt_line = ser_line[6:]
                        idx = lt_line.find('>')
                        if idx != -1:
                            lt_data = lt_line[idx + 1:]
                            lt_line = lt_line[0:idx]
                            idx = lt_line.find(':')
                            if idx != -1:
                                tid = lt_line[0:idx]
                                lt_command = lt_line[idx + 1:]
                            else:
                                tid = lt_line
                                lt_command = None
                        #print(tid + ':' + str(tl_command) + ">" + str(lt_data))
                        if lt_command != None:
                            if lt_command == "connect" and lt_data != None:
                                host = None
                                port = 80
                                idx = lt_data.find(':')
                                if idx != -1:
                                    port = int(lt_data[idx + 1:])
                                    host = lt_data[0:idx]
                                else:
                                    host = lt_data
                                tunnel = Tunnel(self.ser, tid, host, port)
                                self.tunnels[tunnel.tunnel_id] = tunnel
                                threading.Thread(target=tunnel.serve, daemon=True).start()
                                #self.tunnels.append(tunnel)
                            elif lt_command == 'disconnect':
                                tunnel = self.tunnels.get(tid)
                                if tunnel != None:
                                    tunnel.close()
                        else:
                            tunnel = self.tunnels.get(tid)
                            if tunnel != None:
                                tunnel.insertSourceLine(lt_data)
                    if insert_it:
                        self.bridge.insertSourceLine(ser_line)
                    ser_line = ""
                else:
                    ser_line = ser_line + c

class WifiTarget:
    def __init__(self, bridge, host, port):
        self.host = host
        self.port = port
        self.bridge = bridge
        self.sock = None
        self.conn = None
    def serve(self):
        try:
            self._serveOnce()
        except OSError as err:
            print("WiFi connect lost")
            if self.bridge != None:
                self.bridge.insertLogLine("!!!!! WiFi connection lost")
            self.stop()
        # while True:
        #     try:
        #         self._serveOnce()
        #     except OSError as err:
        #         print("WiFi connect lost")
        #         if self.bridge != None:
        #             self.bridge.insertLogLine("!!!!! WiFi connection lost")
        #         self.stop()
        #         break
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
                print("WiFi connect lost (when sending data)")
                if self.bridge != None:
                    self.bridge.insertLogLine("!!!!! WiFi connection lost (when sending data)")
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
                print("WiFi connection made")
                if self.bridge != None:
                    #self.bridge.insertLogLine('Connected by ' + str(addr))
                    self.bridge.insertLogLine("!!!!! WiFi connected by {0}".format(str(addr)))
                rest = ""
                while True:
                    idx = rest.find('\n')
                    if idx != -1:
                        d = rest
                        rest = ''
                    else:
                        r = conn.recv(1024) # blocking
                        if not r: # data is b''
                            break
                        d = r.decode('UTF8')
                    idx = d.find('\n')
                    if idx != -1:
                        line = rest + d[0: idx]
                        rest = d[idx + 1:]
                        if self.bridge != None:
                            self.bridge.insertTargetLine(line)
                    else:
                        rest = rest + d
                    # data = conn.recv(1024) # blocking
                    # if not data: # data is b''
                    #     break
                    # line = data.decode('UTF8').rstrip() # right strip to strip \n
                    # if line != '':
                    #     if self.bridge != None:
                    #         self.bridge.insertTargetLine(line)

if __name__ == "__main__":
    print("Plase run DDWifiBridge.py instead!!!")