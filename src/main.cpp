#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// ---------------- Wi-Fi AP / mDNS ----------------
static const char *AP_SSID = "FirePatrol-Sentinel";
static const char *AP_PASSWORD = "PatrolSafe26"; // Change this before sharing the rover.
static const char *MDNS_NAME = "firepatrol";

// ---------------- Pin map: classic ESP32 DevKit ----------------
// L298N IN pins
constexpr uint8_t LEFT_IN1  = 26;
constexpr uint8_t LEFT_IN2  = 27;
constexpr uint8_t RIGHT_IN3 = 14;
constexpr uint8_t RIGHT_IN4 = 13;
// L298N ENA/ENB must be disconnected from 5V jumpers and wired here for PWM.
constexpr uint8_t LEFT_ENA  = 22;
constexpr uint8_t RIGHT_ENB = 23;
// HC-SR04
constexpr uint8_t SONAR_TRIG = 5;
constexpr uint8_t SONAR_ECHO = 18; // Use a resistor divider/level shifter: ECHO can be 5V.
// Sensors / outputs
constexpr uint8_t SERVO_PIN = 25;
constexpr uint8_t FLAME_PIN = 34; // input-only pin; check module logic
constexpr uint8_t MQ2_PIN   = 32; // ADC1 pin
constexpr uint8_t BUZZER_PIN = 33;

WebServer server(80);
Servo scanServo;

struct RoverState {
  bool running = false;
  bool flame = false;
  bool smoke = false;
  bool obstacle = false;
  bool buzzer = false;
  bool wifi = false;
  int distanceCm = -1;
  int mq2 = 0;
  int speedPct = 0;
  String hazard = "none";
  String mode = "standby";
  int scanAngle = 90;
  unsigned long updatedAt = 0;
};
RoverState state;

int smokeThreshold = 1800; // Provisional only. Calibrate against actual sensor readings.
int obstacleThresholdCm = 30;
int requestedSpeedPct = 45; // Practical starting target; 5% often cannot start TT motors.
bool autoScan = true;
unsigned long lastSensorMs = 0;
unsigned long lastScanMs = 0;
unsigned long lastTelemetryMs = 0;
int scanDirection = 1;
bool alarmPhase = false;
unsigned long lastAlarmMs = 0;

