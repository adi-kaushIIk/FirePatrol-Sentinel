#include <Arduino.h>
#include <ESP32Servo.h>

// ============================================================
// FIREPATROL SENTINEL - AUTONOMOUS PATROL FIRMWARE
// ESP32 + L298N + HC-SR04 + MQ-2 + digital flame sensor
//       + servo scanner + active HIGH buzzer
//
// Uses the pin definitions from the supplied test code.
// IMPORTANT MOTOR SPEED WIRING:
//   ENA and ENB must NOT be hardwired to 5V if PWM speed control is used.
//   Remove the ENA and ENB jumpers from the L298N.
//   ENA -> ESP32 GPIO 26
//   ENB -> ESP32 GPIO 33
//   Keep ESP32 GND and L298N GND connected together.
//   Motor speed is set to 5% PWM duty.
//
// Verify the flame sensor's LOW-active logic and calibrate the
// MQ-2 threshold in your actual environment before relying on it.
// ============================================================

int speed = 100; // Default speed percentage (0-100)
// L298N direction pins (from supplied test code)
constexpr int IN1 = 25;
constexpr int IN2 = 27;
constexpr int IN3 = 21;
constexpr int IN4 = 13;

// L298N enable pins added for PWM speed control
constexpr int ENA_PIN = 26;
constexpr int ENB_PIN = 33;

// Ultrasonic HC-SR04
constexpr int TRIG_PIN = 5;
constexpr int ECHO_PIN = 18;

// Sensors
constexpr int MQ2_PIN = 32;
constexpr int FLAME_PIN = 17;

// Servo and buzzer
constexpr int SERVO_PIN = 12;
constexpr int BUZZER_PIN = 4;

// -------------------- TUNABLE SETTINGS --------------------
constexpr int MOTOR_SPEED_PERCENT = 5;   // Very slow: 5% PWM duty
constexpr int OBSTACLE_DISTANCE_CM = 30;
constexpr int SERVO_CENTER = 90;
constexpr int SERVO_LEFT = 150;
constexpr int SERVO_RIGHT = 30;

constexpr int SMOKE_THRESHOLD = 1800; // Provisional; calibrate this
constexpr bool FLAME_ACTIVE_LOW = true;

constexpr uint32_t SENSOR_INTERVAL_MS = 70;
constexpr uint32_t SERVO_SETTLE_MS = 450;
constexpr uint32_t REVERSE_TIME_MS = 250;
constexpr uint32_t TURN_TIME_MS = 450;
constexpr uint32_t BUZZER_UPDATE_MS = 10;
constexpr uint32_t ALARM_RECHECK_MS = 250;

// ESP32 Arduino analogWrite uses a 0..255 duty range.
constexpr int PWM_DUTY = (255 * MOTOR_SPEED_PERCENT) / 100;

Servo scanner;

enum Hazard : uint8_t { HAZARD_NONE, HAZARD_SMOKE, HAZARD_FIRE };
enum AvoidState : uint8_t {
  AVOID_IDLE,
  AVOID_SCAN_LEFT_WAIT,
  AVOID_SCAN_RIGHT_WAIT,
  AVOID_CENTER_WAIT,
  AVOID_REVERSE,
  AVOID_TURN
};

AvoidState avoidState = AVOID_IDLE;
Hazard activeHazard = HAZARD_NONE;

uint32_t lastSensorRead = 0;
uint32_t stateStarted = 0;
uint32_t lastBuzzerUpdate = 0;
uint32_t lastAlarmMessage = 0;

long frontDistanceCm = -1;
int leftDistanceCm = 400;
int rightDistanceCm = 400;
int mq2Value = 0;
bool flamePresent = false;
bool smokePresent = false;
bool buzzerOn = false;
bool turnLeftNext = false;

// -------------------- MOTOR CONTROL --------------------
void setMotorSpeed(int percent) {
  percent = constrain(percent, 0, 100);
  int duty = (255 * percent) / 100;
  analogWrite(ENA_PIN, duty);
  analogWrite(ENB_PIN, duty);
}

