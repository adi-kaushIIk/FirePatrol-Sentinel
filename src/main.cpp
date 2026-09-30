#include <Arduino.h>

// Left motor
const int LEFT_IN1 = 26;
const int LEFT_IN2 = 27;

// Right motor
const int RIGHT_IN3 = 14;
const int RIGHT_IN4 = 13;

// Stop both motors
void stopMotors() {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, LOW);
}

// Move forward
void moveForward() {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN3, HIGH);
    digitalWrite(RIGHT_IN4, LOW);
}

// Move backward
void moveBackward() {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, HIGH);
}

// Turn left
void turnLeft() {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    digitalWrite(RIGHT_IN3, HIGH);
    digitalWrite(RIGHT_IN4, LOW);
}

// Turn right
void turnRight() {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, HIGH);
}

void setup() {
    Serial.begin(115200);

    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);

    pinMode(RIGHT_IN3, OUTPUT);
    pinMode(RIGHT_IN4, OUTPUT);

    stopMotors();

    Serial.println("FirePatrol Sentinel");
    Serial.println("Motor control initialized.");
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

    Serial.println("LEFT");
    turnLeft();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(1000);

    Serial.println("RIGHT");
    turnRight();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(2000);
}