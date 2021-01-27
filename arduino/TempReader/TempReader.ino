// DHT11 lib
#include <SimpleDHT.h>
SimpleDHT11 dht11(8); // PIN 8

// What pin to connect the sensor to
#define THERMISTORPIN A0 

// Other resistor before sensor
#define SERIESRESISTOR 1195

// resistance at 25 degrees C
#define THERMISTORNOMINAL 1202      

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 23

// #define BCOEFFICIENT (usually 3000 - 4000)
#define BCOEFFICIENT 3000

void setup(void) {
  Serial.begin(9600);
}
 
void loop(void) {

  Serial.println("-------------");

  float reading;
  reading = analogRead(THERMISTORPIN);
 
  float ohm;
  ohm = SERIESRESISTOR / ((1023 / reading)  - 1);

  float temp;
  temp = (1.0 / ((log(ohm / THERMISTORNOMINAL) / BCOEFFICIENT) + 1.0 / (TEMPERATURENOMINAL + 273.15))) - 273.15 ;

  Serial.print("Analog reading "); 
  Serial.println(reading);

  Serial.print("Thermistor resistance "); 
  Serial.println(ohm);
  
  Serial.print("Water temp: "); 
  Serial.println(temp);


  // read DHT11
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }
  
  Serial.print("  Air temp: ");
  Serial.println((int)temperature);
  Serial.print("  Air humidity: "); 
  Serial.println((int)humidity);
  
  // DHT11 sampling rate is 1HZ.
  delay(1500);

}