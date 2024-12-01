/**
 * @file main.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "main.h"

/**
 * Setup function runs once when the micro-controller is powered on
 */
void setup() {
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  Wire.begin(); // initialize I2C communication
  lightSensor.begin(); // initialize light sensor

  loadAllData(); // load light thresholds and LED config from NVS

  // initialize LED PWM
  setupLEDs();

  // initialize secure client
  secureClient.setInsecure(); // set insecure connection (workaround for HiveMQ Cloud TLS connection)

  // initialize and connect to WiFi
  setup_wifi(ssid, password);
  
  // connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port); // set MQTT broker address and port
  client.setCallback(mqttCallback); // set callback function for incoming messages
}

/**
 * Loop function runs repeatedly as long as the micro-controller is powered on
 */
void loop() {
  // connect to MQTT broker or reconnect, if connection is lost
  if (!client.connected()) {
    connectMqtt();
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