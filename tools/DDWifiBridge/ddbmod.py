
import socket
import threading
import time

from jsonparse import JsonStreamParserCore



_LOG_TUNNEL = True
_LOG_TUNNEL_IO = True
_LOG_TUNNEL_JSON = True

_DEBUG_TUNNEL = False
_DEBUG_TUNNEL_INS = False

class DDBridge:
    def __init__(self):
        self.lock = threading.Lock()
        self.line_list = []
        self.no_further_insert = False
    def insertSourceLine(self, source_line):
        self._insertLine('>', source_line)
    def insertTargetLine(self, target_line, no_further_insert = False):
        self._insertLine('<', target_line, no_further_insert)
    def insertLogLine(self, log_line):
        self._insertLine('=', log_line)
    def count(self):
        return self._count();
    def transportLine(self, pushback_if_failed = False):
        while True:
            ori_line = self._popLine()
            line = ori_line
            if line == None:
                return False
            transDir = line[0]
            line = line[1:]
            if _DEBUG_TUNNEL and line.startswith('<lt.'):
                print("** TUNNEL-TX:" + line)
            if not self._sendLine(line, transDir):
                if pushback_if_failed:
                    self._pushbackLine(ori_line)
                return False
            else:
                return True
    def _count(self):
        self.lock.acquire()
        res = len(self.line_list)
        self.lock.release()
        return res
    def _insertLine(self, transDir, line, no_further_insert = False):
        self.lock.acquire()
        if not self.no_further_insert:
            if no_further_insert:
                self.no_further_insert = True
                self.line_list = []
            self.line_list.append(transDir + line)
            if _DEBUG_TUNNEL_INS and line.startswith('<lt.'):
                print("** TUNNEL-INS-" + str(len(self.line_list)) + ":" + line)
        self.lock.release()
    def _popLine(self):
        line = None
        self.lock.acquire()
        if len(self.line_list) > 0:
            line = self.line_list.pop(0)
        self.lock.release()    
        return line
    def _pushbackLine(self, line):
        self.lock.acquire()
        if not self.no_further_insert:
            self.line_list.insert(0, line)
        self.lock.release()
    def _sendLine(self, line, transDir):
        raise Exception("should have been overridden")

