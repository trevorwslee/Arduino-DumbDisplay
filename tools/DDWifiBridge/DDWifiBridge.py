# . will need the PySerial module ... i.e. if needed to, install it like
#       pip install pyserizl
# . in Linux system, access serial port will need access right ... in such case ...
#       sudo usermod -a -G dialout <user>

from DDWifiBridge.__main__ import main

if __name__ == "__main__":
    main()
