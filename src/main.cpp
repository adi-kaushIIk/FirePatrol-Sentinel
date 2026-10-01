#include <Arduino.h>

const int FLAME_PIN = 34;

void setup() {
    Serial.begin(115200);

    pinMode(FLAME_PIN, INPUT);

    Serial.println("FirePatrol Sentinel");
    Serial.println("Flame sensor initialized.");
}

void loop() {
    int flameState = digitalRead(FLAME_PIN);

    if (flameState == LOW) {
        Serial.println("FLAME DETECTED!");
    } else {
        Serial.println("No flame detected.");
    }

    delay(300);
}