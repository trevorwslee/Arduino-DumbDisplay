
DumbDispaly WIFI Bridge (DDWifiBridge) is a tool that accompanies DumbDisplay Arduino Library;hence, you might be interested in DDWifiBridge in the context it -- https://github.com/trevorwslee/Arduino-DumbDisplay


Very likely you will be using your desktop computer (Windows) for Arduino development, which will be connecting to your Arduino board via Serial connection. Wouldn't it be nice to be able to connect to DumbDisplay similarly, via the same Serial wiring as well?

Yes, you can do exactly that, with the help of the simple DumbDisplay WIFI Bridge Python program -- tools/DDWifiBrideg/DDWifiBridge.py. DumbDisplay WIFI Bridge acts as a "bridge" / "proxy" between your Ardiono board (Serial connection) and your mobile phone (WIFI connection). 

When running the DumbDisplay WIFI Bridge, on one side, it connects to your Arduino board via Serial connection, similar to how you Arduino IDE connect to your Arduino board. At the same time, it listens on port 10201 of your desktop, allowing DumbDisply to establish connection via WIFI. In other words, your desktop computer port 10201 is now a "bridge" / "proxy" to your Arduino DumpDisplay code. 

Notes:
* DumbDisply WIFI Bridge makes use of the PySerial library, which can be install like
  ```
  pip install pyserial
  ```
* Can run DumbDisply WIFI Bridge as a command-line tool (without UI). Simply call it with necessary arguments like "port" (-p <port> / --port=<port>), "baud" (-b <baud> / --baud=<baud>), and "wifi port" (-w <wifi-port> / --wifiport=<wifi-port>), like
  ```
  python DDWifiBridge.py --port=COM5 --baud=15200
  ```
* In Linux, acessing serial port will need special access right; you can grant such right to yourself (the user) like
  ```
  sudo usermod -a -G dialout <user>
  ```  
* My own experience shows that using a slower serial baud rate (like 57600 or even lower like 9600) will make the connection more stable.
* ***If DumbDisplay fails to make connection to DumbDisplay WIFI Bridge, check your descktop firewall settings; try switching desktop WIFI to use 2.4 GHz.***  


![](https://raw.githubusercontent.com/trevorwslee/Arduino-DumbDisplay/master/screenshots/ddwifibridge.png)





