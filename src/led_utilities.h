/**
 * @file led_utilities.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef LED_UTILITIES_H
#define LED_UTILITIES_H

#include <Arduino.h>
#include <math.h>
#include "globals.h"

const int LED_1 = 25; // LED connected to GPIO 25
const int LED_2 = 26; // LED connected to GPIO 26
const int freq = 5000;
const int resolution = 8;
const int ledChannel1 = 0;
const int ledChannel2 = 1;

// Function declarations
void setupLEDs();
float gammaLinearization(float percentage);
float brightnessCalculation(float lux);
void brightnessFade(int from, int to);
void signalingLED(int count);

#endif // LED_UTILITIES_H