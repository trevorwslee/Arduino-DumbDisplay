#include <SoftwareSerial.h>
  
SoftwareSerial hc06(2, 3);


// 9600: AT+BAUD4
// 57600: AT+BAUD7
// 115200: AT+BAUD8
// arduino: AT+NAMEarduino
// 1234: AT+PIN1234


const unsigned long baud = 115200;


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


