# . will need the PySerial module ... i.e. if needed to, install it like
#       pip install pyserizl
# . in Linux system, access serial port will need access right ... in such case ...
#       sudo usermod -a -G dialout <user>

import sys, getopt


def main():
    options = None
    if len(sys.argv) > 1:
        options, _ = getopt.getopt(sys.argv[1:], "ip:b:w:", ["port=", "baud=", "wifiport="])
    command_line = False
    port = None
    baud = None
    wifiPort = None
    if options != None:
        command_line = True
        for opt, arg in options:
            if opt in ('-i'):
                command_line = False
            elif opt in ('-p', '--port'):
                port = arg
            elif opt in ('-b', '--baud'):
                baud = int(arg)
            elif opt in ('-w', '--wifiport'):
                wifiPort = int(arg)
    import importlib.util
    spec = importlib.util.find_spec("serial")
    if spec is None:
        message = "pyserial is not installed ...\n can install by running command ...\n pip install pyserial"
        print(message)
        if not command_line:
            from tkinter import messagebox
            messagebox.showinfo("DumbDisplay WIFI Bridge", message)
    else:
        if command_line:
            #options, _ = getopt.getopt(sys.argv[1:], "p:b:w:", ["port=", "baud=", "wifiport="])
            if port == None:
                raise Exception("must provide port")
            #import ddbclmain
            from . import ddbclmain
            param_dict = {"port": port, "baud": baud, "wifiPort": wifiPort }
            ddbclmain.RunDDBridgeClMain(param_dict)
        else:
            #import ddbwinmain
            from . import ddbwinmain
            param_dict = {"port": port, "baud": baud, "wifiPort": wifiPort }
            ddbwinmain.RunDDBridgeWinMain(param_dict)


if __name__ == "__main__":
    main()