class SimpleDDBridge(DDBridge):
    def __init__(self, ser, target):
        super().__init__()
        self.ser = ser
        self.target = target
    def _sendLine(self, line, transDir):
        sent = False
        if line != None:
            if transDir == '>':
                if self.target != None:
                    sent = self.target.forward(line)
            elif transDir == '<':
                if self.ser != None:
                    data = (line + '\n').encode()
                    self.ser.write(data)
                    sent = True
            else:
                sent = True
        return sent

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
    def __init__(self, ser, tunnel_id, end_point):
        protocol = None
        host = None
        if end_point.startswith('http://'):
            protocol = "http"
            host = end_point[7:]
        else:
            host = end_point
        port = None
        idx = host.find(':')
        if idx != -1:
            port_str = host[idx + 1:]
            host = host[0:idx]
            try:
                port = int(port_str)
            except ValueError:
                port = None
        location = None
        idx = host.find('/')
        if idx != -1:
            location = host[idx:]
            host = host[0:idx]
        else:
            location = "/"
        if port == None:
            if protocol == 'http':
                port = 80
            else:
                raise Exception("no port specification for connection to " + end_point)
        self.bridge = SimpleDDBridge(ser, self)
        self.tunnel_id = tunnel_id
        self.protocol = protocol
        self.host = host
        self.port = port
        self.location = location
        self.sock = None
        self.closed = False
    def insertSourceLine(self, source_line):
        if self.bridge != None:
            self.bridge.insertSourceLine(source_line)
    def close(self):
        self._close(None)
    def serve(self):
        try:
            self._serveOnce()
        except BaseException as err:
            print(f"failed to 'serve' end-point {self.host}:{self.port} ... {err}")
            self._close("OS error: {0}".format(err))
    def forward(self, line):
        sent = False
        if True:
            if self.sock != None:
                try:
                    self.__send(line)
                    sent = True
                except:
                    pass
        else:
            if self.sock != None:
                if self.log_io:
                    print(">>>>> :" + line)
                data = bytes(line + "\n", 'UTF8')
                try:
                    self.sock.sendall(data)
                    sent = True
                except:
                    pass
        return sent
    def __send(self, line):
        if _LOG_TUNNEL_IO:
            print(">>>>> :" + line)
        data = bytes(line + "\n", 'UTF8')
        self.sock.sendall(data)
    def _serveOnce(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            self.sock = s
            try:
                self.sock.connect((self.host, self.port))
            except BaseException as err:
                print(f"failed to connect end-point {self.host}:{self.port} ... {err}")
                self._close("OS error: {0}".format(err))
                return
            if self.sock != None and self.protocol == 'http':
                self.__send("GET " + self.location + " HTTP/1.1")
                #self.__send("Accept: application/json, text/plain, */*")
                self.__send("Connection: close")
                self.__send("")
                # line = "GET " + self.location + " HTTP/1.1\n\n"
                # data = bytes(line, 'UTF8')
                # self.sock.sendall(data)
                # if self.log_io:
                #     print("> HEADER > :" + line)
            received_any = False
            rest = ""
            while self.sock != None:
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
                        if _LOG_TUNNEL_IO:
                            print("<<<<< :" + line)
                        # if self.log_io:
                        #     print("<<<<< :" + line)
                        self._handleReceivedTargetLine(line)
                        #self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + "<" + line)
                else:
                    rest = rest + d
            if received_any:
                if self.bridge != None:  # actually, no line-end
                    if _LOG_TUNNEL_IO:
                        print("<<<<| :" + rest)
                    #print("FINAL:" + rest)
                    self._handleReceivedTargetLine(rest, True)
                    #self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + ":final<" + rest)
                self._close(None)
    def _close(self, error_msg):
        if error_msg != None and self.bridge != None:
            self.bridge.insertTargetLine("<lt." + str(self.tunnel_id) + ":error<" + error_msg, True)
        if self.sock != None:
            self.sock.close()
            self.sock = None
        self.closed = True
    def _handleReceivedTargetLine(self, line, final = False):
        self._insertTargetLine(line, final)
    def _insertTargetLine(self, line, final = False):
        id = str(self.tunnel_id)
        if final:
            id = id + ":final"
        self.bridge.insertTargetLine("<lt." + id + "<" + line)

class SimpleJsonTunnel(Tunnel):
    def __init__(self, ser, tunnel_id, end_point):
        super().__init__(ser, tunnel_id, end_point)
        self.parser = JsonStreamParserCore()
        self.parser.onReceived = lambda field_id, field_value: self._onReceived(field_id, field_value)
    def _handleReceivedTargetLine(self, line, final = False):
        self.parser.sinkJsonData(line)
        if self.parser.finalized:
            if _LOG_TUNNEL_JSON:
                print("<{}<| :" + line)
            self._insertTargetLine("", True)
    def _onReceived(self, field_id, field_value):
        line = field_id + ":" + field_value
        if _LOG_TUNNEL_JSON:
            print("<{}<< :" + line)
        self._insertTargetLine(line)



class SerialSource:
    def __init__(self, ser, bridge):
        self.ser = ser
        self.error = None
        self.bridge = bridge
        self.tunnels = {}
    def timeSlice(self, bridge):
        bridge.transportLine()
        if len(self.tunnels) > 0:
            tunnels = set(self.tunnels.values())
            for tunnel in tunnels:
                tx_closed = tunnel.closed
                if _DEBUG_TUNNEL and tx_closed:
                    peek = tunnel.bridge.count()
                    if peek > 1:
                        print("... ??? " + str(peek) + " ???...")
                    else:
                        print("...")
                tx_res = tunnel.bridge.transportLine(True)
                if tx_closed and (not tx_res):
                    del self.tunnels[tunnel.tunnel_id]
                    if _LOG_TUNNEL:
                        print('Released tunnel ' + tunnel.tunnel_id)
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
                            if lt_command == "connect" or lt_command == "reconnect":
                                # host = None
                                # port = None
                                # if lt_data != None:
                                #     idx = lt_data.find(':')
                                #     if idx != -1:
                                #         port = int(lt_data[idx + 1:])
                                #         host = lt_data[0:idx]
                                #     else:
                                #         host = lt_data
                                # if host != None:
                                if lt_data != None:
                                    type = None
                                    end_point = None
                                    if lt_data != None:
                                        idx = lt_data.find('@')
                                        if idx != -1:
                                            type = lt_data[0:idx]
                                            end_point = lt_data[idx + 1:]
                                        else:
                                            end_point = lt_data
                                    if lt_command == "reconnect":
                                        tunnel = self.tunnels.get(tid)
                                        if tunnel != None:
                                            tunnel.close()
                                            while self.tunnels.get(tid) != None:
                                                time.sleep(0.1)
                                    if type == 'ddjson':
                                        tunnel = SimpleJsonTunnel(self.ser, tid, end_point)
                                    else:
                                        tunnel = Tunnel(self.ser, tid, end_point)
                                    self.tunnels[tunnel.tunnel_id] = tunnel
                                    if _LOG_TUNNEL:
                                        log_prefix = 'Re-' if lt_command == "reconnect" else ''
                                        print(log_prefix + 'Create tunnel ' + tunnel.tunnel_id)
                                    threading.Thread(target=tunnel.serve, daemon=True).start()
                                #self.tunnels.append(tunnel)
                            elif lt_command == 'disconnect':
                                tunnel = self.tunnels.get(tid)
                                if _LOG_TUNNEL:
                                    print('Disconnected tunnel ' + tid)
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
    def stop(self):
        if self.conn != None:
            self.conn.close()
            self.conn = None
        if self.sock != None:
            self.sock.close()
            self.sock = None
        self.bridge = None
    def forward(self, line):
        sent = False
        if self.conn != None:
            data = bytes(line + "\n", 'UTF8')
            try:
                self.conn.sendall(data)
                sent = True
            except:
                print("WiFi connect lost (when sending data)")
                if self.bridge != None:
                    self.bridge.insertLogLine("!!!!! WiFi connection lost (when sending data)")
                self.stop()
        return sent
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

if __name__ == "__main__":
    print("Plase run DDWifiBridge.py instead!!!")