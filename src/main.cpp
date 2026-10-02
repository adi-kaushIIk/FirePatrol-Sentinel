#include <Arduino.h>

const int ENA = 25;
const int ENB = 33;

const int LEFT_IN1 = 26;
const int LEFT_IN2 = 27;

const int RIGHT_IN3 = 14;
const int RIGHT_IN4 = 13;

void stopMotors() {
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);

    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);
    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, LOW);
}

void moveForward() {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN3, HIGH);
    digitalWrite(RIGHT_IN4, LOW);
}

void moveBackward() {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, HIGH);
}

void turnLeft() {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    digitalWrite(RIGHT_IN3, HIGH);
    digitalWrite(RIGHT_IN4, LOW);
}

void turnRight() {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);

    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, HIGH);
}

void setup() {
    Serial.begin(115200);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);

    pinMode(RIGHT_IN3, OUTPUT);
    pinMode(RIGHT_IN4, OUTPUT);

    stopMotors();

    Serial.println("=== FirePatrol Sentinel Motor Test ===");
}

void loop() {

    Serial.println("FORWARD");
    moveForward();
    delay(2000);

    Serial.println("STOP");
    stopMotors();
    delay(1000);

    Serial.println("BACKWARD");
    moveBackward();
    delay(2000);

    Serial.println("STOP");
    stopMotors();
    delay(1000);

    Serial.println("TURN LEFT");
    turnLeft();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(1000);

    Serial.println("TURN RIGHT");
    turnRight();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(3000);
}