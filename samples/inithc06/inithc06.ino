// ***
// * notes for HC-06:
// * . default baud rate if 9600
// * . serial monitor needs no line ending
// * notes for HC-05:
// * . default baud rate if 38400
// * . serial monitor needs NL and CR
// * . need to hold its button while powering on, in order to turn on its AT mode
// ***


#include <SoftwareSerial.h>
  

SoftwareSerial hc06(2, 3);  // 2 => TX of HC06; 3 => RX of HC06


// for HC-6
//   9600: AT+BAUD4
//   57600: AT+BAUD7
//   115200: AT+BAUD8
//   arduino: AT+NAMEarduino
//   1234: AT+PIN1234

// for HC-05
//   get name: AT+NAME?
//   set name: AT+NAME=arduino
//   set password: AT+PSWD=1234
//   get baud: AT+UART? 
//   set baud: AT+UART=115200,0,0 



const unsigned long baud = 38400;  // default for hc-06 is 9600; default for hc-05 is 38400 


void setup() {

  // initialize Serial 
  Serial.begin(baud);
  Serial.println("Hello from Arduino. Please enter AT command:");
  
  // initialize HC06
  hc06.begin(baud);

}

void loop() {
  
  // write data from HC06 to Serial
  if (hc06.available()) {
    Serial.write(hc06.read());
  }
  
  // write data from Serial to HC06
  if (Serial.available()) {
    hc06.write(Serial.read());
  }  

}
