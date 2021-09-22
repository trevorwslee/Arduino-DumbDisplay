
//   9600: AT+BAUD4
//   57600: AT+BAUD7
//   115200: AT+BAUD8
//   Pico: AT+NAMEPico
//   1234: AT+PIN1234



UART Serial2(8, 9, 0, 0);  // GP8 => RX of HC06; GP9 => TX of HC06

const unsigned long baud = 115200;


void setup() {
  // initialize Serial 
  Serial.begin(baud);
  
  // initialize HC06 @ Pico Uart2 (Serial2) 
  Serial2.begin(baud);
}

void loop() {
  // write data from Pico Serial2 to Serial
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  
  // write data from Pico Serial2 to HC06
  if (Serial.available()) {
    Serial2.write(Serial.read());
  }  
}
