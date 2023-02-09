

#define LED_PIN 2
#define FAN_PIN 0



void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
    digitalWrite(FAN_PIN, 0);
    digitalWrite(LED_PIN, 0);
    delay(2000);
    digitalWrite(FAN_PIN, 1);
    digitalWrite(LED_PIN, 1);
    delay(5000);
}