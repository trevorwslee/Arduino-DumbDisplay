# . will need the PySerial module ... i.e. if needed to, install it like
#       pip install pyserizl
# . in Linux system, access serial port will need access right ... in such case ...
#       sudo usermod -a -G dialout <user>

if __name__ == "__main__":
    import importlib.util
    spec = importlib.util.find_spec("serial")
    if spec is None:
        message = "pyserial is not installed ...\n can install by running command ...\n pip install pyserial"
        print(message)
        from tkinter import messagebox
        messagebox.showinfo("DumbDisplay WIFI Bridge", message)
    else:
        # import ddbmain
        # ddbmain.RunDDBridgeMain()
        import ddbwinmain
        ddbwinmain.RunDDBridgeWinMain()
else:
    print("This is not a module!")

