#include <Servo.h>


#define SERVO_PIN    11

#define US_TRIG_PIN   9
#define US_ECHO_PIN  10


const int MaxAngle = 120;
const int AngleIncrement = 1;


Servo servo;


int angle = 0;
bool angleIncreasing = true;

void setup() {
  Serial.begin(115200); // Starts the serial communication

  servo.attach(SERVO_PIN);

  servo.write(angle);


  pinMode(US_TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(US_ECHO_PIN, INPUT); // Sets the echoPin as an Input


  delay(1000);  // wait a bit first 


}

void loop() {
  servo.write(angle);
  delay(15);  // give it some time

  // read ultrasoncic sensor for detected object distance
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // . ses the trigPin on HIGH state for 10 micro seconds
  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);
  // . reas the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(US_ECHO_PIN, HIGH);
  // . calculating the distance ... 0.034: sound travels 0.034 cm per second
  int distance = duration * 0.034 / 2;
  // . prins the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  

  if (angleIncreasing) {
    angle += AngleIncrement;
  } else {
    angle -= AngleIncrement;
  }
  if (angle > MaxAngle) {
    angle = MaxAngle;
    angleIncreasing = false;
  } else {
    if (angle < 0) {
      angle = 0;
      angleIncreasing = true;
    }
  }
}