/*
  ToF + motor + dual encoder behavior test
  Board: Arduino Uno R4 WiFi

  Purpose:
    Read VL53L1X / TOF400C distance.
    Convert distance into locomotion behavior states.
    Control two DC motors through TB6612FNG.
    Print left/right encoder pulse counts while the robot moves.

  Motor wiring:
    A channel = AO1/AO2 = right motor
    B channel = BO1/BO2 = left motor

    PWMA -> D5
    AIN2 -> D3
    AIN1 -> D2
    STBY -> D7
    BIN1 -> D4
    BIN2 -> D8
    PWMB -> D6

  Encoder wiring:
    Left encoder DO  -> D9
    Right encoder DO -> D10
    Encoder VCC      -> 5V
    Encoder GND      -> GND
    Encoder AO       -> not connected

  ToF wiring:
    VIN -> 5V
    GND -> GND
    SDA -> SDA
    SCL -> SCL
    INT / SHUT not connected

  Required Arduino library:
    VL53L1X by Pololu

  Serial commands:
    s = stop and pause behavior
    g = resume behavior
    c = clear pulse counts
    1-9 = scale movement speed

  Serial Monitor:
    9600 baud
*/

#include <Wire.h>
#include <VL53L1X.h>
#include <string.h>

VL53L1X sensor;

const int AIN1 = 2;
const int AIN2 = 3;
const int PWMA = 5;

const int BIN1 = 4;
const int BIN2 = 8;
const int PWMB = 6;

const int STBY = 7;

const int LEFT_ENCODER_PIN = 9;
const int RIGHT_ENCODER_PIN = 10;

const int REFUSE_MM = 200;
const int NEGOTIATE_MM = 550;
const int HESITATE_MM = 700;

const float FILTER_ALPHA = 0.25;

const int RIGHT_OFFSET = 35;
const int LEFT_OFFSET = 0;

int baseSpeed = 150;
float filteredDistance = 0;
bool hasReading = false;
bool behaviorEnabled = true;

int lastLeftState = HIGH;
int lastRightState = HIGH;

unsigned long leftPulses = 0;
unsigned long rightPulses = 0;
unsigned long lastSensorTime = 0;
unsigned long lastPrintTime = 0;
unsigned long stateStartTime = 0;

const char* currentState = "START";

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(STBY, OUTPUT);

  pinMode(LEFT_ENCODER_PIN, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_PIN, INPUT_PULLUP);

  digitalWrite(STBY, HIGH);
  stopMotors();

  Serial.begin(9600);
  delay(1000);

  Wire.begin();
  Wire.setClock(400000);

  sensor.setTimeout(500);

  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize VL53L1X sensor.");
    Serial.println("Check VIN, GND, SDA, and SCL wiring.");
    while (true) {
      stopMotors();
      delay(1000);
    }
  }

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);

  lastLeftState = digitalRead(LEFT_ENCODER_PIN);
  lastRightState = digitalRead(RIGHT_ENCODER_PIN);

  Serial.println("ToF motor behavior test ready.");
  Serial.println("States:");
  Serial.println("  > 700 mm   APPROACH");
  Serial.println("  550-700 mm HESITATE");
  Serial.println("  200-550 mm NEGOTIATE");
  Serial.println("  < 200 mm   REFUSE");
  Serial.println("Commands: s=stop, g=go, c=clear pulses, 1-9=speed scale");
}

void loop() {
  readEncoders();
  readSerialCommand();
  readSensorEvery100ms();

  if (behaviorEnabled) {
    runBehavior();
  }

  printStatusEvery500ms();
}

void readEncoders() {
  int currentLeftState = digitalRead(LEFT_ENCODER_PIN);
  int currentRightState = digitalRead(RIGHT_ENCODER_PIN);

  if (currentLeftState != lastLeftState) {
    leftPulses++;
    lastLeftState = currentLeftState;
  }

  if (currentRightState != lastRightState) {
    rightPulses++;
    lastRightState = currentRightState;
  }
}

void readSensorEvery100ms() {
  if (millis() - lastSensorTime < 100) {
    return;
  }

  lastSensorTime = millis();

  int rawDistance = sensor.read();

  if (sensor.timeoutOccurred()) {
    currentState = "TIMEOUT";
    stopMotors();
    return;
  }

  updateFilter(rawDistance);
  setState(getBehaviorState(filteredDistance));
}

