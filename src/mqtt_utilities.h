/**
 * @file mqtt_utilities.h
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#ifndef MQTT_UTILITIES_H
#define MQTT_UTILITIES_H

#include <PubSubClient.h>
#include "secret.h"
#include "globals.h"
#include "nvs_utilities.h"
#include "led_utilities.h"
#include <Arduino.h>


// MQTT broker settings
extern const char* mqtt_server; // MQTT broker address (using HiveMQ Cloud public broker)
extern const int mqtt_port; // MQTT broker port (TLS)
extern const char* mqtt_user; // MQTT username
extern const char* mqtt_password; // MQTT password

// Function declarations
void connectMqtt();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void checkConnectionAndPublish(const char* topic, const char* message);

#endif // MQTT_UTILITIES_H