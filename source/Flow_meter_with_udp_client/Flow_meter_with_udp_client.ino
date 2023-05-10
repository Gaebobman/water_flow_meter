#include <SoftwareSerial.h>
#include "config.h"
// Define AT commands for ESP01 

#define ESP_RX 1
#define ESP_TX 0 // Define the RX and TX pins of ESP01 module
#define YF_PIN 2  // Define the interrupt pin of YF-S201 module

SoftwareSerial esp(ESP_RX, ESP_TX);  // Create a SoftwareSerial object
volatile int pulse_count = 0;        // Define a variable to store the pulse count
float flow_rate = 0;                 // Define a variable to store the flow rate
unsigned long last_time = 0;         // Define a variable to store the last time of calculation
unsigned long interval = 2000;       // Define a variable to store the interval of calculation (in milliseconds)
char buffer[100];                    // Define a buffer to store the AT commands and responses
unsigned long start;
void setup() {
  Serial.begin(9600);
  esp.begin(9600);
  // Initialize the interrupt pin of YF-S201 module
  pinMode(YF_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(YF_PIN), pulse, RISING);
  delay(3000);
  // Connect to the hotspot using AT commands
  sendCommand(CIPSTART_UDP, "", "\r\n", 'K', 2000);
  delay(3000);
  // RETURNS "CONNECT\nOK\r\n"
}

void loop() {
  // Calculate the flow rate every interval
  if (millis() - last_time >= interval) {
    // Update the last time of calculation
    last_time = millis();
    flow_rate = pulse_count / 7.5;  // flow rate (L/min) = pulse frequency (Hz) / 7.5
    pulse_count = 0;                // Reset the pulse count
    // Send the flow rate to the Linux server using AT commands
    // sendCommand("AT+CIPSEND=", "", "\r\n", '>', 2000); // TCP SEND
    sendCommand(CIPSEND_UDP, String(flow_rate).c_str(), "\r\n", 'K', 2000);  // UDP SEND
  }
}

// Interrupt function to count the pulses from YF-S201 module
void pulse() {
  pulse_count++;
}

// Function to send AT commands and check responses
void sendCommand(const char* prefix, const char* data, const char* suffix, char expected, unsigned long timeout) {
  // Clear the buffer
  memset(buffer, 0, sizeof(buffer));
  // Concatenate the prefix, data and suffix into the buffer
  if (strstr(prefix, "AT+CIPSEND")) {
    // Code block for CIPSEND command
    sprintf(buffer, "%s%d%s", prefix, strlen(data), suffix);  // Use sprintf to format the string with the data length
    Serial.print(buffer);
    // esp.print(buffer);   // 왜 이렇게 되는지 모르겠음 
    // Wait for '>' Before sending Water Usage data
    start = millis();
    while (esp.readBytesUntil(expected, buffer, sizeof(buffer)) != 0) {
      if (millis() - start >= timeout) {
        Serial.println("Timeout1");
        break;
      }
    }
    Serial.println(data);

    // If '>' is detected
    memset(buffer, 0, sizeof(buffer));
    strcat(buffer, data);
    strcat(buffer, suffix);
    esp.print(buffer);  // UDP SEND

    // Wait for response
    start = millis();
    while (esp.readBytesUntil(expected, buffer, sizeof(buffer)) != 0) {
      if (millis() - start >= timeout) {
        Serial.println("Timeout2");
        break;
      }
    }

  } else {
    // Code block for the other commands
    strcat(buffer, prefix);
    strcat(buffer, data);
    strcat(buffer, suffix);
    Serial.println(buffer);
    // Send the buffer to the ESP01 module
    esp.print(buffer);
    // Wait for the expected response or timeout
    memset(buffer, 0, sizeof(buffer));
    strcat(buffer, expected);
    // Serial.println(buffer);
    start = millis();
    while (esp.readBytesUntil(expected, buffer, sizeof(buffer)) != 0) {
      if (millis() - start >= timeout) {
        Serial.println("Timeout3");
        return;
      }
    }
  }
}