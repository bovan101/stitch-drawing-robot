# Pin Map

## TB6612FNG motor driver

| Driver pin | Arduino pin | Function |
| --- | --- | --- |
| AIN1 | D2 | Right motor direction |
| AIN2 | D3 | Right motor direction |
| PWMA | D5 | Right motor PWM |
| BIN1 | D4 | Left motor direction |
| BIN2 | D8 | Left motor direction |
| PWMB | D6 | Left motor PWM |
| STBY | D7 | Driver standby control |

Channel A connects to the right motor through AO1 and AO2. Channel B connects to the left motor through BO1 and BO2.

## Wheel encoders

| Encoder connection | Arduino connection |
| --- | --- |
| Left encoder DO | D9 |
| Right encoder DO | D10 |
| VCC | 5V |
| GND | GND |
| AO | Not connected |

## ToF distance sensor

| Sensor connection | Arduino connection |
| --- | --- |
| VIN | 5V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| INT / SHUT | Not connected |

The sensor communicates through I2C. The detected address is `0x29`.

## Power note

The motor driver uses a separate 4AA battery pack for motor power. The Arduino and motor supply share a common ground.
