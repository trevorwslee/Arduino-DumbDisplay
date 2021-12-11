import sys, time

from serial.tools.list_ports import comports


#import ddbcore
from . import ddbcore

def PrintLogMessage(msg):
    print("... " + msg)
def PrintControlMessage(msg):
    print("*** " + msg)
def PrintTransLine(transDir, line):
    print((transDir * 3) + " " + line)

def RunDDBridgeClMain(param_dict = None):
    ddui = DDClUserInterface()
    ddbcore.RunDDBridgeMain(ddui, param_dict)

class DDClUserInterface(ddbcore.DDUserInterface):
    def initialize(self):
        pass
    def syncConnectionState(self, connected):
        pass
    def onSerialException(self, err):
        sys.exit()
    def onConnected(self):
        pass
    def onDisconnected(self):
        sys.exit()
    def timeSlice(self):
        time.sleep(0.0001)
    def bridge_send(self, transDir, line):
        PrintTransLine(transDir, line)
    def printLogMessage(self, msg):
        PrintLogMessage(msg)
    def printControlMessage(self, msg):
        PrintControlMessage(msg)

# if __name__ == "__main__":
#     print("Please run DDClWifiBridge.py instead!!!")
