# . will need the PySerial module ... i.e. if needed to, install it like
#       pip install pyserizl
# . in Linux system, access serial port will need access right ... in such case ...
#       sudo usermod -a -G dialout <user>

import sys, getopt

if __name__ == "__main__":
    command_line = len(sys.argv) > 1
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
            options, _ = getopt.getopt(sys.argv[1:], "p:b:w:", ["port=", "baud=", "wifiport="])
            port = None
            baud = None
            wifiPort = None
            for opt, arg in options:
                if opt in ('-p', '--port'):
                    port = arg
                elif opt in ('-b', '--baud'):
                    baud = int(arg)
                elif opt in ('-w', '--wifiport'):
                    wifiPort = int(arg)
            if port == None:
                raise Exception("must provide port")
            import ddbclmain
            param_dict = {"port": port, "baud": baud, "wifiPort": wifiPort }
            ddbclmain.RunDDBridgeClMain(param_dict)
        else:
            import ddbwinmain
            ddbwinmain.RunDDBridgeWinMain()
else:
    print("This is not a module!")

