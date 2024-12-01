/**
 * @file nvs_utilities.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef NVS_UTILITIES_H
#define NVS_UTILITIES_H

#include <Preferences.h>
#include "globals.h"
#include <Arduino.h>

// Function declarations
void saveLightThresholds(float newMinLux, float newMaxLux);
void loadLightThresholds();
void saveConfig(bool newMode);
void loadConfig();
void loadAllData();

#endif // NVS_UTILITIES_H