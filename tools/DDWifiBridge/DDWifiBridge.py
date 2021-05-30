from serial.tools.list_ports import comports

import serial
import tkinter as tk
import tkinter.ttk as ttk
import tkinter.scrolledtext as st
import threading

import ddbmod


class DDBridge(ddbmod.DDBridge):
    def _sendLine(self, line, transDir):
        if line != None:
            if Auto_scroll_state.get():
                Text_box.see(tk.END)
                if True:
                    pos = Text_box.index(tk.INSERT)
                    end_pos = Text_box.index(tk.END)
                    line_count = int(end_pos.split('.')[0]) - 1
                    check_pos = str(line_count) + '.0'
                    if pos != check_pos:
                        Auto_scroll_state.set(False)
            Window.update()
            Text_box.insert(tk.END, transDir + ' ' + line + '\n')
            if transDir == '>':
                if Wifi != None:
                    Wifi.forward(line)
            elif transDir == '<':
                if Ser != None:
                    data = (line + '\n').encode()
                    Ser.write(data)


Ser = None
Serial = None
Bridge = None
Wifi = None

Window = tk.Tk()
Window.title("DumbDispaly WIFI Bridge")
Window.geometry("800x600")

Auto_scroll_state = tk.BooleanVar()
Auto_scroll_state.set(True)


def Connect():
    port = Port_combo.get()
    baud = Baud_combo.get()
    print("Connect to", port)
    if port != "":
        ser = serial.Serial(port=port,baudrate=baud,
                            parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE,bytesize=serial.EIGHTBITS,
                            timeout=0)
        Text_box.insert(tk.END, "*** connected to: " + ser.portstr + "\n")
        return ser
    else:
        return None

def Disconnect(ser):
    ser.close()


def ClickedConnect():
    global Ser
    global Bridge
    global Serial
    global Wifi
    if Ser == None:
        try:
            Ser = Connect()
            if Ser != None:
                Port_combo["state"] = "disabled"
                Baud_combo["state"] = "disabled"
                Bridge = DDBridge()
                Serial = ddbmod.SerialSource(Ser, Bridge)
                Wifi = ddbmod.WifiTarget(Bridge)
                threading.Thread(target=SerialServe, daemon=True).start()
                threading.Thread(target=WifiServe, daemon=True).start()
        except serial.SerialException as err:
            Text_box.insert(tk.END, "*** serial exception while connecting -- {0}\n".format(err))
    else:
        Disconnect(Ser)

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
    bauds = [2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000]
    Baud_combo['values'] = bauds
    Baud_combo.current(7)

def InitWindow():
    global Connect_button
    global Port_combo
    global Baud_combo
    global Text_box

    tool_bar = tk.Frame()
    Connect_button = tk.Button(tool_bar, command=ClickedConnect)
    clear_button = tk.Button(tool_bar, text='Clear', command=ClickedClear)
    Port_combo = ttk.Combobox(tool_bar, postcommand=FillPortCombo)
    Baud_combo = ttk.Combobox(tool_bar)
    auto_scroll_check = tk.Checkbutton(tool_bar, text='Auto Scroll', var=Auto_scroll_state,
                                       command=lambda: Text_box.mark_set("insert", tk.END))
    Connect_button.pack(side=tk.LEFT)
    Port_combo.pack(side=tk.LEFT)
    Baud_combo.pack(side=tk.LEFT)
    auto_scroll_check.pack(side=tk.LEFT)
    clear_button.pack(side=tk.LEFT)
    tool_bar.pack()
    Text_box = st.ScrolledText(width=100, height=1000)
    Text_box.pack(fill=tk.BOTH)
    FillPortCombo()
    FillBaudCombo()

def SerialLoop():
    while True:
        Window.update()
        if Serial.error != None:
            raise Serial.error
        Bridge.transportLine()

def NoSerialLoop():
    while True:
        Window.update()
        if Ser != None:
            return


def SerialServe():
    if Serial != None:
        Serial.serialServe()
def WifiServe():
    if Wifi != None:
        Wifi.serve()

if __name__ == "__main__":
    InitWindow()
    while True:
        if Ser != None:
            Connect_button.config(text="Disconnect")
            try:
                SerialLoop()
            except Exception as err:
                Text_box.insert(tk.END, "*** exception -- {0}\n".format(err))
            Ser.close()
            Ser = None
            Bridge = None
            if Serial != None:
                Serial = None
            if Wifi != None:
                Wifi.stop()
                Wifi = None
            Port_combo["state"] = "normal"
            Baud_combo["state"] = "normal"
            Text_box.insert(tk.END, "*** disconnected\n")
        else:
            Connect_button.config(text="Connect")
            NoSerialLoop()
else:
    print("This is not a module!")

