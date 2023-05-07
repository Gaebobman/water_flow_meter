#include <SoftwareSerial.h>
#include "config.h" 
// Define the SSID and password of the hotspot,
// IP address and port number of the Linux server

#define ESP_RX 7
#define ESP_TX 6  // Define the RX and TX pins of ESP01 module
#define YF_PIN 2  // Define the interrupt pin of YF-S201 module

SoftwareSerial esp(ESP_RX, ESP_TX); // Create a SoftwareSerial object
volatile int pulse_count = 0; // Define a variable to store the pulse count
float flow_rate = 0;          // Define a variable to store the flow rate
unsigned long last_time = 0;  // Define a variable to store the last time of calculation
unsigned long interval = 1000;// Define a variable to store the interval of calculation (in milliseconds)
char buffer[100];             // Define a buffer to store the AT commands and responses

void setup() {
  Serial.begin(9600);
  esp.begin(9600);
  // Initialize the interrupt pin of YF-S201 module
  pinMode(YF_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(YF_PIN), pulse, RISING);
  // Connect to the hotspot using AT commands
  Serial.println("INIT");
  sendCommand(CWJAP_prefix , password , "\r\n", "OK", 10000);
  // Connect to the Linux server using AT commands
  sendCommand(CIPSTART_prefix, server_port, "\r\n", "OK", 10000);
}

void loop() {
  // Calculate the flow rate every interval
  if (millis() - last_time >= interval) {
    // Update the last time of calculation
    last_time = millis();
    flow_rate = pulse_count / 7.5;      // flow rate (L/min) = pulse frequency (Hz) / 7.5
    pulse_count = 0;  // Reset the pulse count
    // Print the flow rate to the serial monitor
    Serial.print("Flow rate: ");
    Serial.print(flow_rate);
    Serial.println(" L/min");
    // Send the flow rate to the Linux server using AT commands
    sendCommand("AT+CIPSEND=", String(flow_rate).c_str(), "\r\n", ">", 1000);
    sendCommand("","", "\r\n", "SEND OK", 1000);
  }
}

// Interrupt function to count the pulses from YF-S201 module
void pulse() {
  pulse_count++;
}

// Function to send AT commands and check responses
void sendCommand(const char* prefix, const char* data, const char* suffix, const char* expected, unsigned long timeout) {
  // Clear the buffer
  memset(buffer, 0, sizeof(buffer));

  // Concatenate the prefix, data and suffix into the buffer
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
  unsigned long start = millis();

  while (esp.find(buffer) == false) {
    if (millis() - start >= timeout) {
      Serial.println("Timeout");
      break;
    }
  }
}