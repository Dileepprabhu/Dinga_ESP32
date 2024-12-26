#include <Arduino.h>
#include "MotorControl.h"
#include "WiFiModule.h"
#include "EncoderModule.h" // Include the encoder module
#include <ArduinoJson.h> // Include ArduinoJson library

// Define GPIO pins for tasks
#define TASK1_GPIO 2   // Task1 toggles the internal LED (GPIO 2)
#define TASK2_GPIO 4  // Task2 toggles GPIO 26
#define TASK3_GPIO 15  // Task3 toggles GPIO 27

// Encoder variables
volatile uint32_t pulseCount = 0; // Counts encoder pulses
float distancePerPulse = 0.5;     // Distance per pulse in cm (configurable)
float totalDistance = 0.0;        // Total distance covered

// Task Handles
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
TaskHandle_t Task3Handle = NULL;

// Counters for each task
volatile uint32_t task1Counter = 0;
volatile uint32_t task2Counter = 0;
volatile uint32_t task3Counter = 0;

// External flags from WiFiModule.cpp
extern bool broadcastTaskCounter;
extern bool motorTestTriggered;

// Task Functions
void Task1(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms
    TickType_t xLastWakeTime = xTaskGetTickCount();  // Initialize wake time
    for (;;) {
        digitalWrite(TASK1_GPIO, !digitalRead(TASK1_GPIO)); // Toggle GPIO
        task1Counter++;                                    // Increment Task1 counter
        vTaskDelayUntil(&xLastWakeTime, xFrequency);       // Wait for next cycle
    }
}

void Task2(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 50ms
    TickType_t xLastWakeTime = xTaskGetTickCount();  // Initialize wake time
    for (;;) {
        digitalWrite(TASK2_GPIO, !digitalRead(TASK2_GPIO)); // Toggle GPIO
        task2Counter++;                                    // Increment Task2 counter
        vTaskDelayUntil(&xLastWakeTime, xFrequency);       // Wait for next cycle
    }
}

// Configuration to enable/disable device stats
bool show_device_stats = false; // Set to false to disable device stats logging

void sendJsonToSerialAndWebSocket(const JsonObject& json) {
    String jsonString;
    serializeJson(json, jsonString);
    Serial.println(jsonString);
    ws.textAll(jsonString);
}

void sendJsonToWebSocket(const JsonObject& json) {
    String jsonString;
    serializeJson(json, jsonString);
    ws.textAll(jsonString);
}

void Task3(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms
    TickType_t xLastWakeTime = xTaskGetTickCount();   // Initialize wake time
    for (;;) {
        digitalWrite(TASK3_GPIO, !digitalRead(TASK3_GPIO)); // Toggle GPIO
        task3Counter++;                                    // Increment Task3 counter

        // Print counters and optionally device stats
        // Serial.print("Task1 Counter: ");
        // Serial.print(task1Counter);
        // Serial.print(", Task2 Counter: ");
        // Serial.print(task2Counter);
        // Serial.print(", Task3 Counter: ");
        // Serial.println(task3Counter);

        if (show_device_stats) {
            // Create JSON object
            StaticJsonDocument<256> jsonDoc;
            JsonObject json = jsonDoc.to<JsonObject>();

            // Add device stats to JSON
            size_t freeHeap = xPortGetFreeHeapSize();
            size_t totalHeap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
            UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);

            json["freeHeap"] = freeHeap;
            json["totalHeap"] = totalHeap;
            json["usedHeapPercent"] = (totalHeap - freeHeap) * 100 / totalHeap;
            json["taskStackHighWaterMark"] = highWaterMark;

            // Send JSON to WebSocket
            sendJsonToWebSocket(json);

            // Print to Serial
            Serial.print("Free Heap: ");
            Serial.print(freeHeap);
            Serial.print(" bytes, Total Heap: ");
            Serial.print(totalHeap);
            Serial.print(" bytes, Used: ");
            Serial.print((totalHeap - freeHeap) * 100 / totalHeap);
            Serial.println("%");

            Serial.print("Task Stack High Water Mark: ");
            Serial.print(highWaterMark);
            Serial.println(" words");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Wait for next cycle
    }
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    setupWiFiModule(); // Initialize WiFi and WebSocket

    while (!Serial); // Wait for Serial Monitor to connect (useful for some setups)

    // Initialize GPIO pins as OUTPUT
    pinMode(TASK1_GPIO, OUTPUT); // Internal LED
    pinMode(TASK2_GPIO, OUTPUT);
    pinMode(TASK3_GPIO, OUTPUT);

    // Create RTOS Tasks
    xTaskCreate(Task1, "Task1", 1024, NULL, 2, &Task1Handle);
    xTaskCreate(Task2, "Task2", 1024, NULL, 2, &Task2Handle);
    xTaskCreate(Task3, "Task3", 1024, NULL, 2, &Task3Handle);

    // Initialize motors
    initializeMotors();

    // Initialize encoders
    setupEncoder();
    setupEncoder2();
}

