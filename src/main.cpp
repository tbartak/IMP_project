/**
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightSensor; // create instance of BH1750 class (light sensor)

/**
 * Setup function runs once when the microcontroller is powered on
 */
void setup() {
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  Wire.begin(); // initialize I2C communication
  lightSensor.begin(); // initialize light sensor
  pinMode(4, OUTPUT); // LED connected to GPIO 4
  pinMode(2, OUTPUT);  // LED connected to GPIO 2
}

void loop() {
    float lux = lightSensor.readLightLevel(); // read light level in lux
    // DEBUG: print light level
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    // TEST: Turn on if light level is above 100 lux
    if (lux > 100) {
      Serial.println("Light is on");
      digitalWrite(4, HIGH);
      digitalWrite(2, HIGH);
    } else {
      digitalWrite(4, LOW);
      digitalWrite(2, LOW);
    }

    delay(500); // wait half a second between measurements
}