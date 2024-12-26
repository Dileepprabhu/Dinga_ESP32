
#ifndef ENCODERMODULE_H
#define ENCODERMODULE_H

#include <Arduino.h>
#include <driver/pcnt.h>

// Function declarations
void setupEncoder();
void setupEncoder2();
int32_t readEncoder();
int32_t readEncoder2();

#endif // ENCODERMODULE_H