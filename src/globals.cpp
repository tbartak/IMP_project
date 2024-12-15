/**
 * @file globals.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "globals.h"

WiFiClientSecure secureClient; // Create a secure client instance (for TLS)
PubSubClient client(secureClient); // Pass the secure client to PubSubClient (for TLS)
BH1750 lightSensor; // Create instance of BH1750 class (light sensor)
Preferences preferences; // Create instance of Preferences class (NVS)

float minLux; // minimum light level in lux
float maxLux; // maximum light level in lux
bool isNightMode; // night mode status