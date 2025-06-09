#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <time.h>
#include "esp_mac.h"
#include "secrets.h"

// WiFi credentials
// secrets.h

// MQTT Broker settings
// secrets.h

// BME280 setup
Adafruit_BME280 bme; // I2C interface
// Use this for SPI: Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

// MQTT Client ID
char clientId[20];
uint8_t myMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -25200;  // GMT-7 (Arizona time) in seconds
const int daylightOffset_sec = 0;   // Arizona doesn't observe DST

// MQTT Topics
const char* topic_temperature = "garage/enviro_sensors/temperature";
const char* topic_humidity = "garage/enviro_sensors/humidity";
const char* topic_dewpoint = "garage/enviro_sensors/dewpoint";
const char* topic_status = "garage/enviro_sensors/status";

// LED pin for status indication
#define LED_PIN 2

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 600000;  // Send data every 600 seconds (10 minutes)

/**
 * Initial setup function - runs once at startup
 * Initializes serial communication, LED pin, generates unique client ID,
 * initializes DHT sensor, connects to WiFi, synchronizes time via NTP,
 * sets up MQTT connection, and sends startup status message
 */
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  // Generate unique client ID with MAC address
  getMyMACAddress();
  snprintf(clientId, sizeof(clientId), "esp32-%02x%02x%02x", myMAC[3], myMAC[4], myMAC[5]);
  
  // Initialize BME280
  if (!bme.begin(0x76)) { // Try 0x77 if 0x76 doesn't work
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("BME280 sensor initialized successfully");
  
  // Connect to WiFi
  setup_wifi();
  
  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();
  Serial.println("Time synchronized");
  
  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("ESP32 Environmental Sensor initialized");
  
  // Send startup message
  if (client.connected()) {
    client.publish(topic_status, "ESP32 Environmental Sensor Online");
  }
}

/**
 * Retrieves the ESP32's MAC address from internal memory
 * Populates the global myMAC array and prints a formatted client name
 * Used to generate unique MQTT client identifiers
 */
void getMyMACAddress() {
  if (esp_efuse_mac_get_default(myMAC) == ESP_OK) {
      char buffer[64];
      sprintf(buffer, "My name is esp32-%02X%02X%02X", myMAC[3], myMAC[4], myMAC[5]);
      Serial.println(buffer);
  }
  else {
    Serial.println("Error retrieving unit's MAC address.");
  }
}

/**
 * Establishes WiFi connection using credentials from secrets.h
 * Blinks LED during connection process, turns LED solid when connected
 * Prints connection status and assigned IP address to serial monitor
 */
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Blink LED while connecting
  }

  digitalWrite(LED_PIN, HIGH);  // Solid LED when connected
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * MQTT message callback function - handles incoming MQTT messages
 * Currently just prints received messages to serial monitor
 * Can be extended to process specific commands or data
 */
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

/**
 * Reconnects to MQTT broker when connection is lost
 * Continuously attempts connection with 5-second delays between attempts
 * Updates LED status (off during disconnection, on when connected)
 * Publishes reconnection status message upon successful connection
 */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print("Client ID: ");
    Serial.println(clientId);
    
    // Attempt to connect with our predefined client ID
    if (client.connect(clientId, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      digitalWrite(LED_PIN, HIGH);
      
      // Once connected, publish an announcement
      client.publish(topic_status, "ESP32 Environmental Sensor Reconnected");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(LED_PIN, LOW);
      delay(5000);
    }
  }
}

/**
 * Calculates dew point temperature using the Magnus formula
 * Takes temperature (Celsius) and relative humidity (%) as inputs
 * Returns dew point temperature in Celsius
 * Used for environmental monitoring and condensation prediction
 */
float calculateDewpoint(float temperature, float humidity) {
  // Magnus formula for dewpoint calculation
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * temperature) / (b + temperature)) + log(humidity / 100.0);
  float dewpoint = (b * alpha) / (a - alpha);
  return dewpoint;
}

/**
 * Gets current local time as a formatted string
 * Returns time in "YYYY-MM-DD HH:MM:SS" format
 * Returns error message if NTP time synchronization has failed
 * Used for timestamping sensor data
 */
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time not available";
  }
  
  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

/**
 * Gets current time as Unix epoch timestamp
 * Returns seconds since January 1, 1970 UTC
 * Used for precise timestamping in sensor data JSON payload
 */
time_t getEpochTime() {
  time_t now;
  time(&now);
  return now;
}

/**
 * Reads DHT22 sensor data and publishes to MQTT broker
 * Reads temperature and humidity, calculates dew point
 * Converts temperature from Celsius to Fahrenheit
 * Creates JSON payload with sensor data, timestamps, and client ID
 * Publishes data to garage/enviro_sensors/ topic
 * Handles sensor read errors and publishes error status if needed
 */
void publishSensorData() {
  // Read all sensors from BME280
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
  
  // Check if readings are valid
  if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
    Serial.println("Failed to read from BME280 sensor!");
    return;
  }
  
  // Calculate dewpoint
  float dewpoint = calculateDewpoint(temperature, humidity);

  temperature = temperature * 1.8 + 32.0;
  dewpoint = dewpoint * 1.8 + 32.0;
  
  // Get current time
  String currentTime = getFormattedTime();
  time_t epochTime = getEpochTime();
  
  // Create JSON payload (optional - you can also send individual values)
  JsonDocument doc;
  doc["temperature"] = round(temperature * 100) / 100.0;  // Round to 2 decimal places
  doc["humidity"] = round(humidity * 100) / 100.0;
  doc["dewpoint"] = round(dewpoint * 100) / 100.0;
  doc["pressure"] = round(pressure / 100.0);
  doc["timestamp"] = currentTime;
  doc["epoch"] = epochTime;
  doc["client_id"] = clientId;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Publish individual topics
  // client.publish(topic_temperature, String(temperature, 2).c_str());
  // client.publish(topic_humidity, String(humidity, 2).c_str());
  // client.publish(topic_dewpoint, String(dewpoint, 2).c_str());
  
  // Optional: Publish combined JSON payload
  client.publish("garage/enviro_sensors/", jsonString.c_str());
  
  // Print to Serial for debugging
  Serial.println("--- Sensor Data Published ---");
  Serial.print("Time: ");
  Serial.println(currentTime);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °F");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Dewpoint: ");
  Serial.print(dewpoint);
  Serial.println(" °F");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.println("JSON: " + jsonString);
  Serial.println("-----------------------------");
}

/**
 * Main program loop - runs continuously after setup()
 * Maintains MQTT connection (reconnects if needed)
 * Publishes sensor data at regular intervals (every 300 seconds)
 * Includes brief delay to prevent watchdog timer issues
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    publishSensorData();
  }
  
  // Brief delay to prevent watchdog issues
  delay(100);
}
