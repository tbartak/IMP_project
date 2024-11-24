/**
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <Preferences.h>

#define DEFAULT_MAX_LUX 1000.0 // maximum light level in lux
#define DEFAULT_MIN_LUX 100.0 // minimum light level in lux

// TODO: these values can be changed by user through MQTT
float MAX_LUX = DEFAULT_MAX_LUX; // maximum light level in lux
float MIN_LUX = DEFAULT_MIN_LUX; // minimum light level in lux

BH1750 lightSensor; // create instance of BH1750 class (light sensor)

const int LED_1 = 4; // LED connected to GPIO 4
const int LED_2 = 2; // LED connected to GPIO 2
const int freq = 5000; 
const int resolution = 8;
const int ledChannel1 = 0;
const int ledChannel2 = 1;

int previousDutyCycle = 0;

Preferences preferences; // create instance of Preferences class (NVS)

/**
 * Function for saving thresholds to NVS (non-volatile storage).
 * 
 * @param minLux - Minimum light level threshold in lux
 * @param maxLux - Maximum light level threshold in lux
 */
void saveLightThresholds(float minLux, float maxLux) {
  if (!preferences.begin("lightThresholds", false)) // try to open NVS 'lightThresholds' namespace in read-write mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for saving light thresholds");
    return;
  }
  
  preferences.begin("lightThresholds", false); 
  preferences.putFloat("minLux", minLux); // save value
  preferences.putFloat("maxLux", maxLux); // save value
  preferences.end(); // close

  Serial.println("Light thresholds saved");
}

/**
 * Function for loading thresholds from NVS (non-volatile storage).
 */
void loadLightThresholds() {
  if (!preferences.begin("lightThresholds", true)) // try to open NVS 'lightThresholds' namespace in read-only mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for loading light thresholds. Using default values.");
    return;
  }

  MIN_LUX = preferences.getFloat("minLux", DEFAULT_MIN_LUX); // get value or use default
  MAX_LUX = preferences.getFloat("maxLux", DEFAULT_MAX_LUX); // get value or use default
  preferences.end(); // close

  Serial.print("Loaded light thresholds: ");
  Serial.print(MIN_LUX);
  Serial.print(" - ");
  Serial.println(MAX_LUX);
}

// TODO: function to update light thresholds will be called after receiving MQTT message
/**
 * Function for updating light thresholds.
 * 
 * @param minLux - Minimum light level threshold in lux
 * @param maxLux - Maximum light level threshold in lux
 */
void updateLightThresholds(float minLux, float maxLux) {
  MIN_LUX = minLux; // internally update MIN_LUX
  MAX_LUX = maxLux; // internally update MAX_LUX
  saveLightThresholds(MIN_LUX, MAX_LUX); // save new thresholds to NVS for future use
}

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

  loadLightThresholds(); // load light thresholds from NVS

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

    // fade between previous and current duty cycle // TODO: could think of a better option without the need to have delay in the function directly
    brightnessFade(previousDutyCycle, currentDutyCycle);

    // save current duty cycle for next iteration
    previousDutyCycle = currentDutyCycle;

    // delay(255); // wait 255ms between measurements // prozatím nahrazeno delayem v brightnessFade
}