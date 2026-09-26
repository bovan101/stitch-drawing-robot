# Development and Testing

## Development sequence

The project was developed as a set of small hardware tests before the final controller was assembled.

1. Test each motor-driver channel and confirm direction.
2. Add serial commands for forward, reverse, turning, stopping, and speed changes.
3. Read one encoder, then both encoders, and compare pulse counts.
4. Scan the I2C bus and test the ToF distance sensor separately.
5. Map distance readings to four behaviour states.
6. Combine the sensor, motors, encoders, and serial controls in the final sketch.

## Final behaviour thresholds

The final controller uses these boundaries:

- More than 700 mm: `APPROACH`
- 550 to 700 mm: `HESITATE`
- 200 to 550 mm: `NEGOTIATE`
- Less than 200 mm: `REFUSE`

The controller applies an exponential moving average with an alpha value of `0.25` to reduce rapid changes in the raw distance reading.

## Motor calibration

The final code uses a base speed of `150`. It adds a PWM offset of `35` to the right motor and leaves the left offset at `0`. The value is limited to the Arduino PWM range from 0 to 255.

The encoder counts help compare the wheels during testing. The current controller does not automatically correct the motor speeds from encoder feedback.

## Test stages

### Mechanical test

The robot was pushed by hand before powering the motors. This checked whether the rear pen could draw a stable line and whether the pen, wires, wheels, and encoder disks interfered with each other.

### Motor and encoder test

The motors were tested at low speeds while encoder pulses were printed to the serial monitor. This confirmed that both wheels moved and showed the difference between them.

### Distance behaviour test

A hand, card, or body position was placed in front of the ToF sensor. Changes in distance triggered the four movement behaviours and produced different marks on the paper.

## Observed limitations

- Front-only sensing limits interaction from the sides and rear.
- Paper friction and pen pressure change the trace.
- Motor differences affect movement symmetry.
- Encoder polling is suitable for monitoring but can miss fast pulses.
