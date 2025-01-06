#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// Enum for Motor Directions
enum MotorDirection { FORWARD, REVERSE, LEFT, RIGHT, STOP };

// Structure to define motor parameters
struct MotorStatus {
    uint8_t motor1Speed;
    uint8_t motor2Speed;
    MotorDirection direction;
};

// Function Prototypes
void initializeMotors();
bool controlMotors(bool runTestSequence = false, MotorDirection direction = STOP, uint8_t speed = 0);
MotorStatus getMotorStatus(); // New function to get motor status

#endif // MOTORCONTROL_H
