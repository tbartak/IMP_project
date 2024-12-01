/**
 * @file led_utilities.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "led_utilities.h"

/**
 * Function for setting up the LED PWM channels.
 */
void setupLEDs() {
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(LED_1, ledChannel1);
  ledcAttachPin(LED_2, ledChannel2);
}

/**
 * Linearize the brightness of the LEDs using gamma correction.
 * Our eyes perceive light intensity in a non-linear way, so we need to correct the brightness to make it linear. 
 * source: https://learn.adafruit.com/led-tricks-gamma-correction/the-issue
 * source: https://electricfiredesign.com/2022/11/14/gamma-correction-for-led-lighting/
 * 
 * @param percentage - Brightness percentage
 */
float gammaLinearization(float percentage) {
  const float gamma = 2.2;
  float correctedBrightness = pow(percentage / 100.0, gamma) * 100.0;
  return correctedBrightness;
}

/**
 * Calculate brightness percentage based on light level
 * 
 * @param lux - Light level in lux
 * @param minLux - Minimum light level threshold in lux
 * @param maxLux - Maximum light level threshold in lux
 * @param isNightMode - Night mode status
 */
float brightnessCalculation(float lux) {
  float percentage;
  if (lux < minLux) {
    percentage = 0.0;
  } else if (lux > maxLux) {
    percentage = 100.0;
  } else {
    percentage = (lux - minLux) / (maxLux - minLux) * 100.0;
  }

  if (isNightMode) {
    percentage = 100.0 - percentage; // invert the percentage, if night mode is enabled
  }

  percentage = gammaLinearization(percentage);

  return percentage;
}

/**
 * Fade smoothly between two brightness levels, instead of suddenly changing the brightness from one level to another. 
 * To get rid of an effect, where for example the light level is changing from 0 to 100% in one step (aka using the flashlight on the sensor/turning it off).
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
 * Utility function for signaling the user with the LEDs.
 * 
 * @param count - Number of times to signal
 */
void signalingLED(int count) {
  for (int i = 0; i < count; i++) {
    brightnessFade(0, 255);
    brightnessFade(255, 0);
  }
}