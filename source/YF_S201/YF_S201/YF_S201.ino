volatile int flow_frequency;
unsigned int l_hour;      // Litres per hour
unsigned char FLOW_PIN = 2;
unsigned long current_time;
unsigned long cloop_time;

void flow(){
  Serial.println("DETECTED");
  flow_frequency++;
}
void setup() {
  pinMode(FLOW_PIN, INPUT);
  digitalWrite(FLOW_PIN, HIGH);
  Serial.begin(9600);
  attachInterrupt(0, flow, RISING);
  sei();
  current_time = millis();
  cloop_time = current_time;

}

void loop() {
  current_time = millis();
  // Every second, calculate and print litres/hour
  if(current_time >= (cloop_time + 1000))  {
    detachInterrupt(0);
    cloop_time = current_time; // Updates cloopTime
    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
    flow_frequency = 0; // Reset Counter
    Serial.print(l_hour, DEC); // Print litres/hour
    Serial.println(" L/hour");
  }
}
