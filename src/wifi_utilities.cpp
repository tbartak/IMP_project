/**
 * @file wifi_utilities.cpp
 * @author: Tomáš Barták
 * @login: xbarta51
 */

#include "wifi_utilities.h"

const char* ssid = WIFI_SSID; // WiFi SSID
const char* password = WIFI_PASSWORD; // WiFi password

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