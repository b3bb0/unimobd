// PINout: https://components101.com/a3144-hall-effect-sensor

// ALSO CHECK https://www.youtube.com/watch?v=u2uJMJWsfsg

  float hall_count = 1.0;
unsigned long  start = micros();
float hall_thresh = 100.0;
unsigned long  last = micros();


// Install Pin change interrupt for a pin, can be called multiple times

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}
 
// Use one Routine to handle each group
ISR (PCINT2_vect) {

  if (( micros() - last ) < 600) return;
  if (digitalRead(4) != LOW) return;
  
  hall_count+=1.0;
  
  if (hall_count>=hall_thresh) countRPMs();
}  
 
void setup() {  

  Serial.begin(115200);
  digitalWrite(4,HIGH);  // pinMode( ,INPUT) is default
 
  // enable interrupt for pin...
  pciSetup(4);
}
 
 
void loop() {
  // Nothing needed
}

void countRPMs() {
  // print information about Time and RPM
  unsigned long end_time = micros();
  float time_passed = ((end_time-start)/1000000.0);
  Serial.print("Time Passed: ");
  Serial.print(time_passed);
  Serial.println("s");
  float rpm_val = (hall_count/time_passed)*60.0;
  Serial.print(rpm_val);
  Serial.println(" RPM");
  
  hall_count = 1.0;
  start = micros();
}