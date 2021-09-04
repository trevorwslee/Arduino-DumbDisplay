from serial.tools.list_ports import comports

import tkinter as tk
import tkinter.ttk as ttk
import tkinter.scrolledtext as st

import ddbcore


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

def FillPortCombo():
    ports = []
    for port in comports():
        ports.append(port[0])
    Port_combo['values'] = ports
    if len(ports) > 0:
       Port_combo.current(0)

def FillBaudCombo():
    global DefBaudRate

    bauds = [2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000]
    Baud_combo['values'] = bauds
    #Baud_combo.current(7)
    Baud_combo.set(ddbcore.DefBaudRate)

def OnDisconnected():
    Port_combo["state"] = "normal"
    Baud_combo["state"] = "normal"
    WifiPort_entry["state"] = "normal"
    Text_box.insert(tk.END, "*** disconnected\n")

def InitWindow():
    global Window
    global Auto_scroll_state

    global Connect_button
    global Port_combo
    global Baud_combo
    global WifiPort_entry
    global Text_box

    Window = tk.Tk()
    Window.title("DumbDispaly WIFI Bridge")
    Window.geometry("800x600")
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
    auto_scroll_check.pack(side=tk.LEFT)
    clear_button.pack(side=tk.LEFT)
    tool_bar.pack()
    Text_box = st.ScrolledText(width=100, height=1000)
    Text_box.pack(fill=tk.BOTH)
    FillPortCombo()
    FillBaudCombo()
    WifiPort_entry.insert(0, str(ddbcore.DefWifiPort))

# def Initialize():
#     Window = tk.Tk()
#     Window.title("DumbDispaly WIFI Bridge")
#     Window.geometry("800x600")
#     Auto_scroll_state = tk.BooleanVar()
#     Auto_scroll_state.set(True)


def RunDDBridgeWinMain():
    ddui = DDWinUserInterface()
    ddbcore.RunDDBridgeMain(ddui)

class DDWinUserInterface(ddbcore.DDUserInterface):
    def initialize(self):
        InitWindow()
    def syncConnectionState(self, connected):
        Connect_button.config(text="Disconnect" if connected else "Connect")
    def onConnected(self):
        Port_combo["state"] = "disabled"
        Baud_combo["state"] = "disabled"
        WifiPort_entry["state"] = "disabled"
        #Text_box.insert(tk.END, "*** connected\n")
    def onDisconnected(self):
        Port_combo["state"] = "normal"
        Baud_combo["state"] = "normal"
        WifiPort_entry["state"] = "normal"
        #Text_box.insert(tk.END, "*** disconnected\n")
    def timeSlice(self):
        Window.update()
    def bridge_send(self, transDir, line):
        # global Window
        # global Auto_scroll_state
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
        Text_box.insert(tk.END, transDir + ' ' + line + '\n')
    def printLogMessage(self, msg):
        print(msg)
    def printControlMessage(self, msg):
        Text_box.insert(tk.END, msg + "\n")
        pass

if __name__ == "__main__":
    print("Please run DDWifiBridge.py instead!!!")
