// assume the 7 segment display is "common cathod"

// define the pins connected to the 7 segment display
const int p_g = 21;
const int p_f = 20;
const int p_a = 19;
const int p_b = 18;
const int p_e = 10;
const int p_d = 11;
const int p_c = 12;
const int p_dp = 13;

void setup() {
   // setup the pins to output signal
   pinMode(p_g, OUTPUT);
   pinMode(p_f, OUTPUT);
   pinMode(p_a, OUTPUT);
   pinMode(p_b, OUTPUT);
   pinMode(p_e, OUTPUT);
   pinMode(p_d, OUTPUT);
   pinMode(p_c, OUTPUT);
   pinMode(p_dp, OUTPUT);
}

void loop() {
   // delay a second then turn the segments on one by one

   delay(1000);
   digitalWrite(p_g, HIGH);

   delay(1000);
   digitalWrite(p_f, HIGH);

   delay(1000);
   digitalWrite(p_a, HIGH);

   delay(1000);
   digitalWrite(p_b, HIGH);

   delay(1000);
   digitalWrite(p_e, HIGH);

   delay(1000);
   digitalWrite(p_d, HIGH);

   delay(1000);
   digitalWrite(p_c, HIGH);

   delay(1000);
   digitalWrite(p_dp, HIGH);

   // delay 3 seconds and turn all segments off
   delay(3000);
   digitalWrite(p_g, LOW);
   digitalWrite(p_f, LOW);
   digitalWrite(p_a, LOW);
   digitalWrite(p_b, LOW);
   digitalWrite(p_e, LOW);
   digitalWrite(p_d, LOW);
   digitalWrite(p_c, LOW);
   digitalWrite(p_dp, LOW);

   // delay a second and loop
   delay(1000);
}
