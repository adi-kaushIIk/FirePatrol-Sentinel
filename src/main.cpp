#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo;

const int SERVO_PIN = 18;

void setup() {
    Serial.begin(115200);

    servo.attach(SERVO_PIN);

    servo.write(0);
    Serial.println("Servo test started at 0 degrees");

    delay(1000);
}

void loop() {

    Serial.println("Moving 0 -> 90");
    for (int angle = 0; angle <= 90; angle++) {
        servo.write(angle);
        delay(15);
    }

    delay(500);

    Serial.println("Moving 90 -> 180");
    for (int angle = 90; angle <= 180; angle++) {
        servo.write(angle);
        delay(15);
    }

    delay(500);

    Serial.println("Moving 180 -> 90");
    for (int angle = 180; angle >= 90; angle--) {
        servo.write(angle);
        delay(15);
    }

    delay(500);

    Serial.println("Moving 90 -> 0");
    for (int angle = 90; angle >= 0; angle--) {
        servo.write(angle);
        delay(15);
    }

    delay(500);
}