#include "WiFiModule.h"
#include "MotorControl.h"

// Access Point credentials
const char* ssid = "ESP32_AP";
const char* password = "12345678"; // Minimum 8 characters

// Create an Async WebSocket server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint "/ws"

// Sample data (can be replaced with sensor readings)
int parameter = 0;

// Global flags
bool broadcastTaskCounter = false;
bool motorTestTriggered = false;

// WebSocket event handler
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Client connected: %u\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Client disconnected: %u\n", client->id());
  } else if (type == WS_EVT_DATA) {
    String receivedData = String((char *)data).substring(0, len);
    Serial.printf("Data received from client %u: %s\n", client->id(), receivedData.c_str());

    // Process specific commands
    if (receivedData == "Task_counter_start") {
      broadcastTaskCounter = true;
      Serial.println("Task counter broadcasting started.");
    } else if (receivedData == "Task_counter_stop") {
      broadcastTaskCounter = false;
      Serial.println("Task counter broadcasting stopped.");
    } else if (receivedData == "Motor_Test_Start") {
      motorTestTriggered = true;
      Serial.println("Motor test sequence triggered.");
    } else {
      // Handle motor control commands
      char command = receivedData.charAt(0);
      switch (command) {

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
  }
}

void setupWiFiModule() {
  Serial.begin(115200);

  // Set up the ESP32 as an Access Point
  WiFi.softAP(ssid, password);
  Serial.println("ESP32 Access Point started!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP()); // Print the ESP32's IP address (usually 192.168.4.1)

  // Attach WebSocket event handler
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Start the server
  server.begin();
  Serial.println("WebSocket server started!");
}

void loopWiFiModule() {
  // Broadcast data to all connected WebSocket clients
  parameter++;
  String message = "Heartbeat_count: " + String(parameter);
  ws.textAll(message);
  Serial.println("Broadcasting: " + message);

  delay(250); // Delay between broadcasts
}