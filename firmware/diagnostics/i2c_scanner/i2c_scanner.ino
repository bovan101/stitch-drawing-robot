/*
  I2C scanner
  Board: Arduino Uno R4 WiFi

  Use this to check whether the ToF sensor appears on the I2C bus.

  Wiring:
    ToF VIN -> Arduino 5V
    ToF GND -> Arduino GND
    ToF SDA -> Arduino SDA
    ToF SCL -> Arduino SCL

  Serial Monitor:
    9600 baud
*/

#include <Wire.h>

void setup() {
  Serial.begin(9600);
  delay(1000);

  Wire.begin();

  Serial.println("I2C scanner ready.");
}

void loop() {
  int deviceCount = 0;

  Serial.println("Scanning...");

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      deviceCount++;
    }
  }

  if (deviceCount == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s).");
  }

  Serial.println();
  delay(2000);
}
