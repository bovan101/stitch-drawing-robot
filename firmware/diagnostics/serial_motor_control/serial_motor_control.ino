/*
  TB6612FNG serial motor control
  Board: Arduino Uno R4 WiFi

  Wiring:
    A channel = AO1/AO2 = right motor
    B channel = BO1/BO2 = left motor

    PWMA -> D5
    AIN2 -> D3
    AIN1 -> D2
    STBY -> D7
    BIN1 -> D4
    BIN2 -> D8
    PWMB -> D6

  Serial commands:
    f = forward
    b = backward
    l = turn left in place
    r = turn right in place
    s = stop
    t = short forward test
    0-9 = set speed level

  Use Arduino IDE Serial Monitor at 9600 baud.
*/

const int AIN1 = 2;
const int AIN2 = 3;
const int PWMA = 5;

const int BIN1 = 4;
const int BIN2 = 8;
const int PWMB = 6;

const int STBY = 7;
const int LED_PIN = LED_BUILTIN;

int baseSpeed = 220;

const int RIGHT_OFFSET = 35;
const int LEFT_OFFSET = 0;
const int KICK_SPEED = 255;
const int KICK_TIME_MS = 180;

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(STBY, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(STBY, HIGH);
  stopMotors();

  Serial.begin(9600);
  blinkStatus(3);
  delay(1000);
  printHelp();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    handleCommand(command);
  }
}

void handleCommand(char command) {
  if (command == '\n' || command == '\r' || command == ' ') {
    return;
  }

  if (command >= '0' && command <= '9') {
    setSpeedLevel(command - '0');
    return;
  }

  switch (command) {
    case 'f':
    case 'F':
      forward();
      Serial.println("forward");
      break;

    case 'b':
    case 'B':
      backward();
      Serial.println("backward");
      break;

    case 'l':
    case 'L':
      turnLeft();
      Serial.println("left");
      break;

    case 'r':
    case 'R':
      turnRight();
      Serial.println("right");
      break;

    case 's':
    case 'S':
      stopMotors();
      Serial.println("stop");
      break;

    case 't':
    case 'T':
      shortForwardTest();
      Serial.println("short forward test");
      break;

    case 'h':
    case 'H':
    case '?':
      printHelp();
      break;

    default:
      Serial.print("unknown command: ");
      Serial.println(command);
      printHelp();
      break;
  }
}

void setSpeedLevel(int level) {
  if (level == 0) {
    baseSpeed = 0;
    stopMotors();
  } else {
    baseSpeed = map(level, 1, 9, 100, 230);
  }

  Serial.print("base speed = ");
  Serial.println(baseSpeed);
  Serial.print("right speed = ");
  Serial.println(rightSpeed());
  Serial.print("left speed = ");
  Serial.println(leftSpeed());
}

void forward() {
  kickForward();
  rightForward(rightSpeed());
  leftForward(leftSpeed());
}

void backward() {
  kickBackward();
  rightBackward(rightSpeed());
  leftBackward(leftSpeed());
}

void turnLeft() {
  kickTurnLeft();
  rightForward(rightSpeed());
  leftBackward(leftSpeed());
}

void turnRight() {
  kickTurnRight();
  rightBackward(rightSpeed());
  leftForward(leftSpeed());
}

void rightForward(int value) {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, value);
}

void rightBackward(int value) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, value);
}

void leftForward(int value) {
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, value);
}

void leftBackward(int value) {
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, value);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void shortForwardTest() {
  kickForward();
  rightForward(rightSpeed());
  leftForward(leftSpeed());
  delay(700);
  stopMotors();
}

void kickForward() {
  rightForward(KICK_SPEED);
  leftForward(KICK_SPEED);
  delay(KICK_TIME_MS);
}

void kickBackward() {
  rightBackward(KICK_SPEED);
  leftBackward(KICK_SPEED);
  delay(KICK_TIME_MS);
}

void kickTurnLeft() {
  rightForward(KICK_SPEED);
  leftBackward(KICK_SPEED);
  delay(KICK_TIME_MS);
}

void kickTurnRight() {
  rightBackward(KICK_SPEED);
  leftForward(KICK_SPEED);
  delay(KICK_TIME_MS);
}

int rightSpeed() {
  return constrain(baseSpeed + RIGHT_OFFSET, 0, 255);
}

int leftSpeed() {
  return constrain(baseSpeed + LEFT_OFFSET, 0, 255);
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  f = forward");
  Serial.println("  b = backward");
  Serial.println("  l = turn left");
  Serial.println("  r = turn right");
  Serial.println("  s = stop");
  Serial.println("  t = short forward test");
  Serial.println("  0-9 = speed level");
  Serial.print("Base speed = ");
  Serial.println(baseSpeed);
  Serial.print("Right speed = ");
  Serial.println(rightSpeed());
  Serial.print("Left speed = ");
  Serial.println(leftSpeed());
}

void blinkStatus(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(120);
    digitalWrite(LED_PIN, LOW);
    delay(120);
  }
}
