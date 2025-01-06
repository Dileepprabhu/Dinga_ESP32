#include "MotorControl.h"

// Define GPIO pins for motors (H-Bridge control)
#define MOTOR1_A 32
#define MOTOR1_B 33
#define MOTOR2_A 26
#define MOTOR2_B 25

// Global variable to store motor status
MotorStatus motorStatus = {0, 0, STOP};

// Initialize Motors
void initializeMotors() {
    pinMode(MOTOR1_A, OUTPUT);
    pinMode(MOTOR1_B, OUTPUT);
    pinMode(MOTOR2_A, OUTPUT);
    pinMode(MOTOR2_B, OUTPUT);

    // Ensure motors are stopped initially
    digitalWrite(MOTOR1_A, LOW);
    digitalWrite(MOTOR1_B, LOW);
    digitalWrite(MOTOR2_A, LOW);
    digitalWrite(MOTOR2_B, LOW);
}

// Calculate PWM Duty Cycle
uint8_t calculateDutyCycle(uint8_t speedPercentage) {
    return map(speedPercentage, 0, 100, 0, 255); // Map speed (0–100%) to PWM (0–255)
}

// Set Motor Speed and Direction
void setMotorSpeed(uint8_t dutyCycle, MotorDirection dir) {
    switch (dir) {
        case FORWARD:
            motorStatus.motor1Speed = dutyCycle;
            motorStatus.motor2Speed = dutyCycle;
            motorStatus.direction = dir;
            analogWrite(MOTOR1_A, dutyCycle);
            analogWrite(MOTOR1_B, 0);
            analogWrite(MOTOR2_A, dutyCycle);
            analogWrite(MOTOR2_B, 0);
            break;

        case REVERSE:
            motorStatus.motor1Speed = dutyCycle;
            motorStatus.motor2Speed = dutyCycle;
            motorStatus.direction = dir;
            analogWrite(MOTOR1_A, 0);
            analogWrite(MOTOR1_B, dutyCycle);
            analogWrite(MOTOR2_A, 0);
            analogWrite(MOTOR2_B, dutyCycle);
            break;

        case LEFT:
            motorStatus.motor1Speed = 0;
            motorStatus.motor2Speed = dutyCycle;
            motorStatus.direction = dir;
            analogWrite(MOTOR1_A, 0);        // Stop left motor
            analogWrite(MOTOR1_B, 0);
            analogWrite(MOTOR2_A, dutyCycle); // Run right motor forward
            analogWrite(MOTOR2_B, 0);
            break;

        case RIGHT:
            motorStatus.motor1Speed = dutyCycle;
            motorStatus.motor2Speed = 0;
            motorStatus.direction = dir;
            analogWrite(MOTOR1_A, dutyCycle); // Run left motor forward
            analogWrite(MOTOR1_B, 0);
            analogWrite(MOTOR2_A, 0);        // Stop right motor
            analogWrite(MOTOR2_B, 0);
            break;

        case STOP:
        default:
            motorStatus.motor1Speed = 0;
            motorStatus.motor2Speed = 0;
            motorStatus.direction = dir;
            analogWrite(MOTOR1_A, 0); // Stop both motors
            analogWrite(MOTOR1_B, 0);
            analogWrite(MOTOR2_A, 0);
            analogWrite(MOTOR2_B, 0);
            break;
    }
}

// Function to get motor status
MotorStatus getMotorStatus() {
    return motorStatus;
}

// Control Motors
bool controlMotors(bool runTestSequence, MotorDirection direction, uint8_t speed) {
    if (runTestSequence) {
        static uint32_t lastUpdate = millis();
        static int state = 0;

        // Define test directions and speeds
        MotorDirection directions[] = {FORWARD, FORWARD, REVERSE, REVERSE, RIGHT, RIGHT, LEFT, LEFT, STOP};
        uint8_t speeds[] = {20, 80, 20, 80, 20, 80, 20, 80, 0};

        if (millis() - lastUpdate > 1000) { // Update every 5 seconds
            direction = directions[state];
            speed = speeds[state];
            Serial.print("Test Sequence - Direction: ");
            switch (direction) {
                case FORWARD: Serial.println("FORWARD"); break;
                case REVERSE: Serial.println("REVERSE"); break;
                case RIGHT: Serial.println("RIGHT"); break;
                case LEFT: Serial.println("LEFT"); break;
                case STOP: Serial.println("STOP"); break;
            }
            Serial.print("Speed: ");
            Serial.print(speed);
            Serial.println("%");
            // Set motor speed and direction
            uint8_t dutyCycle = calculateDutyCycle(speed);
            setMotorSpeed(dutyCycle, direction);

            // Move to the next state
            state++;
            lastUpdate = millis();

                        // Check if the sequence is complete
            if (state >= (sizeof(directions) / sizeof(directions[0]))) 
            {
                state = 0; // Reset state for future test runs
                return true; // Indicate that the test sequence is complete
            }
        }

        return false; // Test sequence is still running
    } else {
        uint8_t dutyCycle = calculateDutyCycle(speed);
        setMotorSpeed(dutyCycle, direction);
        Serial.print("Manual Control - Direction: ");
        switch (direction) {
            case FORWARD: Serial.println("FORWARD"); break;
            case REVERSE: Serial.println("REVERSE"); break;
            case RIGHT: Serial.println("RIGHT"); break;
            case LEFT: Serial.println("LEFT"); break;
            case STOP: Serial.println("STOP"); break;
        }
        Serial.print("Speed: ");
        Serial.print(speed);
        Serial.println("%");
        return false; // No test sequence
    }
}
