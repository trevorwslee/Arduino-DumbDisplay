#from serial.tools.list_ports import comports

import traceback


import serial as pyserial
import threading

import configparser
import os

import ddbmod


class DDUserInterface:
    def initialize(self):
        pass
    def syncConnectionState(self, connected):
        pass
    def onSerialException(self, err):
        pass
    def onConnected(self):
        pass
    def onDisconnected(self):
        pass
    def timeSlice(self):
        pass
    def bridge_send(self, transDir, line):
        pass
    def printLogMessage(self, msg):
        pass
    def printControlMessage(self, msg):
        '''Text_box.insert(tk.END, transDir + ' ' + line + '\n')'''
        pass


WifiHost = None
DefWifiPort = None
DefBaudRate = None
DdUI = None

def _Initialize(ddui):
    global DdUI
    global Ser
    global Bridge
    global Serial
    global Wifi
    global Config
    global ConfigFileName
    global ConfigSectionName
    global BaudRateConfigName
    global WiFiPortConfigName
    global WifiHost
    global DefWifiPort
    global DefBaudRate
    #global Auto_scroll_state
    #global Window

    DdUI = ddui

    ConfigFileName = os.path.join(os.path.dirname(__file__), 'ddwifibridge.ini')
    ConfigSectionName = 'DEFAULT'
    BaudRateConfigName = 'BaudRate'
    WiFiPortConfigName = 'WiFiPort'
    Config = configparser.ConfigParser()
    Config.read(ConfigFileName)
    DefBaudRate = Config[ConfigSectionName].get(BaudRateConfigName, fallback=115200)
    DefWifiPort = Config[ConfigSectionName].get(WiFiPortConfigName, fallback=10201)

    WifiHost = ddbmod.get_ip()

    Ser = None
    Serial = None
    Bridge = None
    Wifi = None

    # Window = tk.Tk()
    # Window.title("DumbDispaly WIFI Bridge")
    # Window.geometry("800x600")

    # Auto_scroll_state = tk.BooleanVar()
    # Auto_scroll_state.set(True)


class DDBridge(ddbmod.DDBridge):
    def _sendLine(self, line, transDir):
        #global DdUI
        #global Auto_scroll_state
        #global Window
        sent = False
        if line != None:
            #DdUI.bridge_send(transDir + ' ' + line + '\n')
            DdUI.bridge_send(transDir, line)
            # if Auto_scroll_state.get():
            #     Text_box.see(tk.END)
            #     if True:
            #         pos = Text_box.index(tk.INSERT)
            #         end_pos = Text_box.index(tk.END)
            #         line_count = int(end_pos.split('.')[0]) - 1
            #         check_pos = str(line_count) + '.0'
            #         if pos != check_pos:
            #             Auto_scroll_state.set(False)
            # Window.update()
            # Text_box.insert(tk.END, transDir + ' ' + line + '\n')
            if transDir == '>':
                if Wifi != None:
                    sent = Wifi.forward(line)
            elif transDir == '<':
                if Ser != None:
                    data = (line + '\n').encode()
                    Ser.write(data)
                    sent = True
            else:
                sent = True
        return sent


def _Connect(port, baud):
    #global DdUI
    DdUI.printLogMessage("Connect to {0} with baud rate {1}".format(port, baud))
    if port != "":
        ser = pyserial.Serial(port=port,baudrate=baud,
                              parity=pyserial.PARITY_NONE,stopbits=pyserial.STOPBITS_ONE,bytesize=pyserial.EIGHTBITS,
                              timeout=0)
        DdUI.printControlMessage("*** connected to: " + ser.portstr)
        #Text_box.insert(tk.END, "*** connected to: " + ser.portstr + "\n")
        return ser
    else:
        return None

def _Disconnect():
    global Ser
    #global DdUI
    Ser.close()
    Ser = None
    DdUI.printLogMessage("Disconnected")


def InvokeConnect(port, baud, wifiPort):
    global Ser
    global Bridge
    global Serial
    global Wifi
    global Config
    global ConfigFileName
    global DefWifiPort
    global ConfigSectionName
    global BaudRateConfigName
    global WiFiPortConfigName

    if Ser == None:
        # port = Port_combo.get()
        # baud = Baud_combo.get()
        # try:
        #     wifiPort = int(WifiPort_entry.get())
        # except:
        #     wifiPort = DefWifiPort
        Config[ConfigSectionName][BaudRateConfigName] = str(baud)
        Config[ConfigSectionName][WiFiPortConfigName] = str(wifiPort)
        with open(ConfigFileName, 'w') as configfile:
            Config.write(configfile)
        try:
            Ser = _Connect(port, baud)
            if Ser != None:
                # Port_combo["state"] = "disabled"
                # Baud_combo["state"] = "disabled"
                # WifiPort_entry["state"] = "disabled"
                Bridge = DDBridge()
                Serial = ddbmod.SerialSource(Ser, Bridge)
                Wifi = ddbmod.WifiTarget(Bridge, WifiHost, wifiPort)
                #WifiHost_label.configure(text=WifiHost+ ':')
                DdUI.onConnected()
                threading.Thread(target=SerialServe, daemon=True).start()
                threading.Thread(target=WifiServe, daemon=True).start()
        except pyserial.SerialException as err:
            DdUI.printControlMessage("*** serial exception while connecting -- {0}".format(err))
            DdUI.onSerialException(err)
           #Text_box.insert(tk.END, "*** serial exception while connecting -- {0}\n".format(err))
    else:
        _Disconnect()

