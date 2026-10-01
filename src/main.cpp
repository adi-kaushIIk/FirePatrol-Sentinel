#include <Arduino.h>

// HC-SR04 pins
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);

    Serial.println("FirePatrol Sentinel");
    Serial.println("Ultrasonic sensor initialized.");
}

void loop() {
    // Send a 10-microsecond trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the echo time
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
        Serial.println("No echo detected");
    } else {
        // Speed of sound ≈ 0.0343 cm/us
        float distance = duration * 0.0343 / 2;

        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
    }

    delay(500);
}