void updateFilter(int rawDistance) {
  if (!hasReading) {
    filteredDistance = rawDistance;
    hasReading = true;
    return;
  }

  filteredDistance =
    FILTER_ALPHA * rawDistance + (1.0 - FILTER_ALPHA) * filteredDistance;
}

const char* getBehaviorState(float distanceMm) {
  if (distanceMm < REFUSE_MM) {
    return "REFUSE";
  }

  if (distanceMm < NEGOTIATE_MM) {
    return "NEGOTIATE";
  }

  if (distanceMm < HESITATE_MM) {
    return "HESITATE";
  }

  return "APPROACH";
}

void setState(const char* nextState) {
  if (strcmp(currentState, nextState) == 0) {
    return;
  }

  currentState = nextState;
  stateStartTime = millis();
}

void runBehavior() {
  if (strcmp(currentState, "APPROACH") == 0) {
    forwardScaled(0.85);
    return;
  }

  if (strcmp(currentState, "HESITATE") == 0) {
    unsigned long phase = (millis() - stateStartTime) % 1200;

    if (phase < 650) {
      stopMotors();
    } else {
      forwardScaled(0.45);
    }

    return;
  }

  if (strcmp(currentState, "NEGOTIATE") == 0) {
    unsigned long phase = (millis() - stateStartTime) % 1000;

    if (phase < 500) {
      softTurnLeft();
    } else {
      softTurnRight();
    }

    return;
  }

  if (strcmp(currentState, "REFUSE") == 0) {
    unsigned long phase = (millis() - stateStartTime) % 1400;

    if (phase < 450) {
      backwardScaled(0.75);
    } else {
      stopMotors();
    }

    return;
  }

  stopMotors();
}

void readSerialCommand() {
  if (Serial.available() <= 0) {
    return;
  }

  char command = Serial.read();

  if (command == '\n' || command == '\r' || command == ' ') {
    return;
  }

  if (command >= '1' && command <= '9') {
    baseSpeed = map(command - '0', 1, 9, 90, 220);
    Serial.print("base speed = ");
    Serial.println(baseSpeed);
    return;
  }

  switch (command) {
    case 's':
    case 'S':
      behaviorEnabled = false;
      stopMotors();
      Serial.println("behavior paused");
      break;

    case 'g':
    case 'G':
      behaviorEnabled = true;
      Serial.println("behavior resumed");
      break;

    case 'c':
    case 'C':
      clearPulses();
      Serial.println("pulse counts cleared");
      break;

    default:
      Serial.print("unknown command: ");
      Serial.println(command);
      break;
  }
}

void printStatusEvery500ms() {
  if (millis() - lastPrintTime < 500) {
    return;
  }

  lastPrintTime = millis();

  long difference = (long)leftPulses - (long)rightPulses;

  Serial.print("Distance: ");
  Serial.print((int)filteredDistance);
  Serial.print(" mm | State: ");
  Serial.print(currentState);
  Serial.print(" | Left pulses: ");
  Serial.print(leftPulses);
  Serial.print(" | Right pulses: ");
  Serial.print(rightPulses);
  Serial.print(" | L-R diff: ");
  Serial.print(difference);
  Serial.print(" | Base speed: ");
  Serial.println(baseSpeed);
}

void forwardScaled(float scale) {
  rightForward((int)(rightSpeed() * scale));
  leftForward((int)(leftSpeed() * scale));
}

void backwardScaled(float scale) {
  rightBackward((int)(rightSpeed() * scale));
  leftBackward((int)(leftSpeed() * scale));
}

void softTurnLeft() {
  rightForward((int)(rightSpeed() * 0.65));
  leftBackward((int)(leftSpeed() * 0.45));
}

void softTurnRight() {
  rightBackward((int)(rightSpeed() * 0.45));
  leftForward((int)(leftSpeed() * 0.65));
}

void rightForward(int value) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, constrain(value, 0, 255));
}

void rightBackward(int value) {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, constrain(value, 0, 255));
}

void leftForward(int value) {
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, constrain(value, 0, 255));
}

void leftBackward(int value) {
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, constrain(value, 0, 255));
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

int rightSpeed() {
  return constrain(baseSpeed + RIGHT_OFFSET, 0, 255);
}

int leftSpeed() {
  return constrain(baseSpeed + LEFT_OFFSET, 0, 255);
}

void clearPulses() {
  leftPulses = 0;
  rightPulses = 0;
}
