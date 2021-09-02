from serial.tools.list_ports import comports


import ddbcore



def PrintLogMessage(msg):
    print("... " + msg)
def PrintControlMessage(msg):
    print("*** " + msg)
def PrintTransLine(transDir, line):
    print((transDir * 3) + " " + line)

def RunDDBridgeClMain():
    ddui = DDClUserInterface()
    ddbcore.RunDDBridgeMain(ddui)

class DDClUserInterface(ddbcore.DDUserInterface):
    def initialize(self):
        pass
    def syncConnectionState(self, connected):
        pass
    def onConnected(self):
        pass
    def onDisconnected(self):
        pass
    def timeSlice(self):
        pass
    def bridge_send(self, transDir, line):
        PrintTransLine(transDir, line)
    def printLogMessage(self, msg):
        PrintLogMessage(msg)
    def printControlMessage(self, msg):
        PrintControlMessage(msg)

if __name__ == "__main__":
    print("Please run DDClWifiBridge.py instead!!!")
