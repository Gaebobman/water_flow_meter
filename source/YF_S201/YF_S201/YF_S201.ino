volatile int flow_frequency;
float l_minute;      // Litres per minute
unsigned char FLOW_PIN = 2;
unsigned long current_time;
unsigned long cloop_time;

void flow(){
  flow_frequency++;
}
void setup() {
  pinMode(FLOW_PIN, INPUT);
  Serial.begin(9600);
  attachInterrupt(0, flow, RISING);
  sei();
  cloop_time = millis();

}

void loop() {
  current_time = millis();
  // Every second, calculate and print litres/hour
  if(current_time == (cloop_time + 1000))  {
    cloop_time = current_time; // Updates cloopTime
    Serial.println(flow_frequency);
    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    // 60Hz 신호가 계속 잡혀서 -60하여 보상함
    l_minute = ((flow_frequency - 60) / 7.5); // (Pulse frequency x 1 min) / 7.5Q = flowrate in L/hour
    flow_frequency = 0; // Reset Counter
    Serial.print(l_minute, 3); // Print litres/min
    Serial.println(" L/min");
  }
}
