#from serial.tools.list_ports import comports

import traceback


import serial as pyserial
import threading

import configparser
import os

#import ddbmod
from . import ddbmod

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
    def isUIRunning(self):
        return True
    def timeSlice(self):
        pass
    def bridge_send(self, transDir, line):
        pass
    def printLogMessage(self, msg):
        pass
    def printControlMessage(self, msg):
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


class DDBridge(ddbmod.DDBridge):
    def _sendLine(self, line, transDir):
        sent = False
        if line != None:
            DdUI.bridge_send(transDir, line)
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


def _Connect(port, baud) -> pyserial.Serial:
    DdUI.printLogMessage("Connect to {0} with baud rate {1}".format(port, baud))
    if port != "":
        ser = pyserial.Serial(port=port,baudrate=baud,
                              parity=pyserial.PARITY_NONE,stopbits=pyserial.STOPBITS_ONE,bytesize=pyserial.EIGHTBITS,
                              timeout=0)
        DdUI.printControlMessage("*** connected to: " + ser.portstr)
        return ser
    else:
        return None

def _Disconnect():
    global Ser
    #global DdUI
    Ser.close()
    Ser = None
    DdUI.printLogMessage("Disconnected")


def InvokeConnect(port, baud, wifi_port):
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
        Config[ConfigSectionName][BaudRateConfigName] = str(baud)
        Config[ConfigSectionName][WiFiPortConfigName] = str(wifi_port)
        with open(ConfigFileName, 'w') as configfile:
            Config.write(configfile)
        try:
            Ser = _Connect(port, baud)
            if Ser != None:
                Bridge = DDBridge()
                Serial = ddbmod.SerialSource(Ser, Bridge)
                Wifi = ddbmod.WifiTarget(Bridge, WifiHost, wifi_port)
                DdUI.onConnected()
                threading.Thread(target=SerialServe, daemon=True).start()
                threading.Thread(target=WifiServe, daemon=True).start()
        except pyserial.SerialException as err:
            DdUI.printControlMessage("*** serial exception while connecting -- {0}".format(err))
            DdUI.onSerialException(err)
    else:
        _Disconnect()

def SerialLoop():
    while DdUI.isUIRunning():
        DdUI.timeSlice()
        if Serial.error != None:
            if Ser == None:
                break
            raise Serial.error
        if Serial.bridge == None or Wifi.bridge == None:
            _Disconnect()
            return
        Serial.timeSlice(Bridge)

def NoSerialLoop():
    while DdUI.isUIRunning():
        DdUI.timeSlice()
        if Ser != None:
            return


def SerialServe():
    if Serial != None:
        Serial.serialServe()
def WifiServe():
    if Wifi != None:
        Wifi.serve()

def RunDDBridgeMain(ddui, connect_param_dict = None):
    global UIRunning
    global Ser
    global Serial
    global Wifi
    global Bridge

    _Initialize(ddui)
    ddui.initialize()
    if connect_param_dict != None:
        port = connect_param_dict["port"]
        baud = connect_param_dict.get("baud")
        wifi_port = connect_param_dict.get("wifiPort")
        if baud == None:
            baud = int(DefBaudRate)
        if wifi_port == None:
            wifi_port = int(DefWifiPort)
        InvokeConnect(port, baud, wifi_port)
    while ddui.isUIRunning():
        ddui.syncConnectionState(Ser != None)
        if Ser != None:
            try:
                SerialLoop()
            except Exception as err:
                err_msg = "*** exception -- {0}".format(err)
                print(err_msg, err)
                ddui.printControlMessage(err_msg)
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
        else:
            NoSerialLoop()

# if __name__ == "__main__":
#     print("Please run DDWifiBridge.py instead!!!")
