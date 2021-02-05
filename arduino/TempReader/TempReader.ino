// DHT11 lib

/*
 * analog (resistance)  > temp C
 * 604 (1722.63) - 609 (1757.86) = 25.6 C
 * 577 (1549) ==> 29 C (averaged)
 * 408 (792.78) - 413 (809.07) = 45.5 C
 * 345 (608.08) - 350.00 (621.47) = 52.5 C
 * 242 (370.28) - 244 (374.30) = 66.5 C
 * 230 (346.60) - 235 (356.38) = 71.5 C
 */
#include <SimpleDHT.h>
SimpleDHT11 dht11(8); // PIN 8

// What pin to connect the sensor to
#define THERMISTORPIN A0 

// Other resistor before sensor
#define SERIESRESISTOR 1195

// resistance at 25 degrees C
// #define THERMISTORNOMINAL 1722      

// temp. for nominal resistance (almost always 25 C)
// #define TEMPERATURENOMINAL 26

// #define THERMISTORNOMINAL 1068
// #define TEMPERATURENOMINAL 37

#define THERMISTORNOMINAL 1549
#define TEMPERATURENOMINAL 29


// #define BETA COEFFICIENT (usually 3000 - 4000)
// https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
#define BCOEFFICIENT 3888

void setup(void) {
  Serial.begin(115200);
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