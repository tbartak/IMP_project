/**
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <Preferences.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "secret.h"

// WiFi settings
const char* ssid = WIFI_SSID; // WiFi SSID
const char* password = WIFI_PASSWORD; // WiFi password

// MQTT broker settings
const char* mqtt_server = MQTT_SERVER; // MQTT broker address (using HiveMQ Cloud public broker)
const int mqtt_port = MQTT_PORT; // MQTT broker port (TLS)
const char* mqtt_user = MQTT_USER; // MQTT username
const char* mqtt_password = MQTT_PASSWORD; // MQTT password

WiFiClientSecure secureClient; // Create a secure client instance (for TLS)
PubSubClient client(secureClient); // Pass the secure client to PubSubClient (for TLS)

#define DEFAULT_MAX_LUX 1000.0 // maximum light level in lux
#define DEFAULT_MIN_LUX 100.0 // minimum light level in lux

float MAX_LUX; // maximum light level in lux
float MIN_LUX; // minimum light level in lux

BH1750 lightSensor; // create instance of BH1750 class (light sensor)

const int LED_1 = 4; // LED connected to GPIO 4
const int LED_2 = 2; // LED connected to GPIO 2
const int freq = 5000; 
const int resolution = 8;
const int ledChannel1 = 0;
const int ledChannel2 = 1;

int previousDutyCycle = 0;

bool isNightMode;

Preferences preferences; // create instance of Preferences class (NVS)

/**
 * Function for saving thresholds to NVS (non-volatile storage).
 * 
 * @param minLux - Minimum light level threshold in lux
 * @param maxLux - Maximum light level threshold in lux
 */
void saveLightThresholds(float minLux, float maxLux) {
  if (!preferences.begin("lightThresholds", false)) // try to open NVS 'lightThresholds' namespace in read-write mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for saving light thresholds");
    return;
  }
  
  preferences.putFloat("minLux", minLux); // save value
  preferences.putFloat("maxLux", maxLux); // save value
  preferences.end(); // close

  Serial.println("Light thresholds have been successfully updated and saved.");
}

/**
 * Function for loading thresholds from NVS (non-volatile storage).
 */
void loadLightThresholds() {
  if (!preferences.begin("lightThresholds", true)) // try to open NVS 'lightThresholds' namespace in read-only mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for loading light thresholds. Using default values.");
    return;
  }

  MIN_LUX = preferences.getFloat("minLux", DEFAULT_MIN_LUX); // get value or use default
  MAX_LUX = preferences.getFloat("maxLux", DEFAULT_MAX_LUX); // get value or use default
  preferences.end(); // close

  Serial.print("Loaded light thresholds: ");
  Serial.print(MIN_LUX);
  Serial.print(" - ");
  Serial.println(MAX_LUX);
}

/**
 * Function for updating light thresholds.
 * 
 * @param minLux - Minimum light level threshold in lux
 * @param maxLux - Maximum light level threshold in lux
 */
void updateLightThresholds(float minLux, float maxLux) {
  MIN_LUX = minLux; // internally update MIN_LUX
  MAX_LUX = maxLux; // internally update MAX_LUX
  saveLightThresholds(MIN_LUX, MAX_LUX); // save new thresholds to NVS for future use
}

/**
 * Function for saving configuration of the LEDs to NVS (non-volatile storage).
 */
void saveConfig() {
  if (!preferences.begin("config", false)) // try to open NVS 'config' namespace in read-write mode, if something goes wrong, print error message and return
  {
    Serial.println("Failed to open NVS for saving configuration of the LEDs.");
    return;
  }

  preferences.putBool("isNightMode", isNightMode); // save value
  preferences.end(); // close

  Serial.println("Configuration of the LEDs has been successfully updated and saved.");
}

/**
 * Function for loading configuration of the LEDs from NVS (non-volatile storage).
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
 * Function for updating configuration of the LEDs.
 * 
 * @param nightMode - Night mode state
 */
void updateConfig(bool nightMode) {
  isNightMode = nightMode; // internally update isNightMode
  saveConfig(); // save new configuration to NVS for future use
}

/**
 * Load all necessary data from NVS (non-volatile storage).
 */
