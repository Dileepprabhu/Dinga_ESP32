#ifndef WIFIMODULE_H
#define WIFIMODULE_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "MotorControl.h" // Include MotorControl header

// Expose WebSocket server object
extern AsyncWebSocket ws;

// Function declarations
void setupWiFiModule();
void loopWiFiModule();

extern bool broadcastTaskCounter;
extern bool motorTestTriggered;

#endif // WIFIMODULE_H