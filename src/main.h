/**
 * @file main.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef MAIN_H
#define MAIN_H

#include "globals.h"
#include "led_utilities.h"
#include "nvs_utilities.h"
#include "wifi_utilities.h"
#include "mqtt_utilities.h"
#include <Arduino.h>
#include "secret.h"

int previousDutyCycle = 0; // previous duty cycle for PWM

// Function declarations
void setup();
void loop();

#endif // MAIN_H