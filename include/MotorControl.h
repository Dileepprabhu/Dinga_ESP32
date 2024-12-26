#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// Enum for Motor Directions
enum MotorDirection { FORWARD, REVERSE, LEFT, RIGHT, STOP };

// Function Prototypes
void initializeMotors();
bool controlMotors(bool runTestSequence = false, MotorDirection direction = STOP, uint8_t speed = 0);

#endif // MOTORCONTROL_H