void loadAllData() {
  loadLightThresholds(); // load light thresholds
  loadConfig(); // load configuration of the LEDs
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
 */
float brightnessCalculation(float lux) {
  float percentage;
  if (lux < MIN_LUX) {
    percentage = 0.0;
  } else if (lux > MAX_LUX) {
    percentage = 100.0;
  } else {
    percentage = (lux - MIN_LUX) / (MAX_LUX - MIN_LUX) * 100.0;
  }

  if (isNightMode) {
    percentage = 100.0 - percentage; // invert the percentage, if night mode is enabled
  }

  percentage = gammaLinearization(percentage); // 

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

void signalingLED(int times) {
  for (int i = 0; i < times; i++) {
    brightnessFade(0, 255);
    brightnessFade(255, 0);
  }
}

// utility function for checking if it is connected to a device before publishing a message
void checkConnectionAndPublish(const char* topic, const char* message) {
  if (client.connected()) {
    client.publish(topic, message);
  }
}

/**
 * Function for setting up Wi-Fi connection.
 * Will blink the LEDs 3 times to signal successful connection to Wi-Fi.
 * 
 * @param ssid - Wi-Fi SSID
 * @param password - Wi-Fi password
 */
void setup_wifi(const char* ssid, const char* password) {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // connect to chosen WiFi

  // loop until connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  signalingLED(3); // blink 3 times to signal successful connection to WiFi
}

/**
 * Function for connecting to MQTT broker.
 * Will blink the LEDs 3 times to signal successful connection to MQTT.
 */
void connect_mqtt() {
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
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
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
        
        updateLightThresholds(newMinLux, newMaxLux); // everything is valid, update thresholds

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

      updateConfig(isNightMode);

      Serial.println(successMsg);

      // publish success message to MQTT
      checkConnectionAndPublish("config/status/success", successMsg);
    } else if (strcmp(message, "night") == 0) {
      isNightMode = true;

      updateConfig(isNightMode);

      const char *successMsg = "Brightness direction has been set to night.";
      Serial.println(successMsg);

      // publish success message to MQTT
      checkConnectionAndPublish("config/status/success", successMsg);
    } else if (strcmp(message, "swap") == 0) {
      isNightMode = !isNightMode;

      updateConfig(isNightMode);

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

/**
 * Setup function runs once when the micro-controller is powered on
 */
void setup() {
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  Wire.begin(); // initialize I2C communication
  lightSensor.begin(); // initialize light sensor

  loadAllData(); // load light thresholds and LED config from NVS

  // initialize LED PWM
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(LED_1, ledChannel1);
  ledcAttachPin(LED_2, ledChannel2);

  // initialize secure client
  secureClient.setInsecure(); // set insecure connection (workaround for HiveMQ Cloud TLS connection)

  // initialize and connect to WiFi
  setup_wifi(ssid, password);
  
  // connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port); // set MQTT broker address and port
  client.setCallback(mqtt_callback); // set callback function for incoming messages
}

void loop() {
  // connect to MQTT broker or reconnect, if connection is lost
  if (!client.connected()) {
    connect_mqtt();
  }
  client.loop(); // keep the MQTT connection alive

    float lux = lightSensor.readLightLevel(); // read light level in lux
    // DEBUG: print light level
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    // calculate brightness percentage
    float brightness = brightnessCalculation(lux);
    Serial.print("Brightness: ");
    Serial.print(brightness);
    Serial.println("%");

    // calculate duty cycle for PWM (0-255 int values)
    int currentDutyCycle = brightness * 2.55; // the brightness is already linearized using gamma correction
    currentDutyCycle = constrain(currentDutyCycle, 0, 255);

    // fade between previous and current duty cycle // TODO: could think of a better option without the need to have delay in the function directly, use millis() instead
    brightnessFade(previousDutyCycle, currentDutyCycle);

    // save current duty cycle for next iteration
    previousDutyCycle = currentDutyCycle;

    // publish current light level every 5 seconds to MQTT (HiveMQ Cloud)
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish >= 5000) {
      lastPublish = millis();
      char luxMessage[30];
      snprintf(luxMessage, sizeof(luxMessage), "Current lux: %.2f lx.", lux);
      checkConnectionAndPublish("light/lux", luxMessage);
      Serial.println("Published current light level.");
    }

    // delay(255); // wait 255ms between measurements // prozatím nahrazeno delayem v brightnessFade
}