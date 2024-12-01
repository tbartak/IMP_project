/**
 * @file mqtt_utilities.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "mqtt_utilities.h"

const char *mqtt_server = MQTT_SERVER; // MQTT broker address (using HiveMQ Cloud public broker)
const int mqtt_port = MQTT_PORT; // MQTT broker port (TLS)
const char *mqtt_user = MQTT_USER; // MQTT username
const char *mqtt_password = MQTT_PASSWORD; // MQTT password

/**
 * Utility function for checking, if it is connected to a device before publishing a message.
 */
void checkConnectionAndPublish(const char* topic, const char* message) {
  if (client.connected()) {
    client.publish(topic, message);
  }
}

/**
 * Function for connecting to MQTT broker.
 * Will blink the LEDs 3 times to signal successful connection to MQTT.
 */
void connectMqtt() {
  // loop until connected to MQTT
  while (!client.connected()) {
    Serial.print("Trying to connect to MQTT...");
    // try to connect to MQTT broker with given credentials
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // subscribe to the topic for threshold updates and LED configuration updates
      client.subscribe("light/thresholds");
      client.subscribe("config/direction");
      signalingLED(3); // blink 3 times to signal successful connection to MQTT
    } else { // if connection fails, print error message with return code, to understand what went wrong and try again in 5 seconds
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**
 * Callback function for incoming MQTT messages.
 * 
 * @param topic - Topic of the incoming message
 * @param payload - Payload of the incoming message as a byte array
 * @param length - Length of the payload
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived at ");
  Serial.print(topic);
  Serial.print(", ");

  // Convert payload to a string
  char message[length + 1]; // create a char array for the message with the length + 1 for the \0
  strncpy(message, (char*)payload, length); // copy the payload to the message array
  message[length] = '\0'; // add the \0 to the end of the message array

  Serial.print("Message: ");
  Serial.println(message);

  // Handle thresholds update
  if (strcmp(topic, "light/thresholds") == 0) {
    float newMinLux, newMaxLux;
    char *token = strtok(message, ","); // get the first token

    if (token != NULL) // check the first token
    {
      newMinLux = atof(token); // convert the first token to a float
      token = strtok(NULL, ","); // get the next token

      if (token != NULL) // check the second token
      {
        newMaxLux = atof(token); // convert the second token to a float

        // check if the threshold values are valid
        if (newMinLux < 0 || newMaxLux < 0 || newMinLux > newMaxLux)
        {
          const char *errorMsg = "Invalid thresholds received. Thresholds must be positive and minLux must be less than maxLux.";
          Serial.println(errorMsg);

          // publish error message to MQTT
          checkConnectionAndPublish("light/status/error", errorMsg);
          return;
        }
        
        saveLightThresholds(newMinLux, newMaxLux); // everything is valid, update thresholds

        char successMsg[50];
        snprintf(successMsg, sizeof(successMsg), "Thresholds have been updated to %.2f - %.2f lux.", newMinLux, newMaxLux);

        Serial.println(successMsg);

        // publish success message to MQTT
        checkConnectionAndPublish("light/status/success", successMsg);
      }
      else
      {
        const char *errorMsg = "Missing a threshold.";
        Serial.println(errorMsg);

        // publish error message to MQTT
        checkConnectionAndPublish("light/status/error", errorMsg);
      }
    }
    else
    {
      const char *errorMsg = "Missing a threshold.";
      Serial.println(errorMsg);

      // publish error message to MQTT
      checkConnectionAndPublish("light/status/error", errorMsg);
    }
  }
  else if (strcmp(topic, "config/direction") == 0) {
    if (strcmp(message, "day") == 0) {
      isNightMode = false;
      const char *successMsg = "Brightness direction has been set to day.";

      saveConfig(isNightMode);

      Serial.println(successMsg);

      // publish success message to MQTT
      checkConnectionAndPublish("config/status/success", successMsg);
    } else if (strcmp(message, "night") == 0) {
      isNightMode = true;

      saveConfig(isNightMode);

      const char *successMsg = "Brightness direction has been set to night.";
      Serial.println(successMsg);

      // publish success message to MQTT
      checkConnectionAndPublish("config/status/success", successMsg);
    } else if (strcmp(message, "swap") == 0) {
      isNightMode = !isNightMode;

      saveConfig(isNightMode);

      char successMsg[50];
      snprintf(successMsg, sizeof(successMsg), "Brightness direction has been swapped to %s.", isNightMode ? "night" : "day");
      Serial.println(successMsg);

      // publish success message to MQTT
      checkConnectionAndPublish("config/status/success", successMsg);
    } else
    {
      const char *errorMsg = "Unknown message.";
      Serial.println(errorMsg);

      // publish error message to MQTT
      checkConnectionAndPublish("config/status/error", errorMsg);
    }
    
  }
  else
  {
    Serial.println("Unknown topic.");
  }
}