# def ClickedClear():
#     Text_box.delete('1.0', tk.END)
#
# def FillPortCombo():
#     ports = []
#     for port in comports():
#         ports.append(port[0])
#     Port_combo['values'] = ports
#     if len(ports) > 0:
#        Port_combo.current(0)
#
# def FillBaudCombo():
#     global DefBaudRate
#
#     bauds = [2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000]
#     Baud_combo['values'] = bauds
#     #Baud_combo.current(7)
#     Baud_combo.set(DefBaudRate)
#
# def InitWindow():
#     global Connect_button
#     global Port_combo
#     global Baud_combo
#     global WifiPort_entry
#     global Text_box
#
#     tool_bar = tk.Frame()
#     Connect_button = tk.Button(tool_bar, command=ClickedConnect)
#     Port_combo = ttk.Combobox(tool_bar, postcommand=FillPortCombo)
#     Baud_combo = ttk.Combobox(tool_bar)
#     direction_label = tk.Label(tool_bar, text=' <==> ')
#     wifiHost_label = tk.Label(tool_bar, text=WifiHost+':')
#     WifiPort_entry = tk.Entry(tool_bar, width=6)
#     spacer_label = tk.Label(tool_bar, text='  |  ')
#     auto_scroll_check = tk.Checkbutton(tool_bar, text='Auto Scroll', var=Auto_scroll_state,
#                                        command=lambda: Text_box.mark_set("insert", tk.END))
#     clear_button = tk.Button(tool_bar, text='Clear', command=ClickedClear)
#     Connect_button.pack(side=tk.LEFT)
#     Port_combo.pack(side=tk.LEFT)
#     Baud_combo.pack(side=tk.LEFT)
#     direction_label.pack(side=tk.LEFT)
#     wifiHost_label.pack(side=tk.LEFT)
#     WifiPort_entry.pack(side=tk.LEFT)
#     spacer_label.pack(side=tk.LEFT)
#     auto_scroll_check.pack(side=tk.LEFT)
#     clear_button.pack(side=tk.LEFT)
#     tool_bar.pack()
#     Text_box = st.ScrolledText(width=100, height=1000)
#     Text_box.pack(fill=tk.BOTH)
#     FillPortCombo()
#     FillBaudCombo()
#     WifiPort_entry.insert(0, str(DefWifiPort))

def SerialLoop():
    #global DdUI
    while True:
        DdUI.timeSlice()
        #Window.update()
        if Serial.error != None:
            if Ser == None:
                break
            raise Serial.error
        if Serial.bridge == None or Wifi.bridge == None:
            _Disconnect()
            return
        Serial.timeSlice(Bridge)
        #Bridge.transportLine()


def NoSerialLoop():
    #global DdUI
    while True:
        DdUI.timeSlice()
        #Window.update()
        if Ser != None:
            return


def SerialServe():
    if Serial != None:
        Serial.serialServe()
def WifiServe():
    if Wifi != None:
        Wifi.serve()

def RunDDBridgeMain(ddui, param_dict = None):
    global Ser
    global Serial
    global Wifi
    global Bridge

    _Initialize(ddui)
    ddui.initialize()
    #InitWindow()
    if param_dict != None:
        port = param_dict["port"]
        baud = param_dict.get("baud")
        wifiPort = param_dict.get("wifiPort")
        if baud == None:
            baud = int(DefBaudRate)
        if wifiPort == None:
            wifiPort = int(DefWifiPort)
        InvokeConnect(port, baud, wifiPort)
    while True:
        ddui.syncConnectionState(Ser != None)
        if Ser != None:
            #Connect_button.config(text="Disconnect")
            try:
                SerialLoop()
            except Exception as err:
                ddui.printControlMessage("*** exception -- {0}".format(err))
                #Text_box.insert(tk.END, "*** exception -- {0}\n".format(err))
                traceback.print_exc()
                Ser.close()
            Ser = None
            Bridge = None
            if Serial != None:
                Serial = None
            if Wifi != None:
                Wifi.stop()
                Wifi = None
            ddui.onDisconnected()
            ddui.printControlMessage("*** disconnected")
            #Port_combo["state"] = "normal"
            #Baud_combo["state"] = "normal"
            #WifiPort_entry["state"] = "normal"
            #Text_box.insert(tk.END, "*** disconnected\n")
        else:
            #Connect_button.config(text="Connect")
            NoSerialLoop()

if __name__ == "__main__":
    print("Please run DDWifiBridge.py instead!!!")
