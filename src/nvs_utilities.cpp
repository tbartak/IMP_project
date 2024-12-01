/**
 * @file nvs_utilities.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "nvs_utilities.h"


/**
 * Function for saving thresholds to NVS (non-volatile storage).
 * 
 * @param newMinLux - Minimum light level threshold in lux
 * @param newMaxLux - Maximum light level threshold in lux
 */
void saveLightThresholds(float newMinLux, float newMaxLux) {
  if (!preferences.begin("lightThresholds", false)) // try to open NVS 'lightThresholds' namespace in read-write mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for saving light thresholds");
    return;
  }
  
  preferences.putFloat("minLux", newMinLux); // save value
  preferences.putFloat("maxLux", newMaxLux); // save value
  preferences.end(); // close

  minLux = newMinLux; // update global variable
  maxLux = newMaxLux; // update global variable

  Serial.println("Light thresholds have been successfully updated and saved.");
}

/**
 * Function for loading thresholds from NVS (non-volatile storage).
 * 
 */
void loadLightThresholds() {
  if (!preferences.begin("lightThresholds", true)) // try to open NVS 'lightThresholds' namespace in read-only mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for loading light thresholds. Using default values.");
    return;
  }

  minLux = preferences.getFloat("minLux", DEFAULT_MIN_LUX); // get value or use default
  maxLux = preferences.getFloat("maxLux", DEFAULT_MAX_LUX); // get value or use default
  preferences.end(); // close

  Serial.print("Loaded light thresholds: ");
  Serial.print(minLux);
  Serial.print(" - ");
  Serial.println(maxLux);
}

/**
 * Function for saving configuration of the LEDs to NVS (non-volatile storage).
 * 
 * @param isNightMode - Night mode state
 */
void saveConfig(bool newMode) {
  if (!preferences.begin("config", false)) // try to open NVS 'config' namespace in read-write mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for saving configuration of the LEDs.");
    return;
  }

  preferences.putBool("isNightMode", newMode); // save value
  preferences.end(); // close

  Serial.println("Configuration of the LEDs has been successfully updated and saved.");
}

/**
 * Function for loading configuration of the LEDs from NVS (non-volatile storage).
 * 
 */
void loadConfig() {
  if (!preferences.begin("config", true)) // try to open NVS 'config' namespace in read-only mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for loading configuration of the LEDs. Using default values.");
    return;
  }

  isNightMode = preferences.getBool("isNightMode", false); // get value or use default
  preferences.end(); // close

  Serial.print("Loaded configuration of the LEDs: ");
  Serial.println(isNightMode ? "night" : "day");
}

/**
 * Load all necessary data from NVS (non-volatile storage).
 */
void loadAllData() {
  loadLightThresholds(); // load light thresholds
  loadConfig(); // load configuration of the LEDs
}