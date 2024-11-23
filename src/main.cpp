/**
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightSensor; // create instance of BH1750 class (light sensor)

// TODO: potřeba nastavit uživatelem přes MQTT broker (thresholds)
#define MAX_LUX 1000.0 // maximum light level in lux
#define MIN_LUX 100.0 // minimum light level in lux

const int LED_1 = 4; // LED connected to GPIO 4
const int LED_2 = 2; // LED connected to GPIO 2
const int freq = 5000; 
const int resolution = 8;
const int ledChannel1 = 0;
const int ledChannel2 = 1;

// TODO: linearizace úrovně svitu
float linearization(float lux) {
  return lux;
}

/**
 * Calculate brightness percentage based on light level
 */
float brightnessCalculation(float lux) {
  float percentage;
  if (lux < MIN_LUX) {
    percentage = 0;
  } else if (lux > MAX_LUX) {
    percentage = 100;
  } else {
    percentage = (lux - MIN_LUX) / (MAX_LUX - MIN_LUX) * 100;
  }
  return percentage;
}

// TODO: funkce an plynulý přechod mezi úrovněmi svitu

/**
 * Setup function runs once when the microcontroller is powered on
 */
void setup() {
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  Wire.begin(); // initialize I2C communication
  lightSensor.begin(); // initialize light sensor

  // initialize LED PWM
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(LED_1, ledChannel1);
  ledcAttachPin(LED_2, ledChannel2);
}

void loop() {
    float lux = lightSensor.readLightLevel(); // read light level in lux
    // DEBUG: print light level
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    // TODO: linearizace úrovně svitu
    float linearizedLux = linearization(lux);

    // calculate brightness percentage
    float brightness = brightnessCalculation(linearizedLux);
    Serial.print("Brightness: ");
    Serial.print(brightness);
    Serial.println("%");

    // calculate duty cycle for PWM (0-255 int values)
    int dutyCycle = brightness * 2.55; 
    dutyCycle = constrain(dutyCycle, 0, 255);

    // set the brightness of the LED (under the MIN_LUX the LED is off, over the MAX_LUX the LED is on full brightness)
    ledcWrite(ledChannel1, dutyCycle);
    ledcWrite(ledChannel2, dutyCycle);

    delay(1000); // wait a second between measurements
}