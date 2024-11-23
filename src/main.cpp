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

int previousDutyCycle = 0;

// TODO: linearizace úrovně svitu
float linearization(float lux) {
  return lux;
}

/**
 * Calculate brightness percentage based on light level
 * 
 * @param lux - Light level in lux
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

/**
 * Fade smoothly between two brightness levels
 * 
 * @param from - Starting brightness level
 * @param to - Target brightness level
 */
void brightnessFade(int from, int to) {
  int steps = abs(from - to); // steps between current and target duty cycle
  int duration = 255; // duration of the fade in ms

  if (steps == 0) {
    delay(duration); // wait 255ms between measurements
    return;
  }

  int delayTime = duration / steps; // difference between each step in ms

  if (from < to) {
    for (int i = from; i <= to; i++) {
      ledcWrite(ledChannel1, i);
      ledcWrite(ledChannel2, i);
      delay(delayTime); // for smooth transitions based on the number of steps
    }
  } else {
    for (int i = from; i >= to; i--) {
      ledcWrite(ledChannel1, i);
      ledcWrite(ledChannel2, i);
      delay(delayTime); // for smooth transitions based on the number of steps
    }
  }
}

/**
 * Setup function runs once when the micro-controller is powered on
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
    int currentDutyCycle = brightness * 2.55; 
    currentDutyCycle = constrain(currentDutyCycle, 0, 255);

    // fade between previous and current duty cycle
    brightnessFade(previousDutyCycle, currentDutyCycle);

    // save current duty cycle for next iteration
    previousDutyCycle = currentDutyCycle;

    // delay(255); // wait 255ms between measurements // prozatím nahrazeno delayem v brightnessFade
}