#include <Arduino.h>

const int MQ2_PIN = 32;

void setup() {
    Serial.begin(115200);

    pinMode(MQ2_PIN, INPUT);

    Serial.println("FirePatrol Sentinel");
    Serial.println("MQ-2 sensor initialized.");
}

void loop() {
    int smokeValue = analogRead(MQ2_PIN);

    Serial.print("MQ-2 Value: ");
    Serial.println(smokeValue);

    delay(500);
}