void loop() {
    static bool testSequenceTriggered = false; // Flag to start the test sequence
    static bool testSequenceCompleted = false; // Flag to track if the test sequence is complete
    static bool messagePrinted = false; // Flag to ensure message is printed only once

    loopWiFiModule(); // Handle WiFi and WebSocket tasks

    if (Serial.available() > 0) { // Check for serial input
        char command = Serial.read(); // Read the incoming command

        switch (command) {
            case 'T': // Trigger test sequence
                if (!testSequenceTriggered) {
                    Serial.println("Starting Motor Test Sequence...");
                    testSequenceTriggered = true;
                    messagePrinted = false; // Reset the message flag
                }
                break;
            case 'W': // Move forward
                controlMotors(false, MotorDirection::FORWARD, 50); // Example: Move forward at 50% speed
                break;
            case 'S': // Move backward
                controlMotors(false, MotorDirection::REVERSE, 50); // Example: Move backward at 50% speed
                break;
            case 'A': // Turn left
                controlMotors(false, MotorDirection::LEFT, 50); // Example: Turn left at 50% speed
                break;
            case 'D': // Turn right
                controlMotors(false, MotorDirection::RIGHT, 50); // Example: Turn right at 50% speed
                break;
            case 'X': // Stop
                controlMotors(false, MotorDirection::STOP, 0); // Stop the motor
                break;
            default:
                break;
        }
    }

    if (testSequenceTriggered && !testSequenceCompleted) {
        // Run the test sequence and check if it's complete
        testSequenceCompleted = controlMotors(true, STOP, 0);
    }

    if (testSequenceCompleted && !messagePrinted) { // Ensure the message is printed only once
        Serial.println("Test Sequence Completed. Motors are now idle.");
        controlMotors(false, STOP, 0); // Stop the motors
        testSequenceTriggered = false; // Reset for potential future test sequences
        messagePrinted = true; // Mark that the message has been printed
    }

    // Task counter broadcasting logic
    if (broadcastTaskCounter) {
        StaticJsonDocument<256> jsonDoc;
        JsonObject json = jsonDoc.to<JsonObject>();

        json["task1Counter"] = task1Counter;
        json["task2Counter"] = task2Counter;
        json["task3Counter"] = task3Counter;

        sendJsonToWebSocket(json);

        // Print to Serial
        Serial.print("Task1 Counter: ");
        Serial.print(task1Counter);
        Serial.print(", Task2 Counter: ");
        Serial.print(task2Counter);
        Serial.print(", Task3 Counter: ");
        Serial.println(task3Counter);

        delay(1000); // Adjust the delay as needed
    }

    // Motor test sequence logic
    static bool testSequenceStarted = false; // Track if sequence announcement has been made

    if (motorTestTriggered && !testSequenceCompleted) {
        if (!testSequenceStarted){
            Serial.println("Starting Motor Test Sequence...");
            testSequenceStarted = true;
        }

        // Execute the motor test sequence
        testSequenceCompleted = controlMotors(true, STOP, 0);

        if (testSequenceCompleted) {
            Serial.println("Motor Test Sequence Completed.");
            controlMotors(false, STOP, 0); // Stop motors
            motorTestTriggered = false;   // Reset the flag for future triggers
            testSequenceStarted = false; // Reset the sequence announcement flag
        }
    }

    // Print encoder values every 500 ms
    static unsigned long lastPrintTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastPrintTime >= 500) {
        lastPrintTime = currentTime;
        int32_t ticks1 = readEncoder();
        int32_t ticks2 = readEncoder2();

        StaticJsonDocument<256> jsonDoc;
        JsonObject json = jsonDoc.to<JsonObject>();

        json["encoder1Ticks"] = ticks1;
        json["encoder2Ticks"] = ticks2;

        sendJsonToWebSocket(json);

        // Print to Serial
        Serial.printf("Encoder 1 ticks: %d\n", ticks1);
        Serial.printf("Encoder 2 ticks: %d\n", ticks2);
    }
}
