
#ifdef ESP32

// for ESP32 Dev Kit compatible board
// - pin D16 connect to TX of XFS5152CE board; D17 connect to RX of XFS5152CE board
// - use Serial2

#define synthesizer Serial2

#else

// for Arduino UNO compatible board
// - pin 9 connect to TX of XFS5152CE board; 8 connect to RX of XFS5152CE board
// - use SoftwareSerial

#include <SoftwareSerial.h>
SoftwareSerial synthesizer(9, 8);

#endif



void setup() {
    synthesizer.begin(115200);

    String text = "[h0]Hi! Peace be with you! Jesus loves you!";  // [h0] annotates that the text that follows should be read out (instead of spelt out)

    synthesizer.write((byte) 0xFD);                 // header
    synthesizer.write((byte) 0x00);                 // data len: higher order byte
    synthesizer.write((byte) (2 + text.length()));  // data len: lower order byte
    synthesizer.write((byte) 0x01);                 // command -- synthesize
    synthesizer.write((byte) 0x00);                 // text encoding; for ASCII, simply use 0x00      
    synthesizer.print(text);                        // data -- text to synthesize
}


void loop() {
}