void stopMotors() {
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
  setMotorSpeed(0);
}

void moveForward() {
  setMotorSpeed(MOTOR_SPEED_PERCENT);
  analogWrite(IN1, speed);
  analogWrite(IN2, 0);
  analogWrite(IN3, speed);
  analogWrite(IN4, 0);
}

void moveBackward() {
  setMotorSpeed(MOTOR_SPEED_PERCENT);
  analogWrite(IN1, 0);
  analogWrite(IN2, speed);
  analogWrite(IN3, 0);
  analogWrite(IN4, speed);
}

void turnLeft() {
  setMotorSpeed(MOTOR_SPEED_PERCENT);
  analogWrite(IN1, 0);
  analogWrite(IN2, speed);
  analogWrite(IN3, speed);
  analogWrite(IN4, 0);
}

void turnRight() {
  setMotorSpeed(MOTOR_SPEED_PERCENT);
  analogWrite(IN1, speed);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, speed);
}

// -------------------- SENSOR READS --------------------
long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 25 ms timeout limits blocking time; timeout returns -1.
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return -1;

  long cm = static_cast<long>(duration * 0.0343f / 2.0f);
  if (cm < 2 || cm > 400) return -1;
  return cm;
}

void readHazardSensors() {
  mq2Value = analogRead(MQ2_PIN);
  int flameState = digitalRead(FLAME_PIN);
  flamePresent = FLAME_ACTIVE_LOW ? (flameState == LOW) : (flameState == HIGH);
  smokePresent = (mq2Value >= SMOKE_THRESHOLD);

  // Fire has priority if both sensor conditions are active.
  if (flamePresent) activeHazard = HAZARD_FIRE;
  else if (smokePresent) activeHazard = HAZARD_SMOKE;
  else activeHazard = HAZARD_NONE;
}

int distanceAtAngle(int angle) {
  scanner.write(angle);
  delay(SERVO_SETTLE_MS);
  long d = readDistanceCm();
  return (d > 0) ? static_cast<int>(d) : 400;
}

// -------------------- ALARM --------------------
// Non-blocking repeating patterns so sensors can continue to be read.
// This assumes the buzzer module sounds when its input is HIGH.
void updateBuzzer() {
  uint32_t now = millis();
  if (now - lastBuzzerUpdate < BUZZER_UPDATE_MS) return;
  lastBuzzerUpdate = now;

  if (activeHazard == HAZARD_NONE) {
    buzzerOn = false;
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }

  uint32_t t = now % (activeHazard == HAZARD_FIRE ? 1000UL : 1200UL);
  bool shouldSound;

  if (activeHazard == HAZARD_FIRE) {
    // Fast repeating fire alarm: 150 ms ON, 100 ms OFF, repeated.
    shouldSound = (t < 150) || (t >= 250 && t < 400) ||
                  (t >= 500 && t < 650) || (t >= 750 && t < 900);
  } else {
    // Smoke alarm: two short beeps, then a pause.
    shouldSound = (t < 180) || (t >= 300 && t < 480);
  }

  buzzerOn = shouldSound;
  digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
}

void handleHazard() {
  stopMotors();
  avoidState = AVOID_IDLE;
  scanner.write(SERVO_CENTER);

  uint32_t now = millis();
  if (now - lastAlarmMessage >= ALARM_RECHECK_MS) {
    lastAlarmMessage = now;
    if (activeHazard == HAZARD_FIRE) {
      Serial.println("!!! FIRE DETECTED: MOTORS STOPPED, FIRE ALARM ACTIVE !!!");
    } else if (activeHazard == HAZARD_SMOKE) {
      Serial.println("!!! SMOKE THRESHOLD EXCEEDED: MOTORS STOPPED, SMOKE ALARM ACTIVE !!!");
    }
  }
}

// -------------------- OBSTACLE AVOIDANCE STATE MACHINE --------------------
void beginObstacleAvoidance() {
  stopMotors();
  scanner.write(SERVO_LEFT);
  stateStarted = millis();
  avoidState = AVOID_SCAN_LEFT_WAIT;
  Serial.println("Obstacle detected. Stopping and scanning left...");
}

