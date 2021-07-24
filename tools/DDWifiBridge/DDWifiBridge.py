
import importlib.util


if __name__ == "__main__":
    spec = importlib.util.find_spec("serial")
    if spec is None:
        message = "pyserial is not installed ...\n can install by running command ...\n pip install pyserial"
        print(message)
        from tkinter import messagebox
        messagebox.showinfo("DumbDisplay Bridge", message)
    else:    
        import ddbmain
        ddbmain.RunDumbDisplayBridgeMain()
else:
    print("This is not a module!")

