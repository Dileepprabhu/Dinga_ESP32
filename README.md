Maze Solver Robot with ESP32
Project Overview
This project involves building a maze solver robot using an ESP32 microcontroller. The robot is equipped with motors, encoders, a Time-of-Flight (TOF) sensor, and a WebSocket server for real-time data streaming and control. The robot uses FreeRTOS for multitasking, allowing it to handle multiple tasks concurrently.

Project Structure
Components
1. Motor Control
The motor control module handles the initialization and control of the motors. It includes functions to set motor speed and direction.

Initialization: initializeMotors()
Set Motor Speed and Direction: setMotorSpeed(uint8_t dutyCycle, MotorDirection dir)
Control Motors: controlMotors(bool runTestSequence, MotorDirection direction, uint8_t speed)
2. Encoder Module
The encoder module handles the setup and reading of encoder ticks. It includes interrupt service routines (ISRs) for handling encoder overflows and functions to read encoder values.

Setup Encoders: setupEncoder(), setupEncoder2()
Read Encoder Values: readEncoder(), readEncoder2()
3. WiFi and WebSocket Module
The WiFi and WebSocket module sets up the ESP32 as an access point and handles WebSocket communication for real-time data streaming and control.

Setup WiFi and WebSocket: setupWiFiModule()
Handle WebSocket Events: onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
Loop WiFi Module: loopWiFiModule()
4. Main Application
The main application initializes the components and creates FreeRTOS tasks for handling different functionalities.

Setup Function: setup()
Main Loop: loop()
FreeRTOS Tasks: Task1(void *pvParameters), Task2(void *pvParameters), Task3(void *pvParameters)