void updateObstacleAvoidance() {
  uint32_t now = millis();

  switch (avoidState) {
    case AVOID_IDLE:
      if (frontDistanceCm > 0 && frontDistanceCm <= OBSTACLE_DISTANCE_CM) {
        beginObstacleAvoidance();
      } else {
        moveForward();
      }
      break;

    case AVOID_SCAN_LEFT_WAIT:
      if (now - stateStarted >= SERVO_SETTLE_MS) {
        long d = readDistanceCm();
        leftDistanceCm = (d > 0) ? static_cast<int>(d) : 400;
        Serial.printf("Left clearance: %d cm\n", leftDistanceCm);
        scanner.write(SERVO_RIGHT);
        stateStarted = now;
        avoidState = AVOID_SCAN_RIGHT_WAIT;
      }
      break;

    case AVOID_SCAN_RIGHT_WAIT:
      if (now - stateStarted >= SERVO_SETTLE_MS) {
        long d = readDistanceCm();
        rightDistanceCm = (d > 0) ? static_cast<int>(d) : 400;
        Serial.printf("Right clearance: %d cm\n", rightDistanceCm);
        scanner.write(SERVO_CENTER);
        stateStarted = now;
        avoidState = AVOID_CENTER_WAIT;
      }
      break;

    case AVOID_CENTER_WAIT:
      if (now - stateStarted >= 250) {
        // Reverse briefly before turning, at the same low speed.
        moveBackward();
        stateStarted = now;
        avoidState = AVOID_REVERSE;
        Serial.println("Reversing briefly...");
      }
      break;

    case AVOID_REVERSE:
      if (now - stateStarted >= REVERSE_TIME_MS) {
        if (leftDistanceCm > rightDistanceCm) {
          Serial.println("Turning left...");
          turnLeft();
        } else {
          Serial.println("Turning right...");
          turnRight();
        }
        stateStarted = now;
        avoidState = AVOID_TURN;
      }
      break;

    case AVOID_TURN:
      if (now - stateStarted >= TURN_TIME_MS) {
        stopMotors();
        avoidState = AVOID_IDLE;
        Serial.println("Obstacle cleared check; resuming patrol.");
      }
      break;
  }
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(MQ2_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Make sure motors stay stopped during initialization.
  stopMotors();

  scanner.setPeriodHertz(50);
  scanner.attach(SERVO_PIN, 500, 2400);
  scanner.write(SERVO_CENTER);

  analogReadResolution(12); // ESP32 ADC range is normally 0..4095
  delay(700);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       FIREPATROL SENTINEL");
  Serial.println("========================================");
  Serial.printf("Motor speed: %d%% PWM\n", MOTOR_SPEED_PERCENT);
  Serial.printf("Obstacle threshold: %d cm\n", OBSTACLE_DISTANCE_CM);
  Serial.printf("MQ-2 threshold: %d (provisional)\n", SMOKE_THRESHOLD);
  Serial.println("Patrol started. Serial Monitor: 115200 baud.");
}

// -------------------- MAIN LOOP --------------------
void loop() {
  uint32_t now = millis();

  // Keep the hazard checks and buzzer service running throughout patrol.
  readHazardSensors();
  updateBuzzer();

  if (activeHazard != HAZARD_NONE) {
    handleHazard();
    delay(1);
    return;
  }

  // Read the ultrasonic sensor repeatedly, including while the servo
  // is scanning or the rover is turning. It is one sensor, not multiple.
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    frontDistanceCm = readDistanceCm();

    Serial.printf("Front: %s | MQ-2: %d | Flame: %s | Speed: %d%%\n",
                  frontDistanceCm > 0 ? String(frontDistanceCm).c_str() : "No echo",
                  mq2Value,
                  flamePresent ? "DETECTED" : "clear",
                  MOTOR_SPEED_PERCENT);
  }

  updateObstacleAvoidance();
  delay(1);
}
