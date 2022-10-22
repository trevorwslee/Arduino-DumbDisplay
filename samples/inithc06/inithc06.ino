// ***
// * notes for HC-06 / HC-08:
// * . default baud rate is 9600
// * . serial monitor needs no line ending; ie no NL and no CR
// * notes for HC-05:
// * . need be in AT mode; need to hold its button while powering on; LED on HC-05 will flash slowly
// * . baud rate is 38400
// * . serial monitor needs NL and CR
// ***


#include <SoftwareSerial.h>
  
//SoftwareSerial hc(11, 10);
SoftwareSerial hc(2, 3);  // 2 => TX of HC; 3 => RX of HC


// for HC-06
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
//   reset: AT+ORGL


// for HC-08 (set up like HC-06)
//  get info: AT+RX  
//  set name: AT+NAME=xxx
//  set baud: AT+BAUD=nnn 
//  reset: AT+RESET
//  reset to default: AT+DEFAULT


const unsigned long baud = 115200;  // default for HC-06 / HC-08 is 9600; for HC-05, set to 38400 (assume HC-05 in AT mode)


void setup() {

  // initialize Serial 
  Serial.begin(baud);
  Serial.println("Hello from Arduino. Please enter AT command:");
  
  // initialize HC06
  hc.begin(baud);

}

void loop() {
  
  // write data from HC06 to Serial
  if (hc.available()) {
    Serial.write(hc.read());
  }
  
  // write data from Serial to HC06
  if (Serial.available()) {
    hc.write(Serial.read());
  }  
}


