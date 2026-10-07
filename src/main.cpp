#include <Arduino.h>

// Left motor
const int LEFT_IN1 = 26;
const int LEFT_IN2 = 27;

// Right motor
const int RIGHT_IN3 = 14;
const int RIGHT_IN4 = 13;

// Testing speed: 0-255
const int SPEED = 70;

void stopMotors() {
    analogWrite(LEFT_IN1, 0);
    analogWrite(LEFT_IN2, 0);
    analogWrite(RIGHT_IN3, 0);
    analogWrite(RIGHT_IN4, 0);
}

void moveForward() {
    analogWrite(LEFT_IN1, SPEED);
    analogWrite(LEFT_IN2, 0);

    analogWrite(RIGHT_IN3, SPEED);
    analogWrite(RIGHT_IN4, 0);
}

void moveBackward() {
    analogWrite(LEFT_IN1, 0);
    analogWrite(LEFT_IN2, SPEED);

    analogWrite(RIGHT_IN3, 0);
    analogWrite(RIGHT_IN4, SPEED);
}

void turnLeft() {
    analogWrite(LEFT_IN1, 0);
    analogWrite(LEFT_IN2, SPEED);

    analogWrite(RIGHT_IN3, SPEED);
    analogWrite(RIGHT_IN4, 0);
}

void turnRight() {
    analogWrite(LEFT_IN1, SPEED);
    analogWrite(LEFT_IN2, 0);

    analogWrite(RIGHT_IN3, 0);
    analogWrite(RIGHT_IN4, SPEED);
}

void setup() {
    Serial.begin(115200);

    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);
    pinMode(RIGHT_IN3, OUTPUT);
    pinMode(RIGHT_IN4, OUTPUT);

    stopMotors();

    Serial.println("FirePatrol Sentinel");
    Serial.println("SLOW MOTOR TEST");
}

void loop() {

    Serial.println("FORWARD");
    moveForward();
    delay(2000);

    Serial.println("STOP");
    stopMotors();
    delay(1500);

    Serial.println("BACKWARD");
    moveBackward();
    delay(2000);

    Serial.println("STOP");
    stopMotors();
    delay(1500);

    Serial.println("LEFT");
    turnLeft();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(1500);

    Serial.println("RIGHT");
    turnRight();
    delay(1500);

    Serial.println("STOP");
    stopMotors();
    delay(3000);
}