void setMotorPinsLow() {
  digitalWrite(LEFT_IN1, LOW); digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW); digitalWrite(RIGHT_IN4, LOW);
}
void applyMotorSpeed(int pct) {
  pct = constrain(pct, 0, 100);
  int duty = map(pct, 0, 100, 0, 255);
  analogWrite(LEFT_ENA, duty);
  analogWrite(RIGHT_ENB, duty);
}
void stopMotors() {
  state.running = false;
  state.speedPct = 0;
  setMotorPinsLow();
  applyMotorSpeed(0);
}
void driveForward() {
  digitalWrite(LEFT_IN1, HIGH); digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, HIGH); digitalWrite(RIGHT_IN4, LOW);
  applyMotorSpeed(requestedSpeedPct);
}
void driveBackward() {
  digitalWrite(LEFT_IN1, LOW); digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, LOW); digitalWrite(RIGHT_IN4, HIGH);
  applyMotorSpeed(requestedSpeedPct);
}
void pivotLeft() {
  digitalWrite(LEFT_IN1, LOW); digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, HIGH); digitalWrite(RIGHT_IN4, LOW);
  applyMotorSpeed(requestedSpeedPct);
}
void pivotRight() {
  digitalWrite(LEFT_IN1, HIGH); digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW); digitalWrite(RIGHT_IN4, HIGH);
  applyMotorSpeed(requestedSpeedPct);
}
long readDistanceCm() {
  digitalWrite(SONAR_TRIG, LOW); delayMicroseconds(3);
  digitalWrite(SONAR_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(SONAR_TRIG, LOW);
  unsigned long duration = pulseIn(SONAR_ECHO, HIGH, 25000UL);
  if (duration == 0) return -1;
  return (long)(duration * 0.0343f / 2.0f);
}
void updateSensors() {
  long d = readDistanceCm();
  state.distanceCm = (d >= 2 && d <= 400) ? (int)d : -1;
  state.mq2 = analogRead(MQ2_PIN);
  // KY-026 digital output is often active LOW, but verify with your module.
  state.flame = (digitalRead(FLAME_PIN) == LOW);
  state.smoke = state.mq2 >= smokeThreshold;
  state.obstacle = (state.distanceCm >= 0 && state.distanceCm <= obstacleThresholdCm);
  if (state.flame) state.hazard = "fire";
  else if (state.smoke) state.hazard = "smoke";
  else state.hazard = "none";
  state.updatedAt = millis();
}
void alarmTick() {
  bool hazard = state.hazard != "none";
  if (!hazard) {
    noTone(BUZZER_PIN);
    state.buzzer = false;
    return;
  }
  unsigned long now = millis();
  if (now - lastAlarmMs >= (state.hazard == "fire" ? 220 : 650)) {
    lastAlarmMs = now;
    alarmPhase = !alarmPhase;
    if (alarmPhase) tone(BUZZER_PIN, state.hazard == "fire" ? 2200 : 1300);
    else noTone(BUZZER_PIN);
  }
  state.buzzer = alarmPhase;
}
void patrolTick() {
  if (!state.running) return;
  if (state.hazard != "none") {
    stopMotors();
    state.mode = "hazard-stop";
    return;
  }
  if (state.obstacle) {
    stopMotors();
    state.mode = "obstacle-avoidance";
    // Small safe avoidance maneuver. The loop is non-blocking except brief turn.
    pivotRight();
    delay(350);
    stopMotors();
    state.mode = "patrolling";
  } else {
    driveForward();
    state.speedPct = requestedSpeedPct;
    state.mode = "patrolling";
  }
}
void scanTick() {
  if (!autoScan || !state.running || state.hazard != "none") return;
  if (millis() - lastScanMs < 45) return;
  lastScanMs = millis();
  state.scanAngle += scanDirection;
  if (state.scanAngle >= 150) { state.scanAngle = 150; scanDirection = -1; }
  if (state.scanAngle <= 30) { state.scanAngle = 30; scanDirection = 1; }
  scanServo.write(state.scanAngle);
}
void sendState() {
  JsonDocument doc;
  doc["connected"] = true;
  doc["running"] = state.running;
  doc["distance_cm"] = state.distanceCm;
  doc["mq2"] = state.mq2;
  doc["flame"] = state.flame;
  doc["smoke"] = state.smoke;
  doc["obstacle"] = state.obstacle;
  doc["hazard"] = state.hazard;
  doc["buzzer"] = state.buzzer;
  doc["speed_pct"] = state.speedPct;
  doc["requested_speed_pct"] = requestedSpeedPct;
  doc["mode"] = state.mode;
  doc["scan_angle"] = state.scanAngle;
  doc["smoke_threshold"] = smokeThreshold;
  doc["obstacle_threshold_cm"] = obstacleThresholdCm;
  doc["uptime_ms"] = millis();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}
void sendOk(const char *message) {
  JsonDocument doc;
  doc["ok"] = true;
  doc["message"] = message;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}
bool bodyJson(JsonDocument &doc) {
  if (!server.hasArg("plain")) return false;
  return deserializeJson(doc, server.arg("plain")) == DeserializationError::Ok;
}
void commandHandler() {
  JsonDocument doc;
  if (!bodyJson(doc)) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"Expected JSON body\"}"); return; }
  String cmd = doc["cmd"] | "";
  if (cmd == "start") {
    updateSensors();
    if (state.hazard != "none") { server.send(409, "application/json", "{\"ok\":false,\"error\":\"Hazard active; clear the area and check sensors\"}"); return; }
    state.running = true; state.mode = "patrolling";
    sendOk("Patrol started");
  } else if (cmd == "stop") {
    stopMotors(); state.mode = "standby"; sendOk("Rover stopped");
  } else if (cmd == "forward") {
    updateSensors();
    if (state.hazard != "none" || state.obstacle) { stopMotors(); server.send(409, "application/json", "{\"ok\":false,\"error\":\"Movement blocked by hazard or obstacle\"}"); return; }
    state.running = false; driveForward(); state.speedPct = requestedSpeedPct; state.mode = "manual-forward"; sendOk("Forward");
  } else if (cmd == "backward") {
    state.running = false; driveBackward(); state.speedPct = requestedSpeedPct; state.mode = "manual-backward"; sendOk("Backward");
  } else if (cmd == "left") {
    state.running = false; pivotLeft(); state.speedPct = requestedSpeedPct; state.mode = "manual-left"; sendOk("Turn left");
  } else if (cmd == "right") {
    state.running = false; pivotRight(); state.speedPct = requestedSpeedPct; state.mode = "manual-right"; sendOk("Turn right");
  } else if (cmd == "speed") {
    requestedSpeedPct = constrain((int)(doc["value"] | 45), 20, 100);
    if (state.running) driveForward();
    sendOk("Speed updated");
  } else if (cmd == "thresholds") {
    smokeThreshold = constrain((int)(doc["smoke"] | smokeThreshold), 0, 4095);
    obstacleThresholdCm = constrain((int)(doc["obstacle"] | obstacleThresholdCm), 10, 100);
    sendOk("Thresholds updated");
  } else if (cmd == "scan") {
    autoScan = doc["enabled"] | true;
    sendOk("Auto scan setting updated");
  } else {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"Unknown command\"}");
  }
}
void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    File f = LittleFS.open("/index.html", "r");
    if (!f) { server.send(500, "text/plain", "Dashboard files missing. Upload the LittleFS filesystem image."); return; }
    server.streamFile(f, "text/html"); f.close();
  });
  server.on("/api/state", HTTP_GET, sendState);
  server.on("/api/cmd", HTTP_POST, commandHandler);
  server.onNotFound([]() {
    String path = server.uri();
    if (path.indexOf("..") >= 0) { server.send(400, "text/plain", "Bad path"); return; }
    if (path == "/") path = "/index.html";
    File f = LittleFS.open(path, "r");
    if (!f || f.isDirectory()) { server.send(404, "text/plain", "Not found"); return; }
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".css")) type = "text/css";
    else if (path.endsWith(".js")) type = "application/javascript";
    else if (path.endsWith(".svg")) type = "image/svg+xml";
    server.streamFile(f, type); f.close();
  });
}
void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(LEFT_IN1, OUTPUT); pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT); pinMode(RIGHT_IN4, OUTPUT);
  pinMode(SONAR_TRIG, OUTPUT); pinMode(SONAR_ECHO, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  setMotorPinsLow();

  // analogWrite() generates PWM on the ESP32 enable pins.
  applyMotorSpeed(0);

  analogReadResolution(12);
  analogSetPinAttenuation(MQ2_PIN, ADC_11db);

  scanServo.setPeriodHertz(50);
  scanServo.attach(SERVO_PIN, 500, 2400);
  scanServo.write(90);
  state.scanAngle = 90;

  if (!LittleFS.begin(true)) Serial.println("LittleFS mount failed");
  WiFi.mode(WIFI_AP);
  bool apOk = WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println();
  Serial.println("FirePatrol Sentinel starting");
  Serial.printf("AP started: %s\n", apOk ? "yes" : "no");
  Serial.print("Wi-Fi name: "); Serial.println(AP_SSID);
  Serial.print("Wi-Fi password: "); Serial.println(AP_PASSWORD);
  Serial.print("Dashboard URL: http://"); Serial.println(WiFi.softAPIP());
  if (MDNS.begin(MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS: http://firepatrol.local");
  } else Serial.println("mDNS failed; use the IP address above");
  setupRoutes();
  server.begin();
  Serial.println("HTTP server ready");
}
void loop() {
  server.handleClient();
  unsigned long now = millis();
  if (now - lastSensorMs >= 250) {
    lastSensorMs = now;
    updateSensors();
    alarmTick();
  }
  patrolTick();
  scanTick();
}
