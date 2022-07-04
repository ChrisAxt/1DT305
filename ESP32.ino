/* ---------- Include section ---------- */
#if SSL_ENABLED
#include <WiFiClientSecure.h>
#else
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include "DHT.h"
#include "secrets.h"

/* ---------- Define section ---------- */
#define DHTPIN 4
#define DHTTYPE DHT11
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800
/* ---------- Function declaration ---------- */

void wifiConnect();
void mqttConnect();
void readTempHumidity();
void print_wakeup_reason();

/* ---------- Global variables ---------- */
#if SSL_ENABLED
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif

PubSubClient mqttClient(wifiClient);
DHT dht(DHTPIN, DHTTYPE);
char buffer[30];

/*
 * Setup function
 */
void setup() {

  delay(2000);                                                        // Delay to give time to open serial monitor
  Serial.begin(115200);
  dht.begin();                                                        // Initialize the dht object 
  pinMode(DHTPIN, OUTPUT);                                            // Set DHTPIN to output
  WiFi.setHostname(hostname);                                         // Initialise wifi
  WiFi.mode(WIFI_AP_STA);
  wifiConnect();

  #if SSL_ENABLED                                                     // Initialize MQTT
  wifiClient.setCACert(root_ca); // Verify CA certificate
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  #else
  mqttClient.setServer(MQTT_HOST, MQTT_PORT_INSECURE);
  #endif
  mqttConnect();

  print_wakeup_reason();                                              // Print the wakeup reason for ESP32
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);      // Initialize sleep timer

}

/*
 * Main loop
 */
void loop() {
  mqttClient.loop();                                                    // MQTT loop

  readTempHumidity();                                                   // Function call to read sensor
  Serial.println("Going to sleep now");                                 // Print a sleep message
  delay(1000);
  Serial.flush();                                                       // Flush the serial
  esp_deep_sleep_start();                                               // Start deep sleep
  Serial.println("Test sleep (Should not be printed)");                 // Message to check if sleep was entered
}

/*
 * Function that connects to wifi using the info stored in the secrets.h header
 */
void wifiConnect() {
 
  WiFi.begin(ssid, pass);                                               // Connect to wifi using the ssid and password stored in the secrets.h file
  Serial.print("Connecting to WiFi...");

  uint8_t retryCounter = 10;                                            // Check the connection and try again if failed
  while (WiFi.status() != WL_CONNECTED && retryCounter > 0) {
    Serial.print(".");
    delay(500);
    retryCounter--;
  }
  if (retryCounter > 0) {                                               // Print the status if successful
    Serial.println("connected");
  } else {                                                              // Print the status is failed and restart the ESP32
    Serial.print("failed, status code =");
    Serial.println(WiFi.status());
    Serial.println("Restarting in 5 sec");
    delay(5000);
    ESP.restart();
  }
}

/*
 * Function to connect to MQTT broker once a wifi connection is established
 */
void mqttConnect() {
  
  while (!mqttClient.connected()) {                                      // Connect to the MQTT broker
    Serial.print("MQTT connecting...");
    if (mqttClient.connect("", MQTT_USER, MQTT_PASS)) {                  // Print status is sucessful
      Serial.println("connected");
      //mqttClient.subscribe(MQTT_SUB_TOPIC);                            //Subscribe to topic if needed
    } else {                                                             // Print status if sucessful
      Serial.print("failed, status code =");
      Serial.print(mqttClient.state());
      Serial.println("try again in 5 seconds");
      delay(5000);                                                       // Wait 5 seconds before retrying
    }
  }
}

/*
 * Function used read the temp and humidity from the sensor and publish it via MQTT to the broker
 */
void readTempHumidity() {

  if (WiFi.status() != WL_CONNECTED) {                                  // Check wifi connection
    wifiConnect();
  }
  if (!mqttClient.connected()) {                                        // Check MQTT connection
    mqttConnect();
  }
  double h = dht.readHumidity();                                        // Reading temperature and humidity and store value in double
  double t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {                                           // Check if any reading failed and exit early.
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  sprintf(buffer, "{\"temp\": %f, \"humidity\": %f}", t, h);            // Store a string with the two values in a char array
  
  if (mqttClient.publish(MQTT_PUB_TOPIC, buffer)) {                     // Check if the payload is sent successfully
    Serial.print(F("Package sent: "));
    Serial.println(buffer);
  } else {
    Serial.print(F("Error sending package!"));
  }
}

/*
 * Function to check why ESP woke up
 */
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}
