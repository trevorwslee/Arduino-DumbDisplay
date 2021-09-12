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


def _Connect(port, baud):
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
        Config[ConfigSectionName][BaudRateConfigName] = str(baud)
        Config[ConfigSectionName][WiFiPortConfigName] = str(wifiPort)
        with open(ConfigFileName, 'w') as configfile:
            Config.write(configfile)
        try:
            Ser = _Connect(port, baud)
            if Ser != None:
                Bridge = DDBridge()
                Serial = ddbmod.SerialSource(Ser, Bridge)
                Wifi = ddbmod.WifiTarget(Bridge, WifiHost, wifiPort)
                DdUI.onConnected()
                threading.Thread(target=SerialServe, daemon=True).start()
                threading.Thread(target=WifiServe, daemon=True).start()
        except pyserial.SerialException as err:
            DdUI.printControlMessage("*** serial exception while connecting -- {0}".format(err))
            DdUI.onSerialException(err)
    else:
        _Disconnect()

def SerialLoop():
    while True:
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
    while True:
        DdUI.timeSlice()
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
            try:
                SerialLoop()
            except Exception as err:
                ddui.printControlMessage("*** exception -- {0}".format(err))
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

if __name__ == "__main__":
    print("Please run DDWifiBridge.py instead!!!")
