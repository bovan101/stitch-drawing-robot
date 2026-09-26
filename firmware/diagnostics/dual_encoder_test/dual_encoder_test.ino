/*
  Dual LM393 slot encoder test
  Board: Arduino Uno R4 WiFi

  Purpose:
    Read two slot encoders at the same time.
    This sketch does not control motors.

  Wiring:
    Left encoder:
      VCC -> Arduino 5V / breadboard + rail
      GND -> Arduino GND / breadboard - rail
      DO  -> Arduino D9
      AO  -> not connected

    Right encoder:
      VCC -> Arduino 5V / breadboard + rail
      GND -> Arduino GND / breadboard - rail
      DO  -> Arduino D10
      AO  -> not connected

  Serial commands:
    r = reset pulse counts

  Serial Monitor:
    9600 baud
*/

const int LEFT_ENCODER_PIN = 9;
const int RIGHT_ENCODER_PIN = 10;

int lastLeftState = HIGH;
int lastRightState = HIGH;

unsigned long leftPulses = 0;
unsigned long rightPulses = 0;

unsigned long lastPrintTime = 0;

void setup() {
  pinMode(LEFT_ENCODER_PIN, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  delay(1000);

  lastLeftState = digitalRead(LEFT_ENCODER_PIN);
  lastRightState = digitalRead(RIGHT_ENCODER_PIN);

  Serial.println("Dual encoder test ready.");
  Serial.println("Rotate each wheel by hand.");
  Serial.println("Command: r = reset counts");
}

void loop() {
  readEncoders();
  readSerialCommand();
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

void readSerialCommand() {
  if (Serial.available() <= 0) {
    return;
  }

  char command = Serial.read();

  if (command == 'r' || command == 'R') {
    leftPulses = 0;
    rightPulses = 0;
    Serial.println("pulse counts reset");
  }
}

void printStatusEvery500ms() {
  if (millis() - lastPrintTime < 500) {
    return;
  }

  lastPrintTime = millis();

  int leftState = digitalRead(LEFT_ENCODER_PIN);
  int rightState = digitalRead(RIGHT_ENCODER_PIN);

  Serial.print("Left: ");
  Serial.print(leftState == HIGH ? "HIGH" : "LOW");
  Serial.print(" | Left pulses: ");
  Serial.print(leftPulses);

  Serial.print(" || Right: ");
  Serial.print(rightState == HIGH ? "HIGH" : "LOW");
  Serial.print(" | Right pulses: ");
  Serial.println(rightPulses);
}
