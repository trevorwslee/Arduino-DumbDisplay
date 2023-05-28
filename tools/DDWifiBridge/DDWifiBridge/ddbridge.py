import sys, getopt


# def checkDependencies(command_line):
#     import importlib.util
#     while True:
#         spec = importlib.util.find_spec("serial")
#         if spec is None:
#             message = "pyserial is not installed ...\n can install by running command ...\n pip install pyserial"
#             print(message)
#             if not command_line:
#                 from tkinter import messagebox
#                 #messagebox.showinfo("DumbDisplay WIFI Bridge", message)
#                 message = message + "\ninstall it now?"
#                 answer = messagebox.askyesno("DumbDisplay WIFI Bridge", message)
#                 if answer:
#                     import subprocess
#                     subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyserial'], stdout=subprocess.DEVNULL)
#                     continue
#         break
#     return True

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
        message = "package 'pyserial' is not installed\ncan install it by running command\n    pip install pyserial"
        print(message)
        if not command_line:
            from tkinter import messagebox
            #messagebox.showinfo("DumbDisplay WIFI Bridge", message)
            message = message + "\ninstall it now?"
            answer = messagebox.askyesno("DumbDisplay WIFI Bridge", message)
            if answer:
                import subprocess
                subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyserial'], stdout=subprocess.DEVNULL)
                messagebox.showinfo("DumbDisplay WIFI Bridge", "Please try running DDWifiBridge again.")
    else:
        if command_line:
            #options, _ = getopt.getopt(sys.argv[1:], "p:b:w:", ["port=", "baud=", "wifiport="])
            if port == None:
                raise Exception("DDWifiBridge: must provide port")
            #import ddbclmain
            from . import ddbclmain
            param_dict = {"port": port, "baud": baud, "wifiPort": wifiPort }
            ddbclmain.RunDDBridgeClMain(param_dict)
        else:
            #import ddbwinmain
            from . import ddbwinmain
            param_dict = {"port": port, "baud": baud, "wifiPort": wifiPort }
            try:
                ddbwinmain.RunDDBridgeWinMain(param_dict)
            except:
                print("RunDDBridgeWinMain exception")
 
# if __name__ == "__main__":
#     main()
