from serial.tools.list_ports import comports

import tkinter as tk
import tkinter.ttk as ttk
import tkinter.scrolledtext as st

#import ddbcore
from . import ddbcore

# def Connect(port, baud):
#     print("Connect to", port, "with baud rate", baud)
#     if port != "":
#         ser = pyserial.Serial(port=port,baudrate=baud,
#                               parity=pyserial.PARITY_NONE,stopbits=pyserial.STOPBITS_ONE,bytesize=pyserial.EIGHTBITS,
#                               timeout=0)
#         Text_box.insert(tk.END, "*** connected to: " + ser.portstr + "\n")
#         return ser
#     else:
#         return None
#
# def Disconnect(ser):
#     ser.close()
#     print("Disconnected")

WindowRunning = True

def OnWindowClosed():
    global WindowRunning

    print("DDWifiBridgeWindow closed!")
    WindowRunning = False
    Window.destroy()

def ClickedConnect():
    port = Port_combo.get()
    baud = Baud_combo.get()
    try:
        wifiPort = int(WifiPort_entry.get())
    except:
        wifiPort = ddbcore.DefWifiPort
    ddbcore.InvokeConnect(port, baud, wifiPort)

def ClickedClear():
    Text_box.delete('1.0', tk.END)

def FillPortCombo(port = None):
    ports = []
    if port != None:
        ports.append(port)
    for comps in comports():
        ports.append(comps[0])
    Port_combo['values'] = ports
    if len(ports) > 0:
        if port != None:
            Port_combo.set(port)
        else:
            Port_combo.current(0)


def FillBaudCombo(baud: None):
    global DefBaudRate

    bauds = [2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000]
    Baud_combo['values'] = bauds
    #Baud_combo.current(7)
    Baud_combo.set(baud if baud != None else ddbcore.DefBaudRate)

def OnDisconnected():
    Port_combo["state"] = "normal"
    Baud_combo["state"] = "normal"
    WifiPort_entry["state"] = "normal"
    Text_box.insert(tk.END, "*** disconnected\n")

def InitWindow(param_dict: None):
    global Window
    global Show_state
    global Auto_scroll_state

    global Connect_button
    global Port_combo
    global Baud_combo
    global WifiPort_entry
    global Text_box

    port = None
    baud = None
    wifi_port = None
    if param_dict != None:
        port = param_dict["port"]
        baud = param_dict.get("baud")
        wifi_port = param_dict.get("wifiPort")

    Window = tk.Tk()
    Window.protocol("WM_DELETE_WINDOW", OnWindowClosed)
    Window.title("DumbDispaly WIFI Bridge")
    Window.geometry("800x600")
    Show_state = tk.BooleanVar()
    Show_state.set(True)
    Auto_scroll_state = tk.BooleanVar()
    Auto_scroll_state.set(True)

    tool_bar = tk.Frame()
    Connect_button = tk.Button(tool_bar, command=ClickedConnect)
    Port_combo = ttk.Combobox(tool_bar, postcommand=FillPortCombo)
    Baud_combo = ttk.Combobox(tool_bar)
    direction_label = tk.Label(tool_bar, text=' <==> ')
    wifiHost_label = tk.Label(tool_bar, text=ddbcore.WifiHost+':')
    WifiPort_entry = tk.Entry(tool_bar, width=6)
    spacer_label = tk.Label(tool_bar, text='  |  ')
    show_check = tk.Checkbutton(tool_bar, text='Show', var=Show_state)
    #clear_button = tk.Button(tool_bar, text='Clear', command=ClickedClear)
    auto_scroll_check = tk.Checkbutton(tool_bar, text='Auto Scroll', var=Auto_scroll_state,
                                       command=lambda: Text_box.mark_set("insert", tk.END))
    clear_button = tk.Button(tool_bar, text='Clear', command=ClickedClear)
    Connect_button.pack(side=tk.LEFT)
    Port_combo.pack(side=tk.LEFT)
    Baud_combo.pack(side=tk.LEFT)
    direction_label.pack(side=tk.LEFT)
    wifiHost_label.pack(side=tk.LEFT)
    WifiPort_entry.pack(side=tk.LEFT)
    spacer_label.pack(side=tk.LEFT)
    show_check.pack(side=tk.LEFT)
    auto_scroll_check.pack(side=tk.LEFT)
    clear_button.pack(side=tk.LEFT)
    tool_bar.pack()
    Text_box = st.ScrolledText(width=100, height=1000)
    Text_box.pack(fill=tk.BOTH)
    FillPortCombo(port)
    FillBaudCombo(baud)
    WifiPort_entry.insert(0, str(wifi_port if wifi_port != None else ddbcore.DefWifiPort))

# def Initialize():
#     Window = tk.Tk()
#     Window.title("DumbDispaly WIFI Bridge")
#     Window.geometry("800x600")
#     Auto_scroll_state = tk.BooleanVar()
#     Auto_scroll_state.set(True)


def RunDDBridgeWinMain(param_dict = None):
    ddui = DDWinUserInterface(param_dict)
    ddbcore.RunDDBridgeMain(ddui)

class DDWinUserInterface(ddbcore.DDUserInterface):
    def __init__(self, param_dict = None):
        self.param_dict = param_dict
    def initialize(self):
        InitWindow(self.param_dict)
    def syncConnectionState(self, connected):
        Connect_button.config(text="Disconnect" if connected else "Connect", fg="white" if connected else "green", bg="gray" if connected else "lightgrey")
    def onConnected(self):
        Port_combo["state"] = "disabled"
        Baud_combo["state"] = "disabled"
        WifiPort_entry["state"] = "disabled"
        #Text_box.insert(tk.END, "*** connected\n")
    def onDisconnected(self):
        try:
            Port_combo["state"] = "normal"
            Baud_combo["state"] = "normal"
            WifiPort_entry["state"] = "normal"
            #Text_box.insert(tk.END, "*** disconnected\n")
        except:
            pass
    def isUIRunning(self):
        return WindowRunning    
    def timeSlice(self):
        Window.update()
    def bridge_send(self, transDir, line):
        # global Window
        # global Auto_scroll_state
        if Show_state.get():
            if Auto_scroll_state.get():
                Text_box.see(tk.END)
                if True:
                    pos = Text_box.index(tk.INSERT)
                    end_pos = Text_box.index(tk.END)
                    line_count = int(end_pos.split('.')[0]) - 1
                    check_pos = str(line_count) + '.0'
                    if pos != check_pos:
                        Auto_scroll_state.set(False)
            #Window.update()
            if isinstance(line, bytes):
                Text_box.insert(tk.END, '......\n')
            else:
                Text_box.insert(tk.END, transDir + ' ' + line + '\n')
    def printLogMessage(self, msg):
        print(msg)
    def printControlMessage(self, msg):
        Text_box.insert(tk.END, msg + "\n")
        pass

# if __name__ == "__main__":
#     print("Please run DDWifiBridge.py instead!!!")
