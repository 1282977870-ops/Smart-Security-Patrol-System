#include <ESP32Servo.h>

// =====================================================
// Smart Security Patrol System
// Option A – Embedded Intelligence (Wokwi)
// ESP32 + Ultrasonic Sensor + Line Sensor(Button)
// Servo + LEDs + Stepper Motor(A4988)
// =====================================================


// =======================
// Pin Definitions
// =======================

// Ultrasonic Sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

// Simulated Line Sensor using Push Button
#define LINE_SENSOR_PIN 19

// LED Pins
#define LED_GREEN 21
#define LED_YELLOW 22
#define LED_RED 23

// Servo Motor
#define SERVO_PIN 13

// Stepper Motor Driver A4988
#define MOTOR_STEP 26
#define MOTOR_DIR 27


// =======================
// Thresholds
// =======================

const int obstacleThreshold = 20; // cm


// =======================
// FSM State Definitions
// =======================

enum State {
  IDLE,
  PATROL,
  AVOID,
  WARNING,
  ALERT,
  STOP_FAILSAFE
};

State currentState = IDLE;

Servo scannerServo;


// =======================
// Setup
// =======================

void setup() {
  Serial.begin(115200);

  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Button / line sensor
  // INPUT_PULLUP means:
  // Not pressed = HIGH
  // Pressed = LOW
  pinMode(LINE_SENSOR_PIN, INPUT);

  // LEDs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Stepper motor driver
  pinMode(MOTOR_STEP, OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);

  // Servo
  scannerServo.attach(SERVO_PIN);

  stopRobot();
  updateLED("GREEN");

  currentState = PATROL;

  Serial.println("====================================");
  Serial.println("Smart Security Patrol System Started");
  Serial.println("FSM Initial State: PATROL");
  Serial.println("====================================");
}


// =======================
// Main Loop
// =======================

void loop() {
  long distance = readUltrasonic();
  bool lineDetected = readLineSensor();

  updateState(distance, lineDetected);
  runStateAction(distance, lineDetected);

  delay(200);
}


// =======================
// Sensor Functions
// =======================

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1; // invalid reading
  }

  long distance = duration * 0.034 / 2;
  return distance;
}


bool readLineSensor() {
  int value = digitalRead(LINE_SENSOR_PIN);

  Serial.print("Line Sensor Raw: ");
  Serial.print(value);
  Serial.print(" | ");

  if (value == HIGH) {
    return true;   // Boundary detected
  } else {
    return false;  // No boundary
  }
}


// =======================
// FSM State Transition Logic
// =======================

void updateState(long distance, bool lineDetected) {
  if (distance <= 1) {
    currentState = STOP_FAILSAFE;
  }
  else if (distance < obstacleThreshold && lineDetected == true) {
    currentState = ALERT;
  }
  else if (distance < obstacleThreshold && lineDetected == false) {
    currentState = AVOID;
  }
  else if (lineDetected == true && distance >= obstacleThreshold) {
    currentState = WARNING;
  }
  else {
    currentState = PATROL;
  }
}


// =======================
// FSM Action Logic
// =======================

void runStateAction(long distance, bool lineDetected) {
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Boundary: ");
  Serial.print(lineDetected ? "YES" : "NO");
  Serial.print(" | State: ");

  switch (currentState) {
    case IDLE:
      stopRobot();
      updateLED("GREEN");
      Serial.println("IDLE | Action: Standby");
      break;

    case PATROL:
      updateLED("GREEN");
      scanEnvironment();
      moveForward();
      Serial.println("PATROL | Action: Moving forward and scanning");
      break;

    case AVOID:
      updateLED("RED");
      stopRobot();
      turnAway();
      Serial.println("AVOID | Action: Obstacle detected, turning away");
      break;

    case WARNING:
      updateLED("YELLOW");
      stopRobot();
      Serial.println("WARNING | Action: Boundary detected, stopping");
      break;

    case ALERT:
      updateLED("RED");
      stopRobot();
      Serial.println("ALERT | Action: Multiple unsafe conditions detected");
      break;

    case STOP_FAILSAFE:
      updateLED("YELLOW");
      stopRobot();
      Serial.println("STOP_FAILSAFE | Action: Invalid sensor reading, system stopped");
      break;
  }
}


// =======================
// Motor Functions
// =======================

void moveForward() {
  // Stepper motor forward direction
  digitalWrite(MOTOR_DIR, HIGH);

  for (int i = 0; i < 40; i++) {
    digitalWrite(MOTOR_STEP, HIGH);
    delayMicroseconds(800);
    digitalWrite(MOTOR_STEP, LOW);
    delayMicroseconds(800);
  }
}


void turnAway() {
  // Stepper motor reverse direction
  digitalWrite(MOTOR_DIR, LOW);

  for (int i = 0; i < 80; i++) {
    digitalWrite(MOTOR_STEP, HIGH);
    delayMicroseconds(800);
    digitalWrite(MOTOR_STEP, LOW);
    delayMicroseconds(800);
  }
}


void stopRobot() {
  digitalWrite(MOTOR_STEP, LOW);
}


// =======================
// Servo Function
// =======================

void scanEnvironment() {
  scannerServo.write(60);
  delay(80);
  scannerServo.write(120);
  delay(80);
  scannerServo.write(90);
}


// =======================
// LED Function
// =======================

void updateLED(String colour) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (colour == "GREEN") {
    digitalWrite(LED_GREEN, HIGH);
  }
  else if (colour == "YELLOW") {
    digitalWrite(LED_YELLOW, HIGH);
  }
  else if (colour == "RED") {
    digitalWrite(LED_RED, HIGH);
  }
}
