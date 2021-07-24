
if __name__ == "__main__":
    import importlib.util
    spec = importlib.util.find_spec("serial")
    if spec is None:
        message = "pyserial is not installed ...\n can install by running command ...\n pip install pyserial"
        print(message)
        from tkinter import messagebox
        messagebox.showinfo("DumbDisplay WIFI Bridge", message)
    else:    
        import ddbmain
        ddbmain.RunDDBridgeMain()
else:
    print("This is not a module!")

