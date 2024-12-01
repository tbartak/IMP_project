/**
 * @file wifi_utilities.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef WIFI_UTILITIES_H
#define WIFI_UTILITIES_H

#include <WiFi.h>
#include "secret.h"
#include "led_utilities.h"
#include <Arduino.h>

// WiFi settings
extern const char* ssid; // WiFi SSID
extern const char* password; // WiFi password

// Function declarations
void setup_wifi(const char* ssid, const char* password);

#endif // WIFI_UTILITIES_H