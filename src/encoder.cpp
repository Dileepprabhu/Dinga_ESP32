#include <Arduino.h> // Include Arduino functions
#include <driver/pcnt.h> // Include Pulse Counter driver
#include "EncoderModule.h" // Include the encoder module header

// Encoder pulse input pins
#define ENCODER_PIN 4 // Connect to the encoder pulse output
#define ENCODER_PIN_2 5 // Connect to the second encoder pulse output

// Pulse Counter units and channels
#define PCNT_UNIT PCNT_UNIT_0 // Using Pulse Counter Unit 0
#define PCNT_UNIT_2 PCNT_UNIT_1 // Using Pulse Counter Unit 1
#define PCNT_H_LIM 10000      // High limit for counter
#define PCNT_L_LIM 0          // Low limit for counter
#define PCNT_H_LIM_2 10000    // High limit for second counter
#define PCNT_L_LIM_2 0        // Low limit for second counter

// Variables to store encoder tick counts
volatile int32_t encoderCount = 0;
volatile int32_t encoderCount2 = 0;

// Interrupt Service Routines for PCNT overflow/underflow
void IRAM_ATTR pcntISR(void *arg) {
    uint32_t status;
    pcnt_get_event_status(PCNT_UNIT, &status);

    if (status & PCNT_EVT_H_LIM) {
        encoderCount += PCNT_H_LIM; // Add high limit to count
    } else if (status & PCNT_EVT_L_LIM) {
        encoderCount += PCNT_L_LIM; // Subtract low limit from count
    }
}

void IRAM_ATTR pcntISR2(void *arg) {
    uint32_t status;
    pcnt_get_event_status(PCNT_UNIT_2, &status);

    if (status & PCNT_EVT_H_LIM) {
        encoderCount2 += PCNT_H_LIM_2; // Add high limit to count
    } else if (status & PCNT_EVT_L_LIM) {
        encoderCount2 += PCNT_L_LIM_2; // Subtract low limit from count
    }
}

// Setup functions for encoders
void setupEncoder() {
    pcnt_config_t pcntConfig = {
        .pulse_gpio_num = ENCODER_PIN,       // Pulse input GPIO number
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, // No control pin
        .lctrl_mode = PCNT_MODE_KEEP,       // Counter mode when control signal is low
        .hctrl_mode = PCNT_MODE_KEEP,       // Counter mode when control signal is high
        .pos_mode = PCNT_COUNT_INC,         // Count up on rising edge
        .neg_mode = PCNT_COUNT_DIS,         // Ignore falling edge
        .counter_h_lim = PCNT_H_LIM,        // High limit
        .counter_l_lim = PCNT_L_LIM,        // Low limit
        .unit = PCNT_UNIT,                  // PCNT unit number
        .channel = PCNT_CHANNEL_0           // PCNT channel
    };

    // Initialize Pulse Counter
    pcnt_unit_config(&pcntConfig);

    // Filter configuration (debouncing)
    pcnt_set_filter_value(PCNT_UNIT, 100); // Filter pulses shorter than 100 clock cycles
    pcnt_filter_enable(PCNT_UNIT);

    // Enable counter
    pcnt_counter_pause(PCNT_UNIT);
    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);

    // Enable interrupts for overflow and underflow
    pcnt_isr_service_install(0); // Install ISR service (0 for lowest priority)
    pcnt_isr_handler_add(PCNT_UNIT, pcntISR, NULL); // Add ISR handler
    pcnt_intr_enable(PCNT_UNIT);

    Serial.println("Encoder setup complete.");
}

void setupEncoder2() {
    pcnt_config_t pcntConfig = {
        .pulse_gpio_num = ENCODER_PIN_2,    // Pulse input GPIO number
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, // No control pin
        .lctrl_mode = PCNT_MODE_KEEP,       // Counter mode when control signal is low
        .hctrl_mode = PCNT_MODE_KEEP,       // Counter mode when control signal is high
        .pos_mode = PCNT_COUNT_INC,         // Count up on rising edge
        .neg_mode = PCNT_COUNT_DIS,         // Ignore falling edge
        .counter_h_lim = PCNT_H_LIM_2,      // High limit
        .counter_l_lim = PCNT_L_LIM_2,      // Low limit
        .unit = PCNT_UNIT_2,                // PCNT unit number
        .channel = PCNT_CHANNEL_0           // PCNT channel
    };

    // Initialize Pulse Counter
    pcnt_unit_config(&pcntConfig);

    // Filter configuration (debouncing)
    pcnt_set_filter_value(PCNT_UNIT_2, 100); // Filter pulses shorter than 100 clock cycles
    pcnt_filter_enable(PCNT_UNIT_2);

    // Enable counter
    pcnt_counter_pause(PCNT_UNIT_2);
    pcnt_counter_clear(PCNT_UNIT_2);
    pcnt_counter_resume(PCNT_UNIT_2);

    // Enable interrupts for overflow and underflow
    pcnt_isr_handler_add(PCNT_UNIT_2, pcntISR2, NULL); // Add ISR handler
    pcnt_intr_enable(PCNT_UNIT_2);

    Serial.println("Second encoder setup complete.");
}

// Functions to read encoder ticks non-blocking
int32_t readEncoder() {
    int16_t currentCount = 0;
    pcnt_get_counter_value(PCNT_UNIT, &currentCount);
    return encoderCount + currentCount; // Combine overflow count and current count
}

int32_t readEncoder2() {
    int16_t currentCount = 0;
    pcnt_get_counter_value(PCNT_UNIT_2, &currentCount);
    return encoderCount2 + currentCount; // Combine overflow count and current count
}
