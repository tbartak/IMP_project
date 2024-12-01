/**
 * @file globals.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Preferences.h>

extern WiFiClientSecure secureClient; // Create a secure client instance (for TLS)
extern PubSubClient client; // Pass the secure client to PubSubClient (for TLS)
extern BH1750 lightSensor; // create instance of BH1750 class (light sensor)
extern Preferences preferences; // create instance of Preferences class (NVS)

#define DEFAULT_MIN_LUX 100.0 // default minimum light level in lux
#define DEFAULT_MAX_LUX 1000.0 // default maximum light level in lux

extern float minLux; // minimum light level in lux
extern float maxLux; // maximum light level in lux
extern bool isNightMode; // night mode status

#endif // GLOBALS_H