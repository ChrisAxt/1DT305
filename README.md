
# Tutorial on how to build a temperature and humidity sensor
Give a short and brief overview of what your project is about. What needs to be included:

- **Project title:** Temperature and humidity sensor
- **Author:** Christopher Axt ca223sp
- **Overview:** IOT Temperature and humidity monitor using an ESP32 microcontroller and DHT11 temperature and humidity sensor 
- **Time taken:** 
  - **Circuit connection:** Approx 1 hour
  - **Research and code:** Approx 35 - 40 hours 

# Objective
I have chosen to build this device because I have plans on building a device that monitors conditions on my boat and starting with the temperature and humidity sensor give me insights on how to accomplish that.
The purpose of this device is to monitor temperature and humidity remotely which can be used for various applications to give insights on the conditions at any given location. 

# Material
Explain all material that is needed. All sensors, where you bought them and their specifications. Please also provide pictures of what you have bought and what you are using.

**List of material:**
  - ESP32 Development Board
  - Various jumper wires
  - Bread board (Optional)
  - DHT11 sensor

**Description of materials:**
  - **ESP32 Development Board:** is a low-cost, low-power system on a chip microcontrollers with integrated Wi-Fi and dual-mode Bluetooth.
  - **Jumper wires:** Used to connect sensors and other devices to the microcontroller via the bread board.
  - **Bread board:** is a construction base used to build semi-permanent prototypes of electronic circuits.
  - **DHT11 sensor:** A commonly used temperature and humidity sensor with a measuring range from 0 to 50 degrees Celsius with +-2 degrees accuracy.
  
**Cost:**

| Item  | Purchased from | Cost |
| ------------- | ------------- | ------------- |
| ESP32 Development Board  | Amazon.se  | 84 SEK  |
| Jumper wires  | Amazon.se  | 60 SEK  |
| Bread board  | communica.co.za  | 300 ZAR / 190 SEK  |
| DHT11 sensor  | Amazon.se  | 30 SEK  |

In this project I have chosen to work with the ESP32 development board as seen in the image below, it’s a low cost, low power unit programmed by C and compatible with the Arduino IDE and can connect via Bluetooth and Wi-Fi. The low cost and low power nature of this device as well as the numerous input and output connections of this make it perfect for IoT applications.

![image](https://user-images.githubusercontent.com/81012809/177135865-5f61905c-8e61-454d-8653-dd2685a37097.jpg)

# Computer setup
The device is programmed using the Arduino IDE and the C and C++ programming languages, these languages are ideal for embedded and IoT devices due to the low level nature of the languages.

### Setup:

### Chosen IDE

  **Arduino IDE installation:**  https://www.arduino.cc/en/software
  
  **Plugin's needed:**
  
    - esp32 by espressif systems
    
   **Library  installation steps:**
   
    - Open Arduino IDE
    - Click "Tools" -> "Boards" -> "Boards manager"
    - In the search bar, type the plugin for the board you wish to install.
    
  **Libraries  needed:**
  
    -  WiFi.h
    -  PubSubClient.h
    -  DHT.h
   
  **Library installation steps:**
  
    - Open Arduino IDE
    - Chick "Sketch" -> "Include Library" -> "Manage Libraries"
    - In the search bar, type the library name you wish to install and chick the install button. 

  **Flashing:**
  
  - Flashing the ESP32 can be done using various fleshing tools available online or simply by holding down the Boot button and tapping the EN button. This will remove     the code that is stored in memory. Essentially a factory reset.

  **Uploading  code:**
  
   - Once you can written the code you wish to upload to the ESP32 you can first check if the code compiles by clicking the button marked with a tick on the top left        in the Arduino IDE, alternatively you can click the button marked with an arrow on the top left in the Arduino IDE, this will first check if the code complies          then check if the board is connected and finally upload the code to the board. 

# Putting everything together:
**Computer connection:**

- The ESP32 is connected to a computer using a micro-USB cable.

**ESP32 connection to bread board**

- I've chosen to use a bread board in order to organise the components and make connections and handling easier.
- Half of the pins of ESP32 are connected to the edge of the breadboard.

**ESP32 connection to sensor**

- The ground and 3.3v are connected to the sensor via the breadboard and the D4 pin is connected to the data pin of the DHT11 as depicted in the image in the next section.
- The DHT11 sensor used had a built-in pullup resistor so no additional resistors are needed.   

# Circuit diagram 

![Untitled](https://user-images.githubusercontent.com/81012809/177126930-61c90ae8-17cc-481a-a206-65145e629306.png)


# Platform

**Chosen platform:**
- Datacake.de is a cloud based IoT platform.

**Subscription:**
- Current subscription is a free subscription for up to two devices.
- Scaling the project would be as easy as changing the subscription to allow for more devices.

**Platform functionality**
- The platform is connected to the device via MQTT. The platform subscribes to the topic that the device publishes to.
- The platform is also capable of polishing topics that the device can subscribe to if needed.

# The code

**Function to connect to wifi**
```c
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
```

**Function to connect to MQTT**
```c
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
```

**Function to read sensor**
```c
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
```

# Explain your code!
**Connectivity:**

- The ESP32 is connected to the internet using a wifi connection.
- The data is sent as a string formatted so it can be parsed to a json object which then can be accessed using the attributes of the object, this is done using the MQTT protocol and the test.mosquitto.org broker.

**Data transmission:**

- The ESP32 wakes up from deep sleep every 30 minutes, connects to wifi and MQTT then the sensor is read, and the data is published to the broker using MQTT. The data   is then sent to all the subscribers of the topic used to send the data.
- In total the device is out of deep sleep for approximately 5 seconds before going into deep sleep again.  

# Presenting the data

- The data is presented using two area charts which saves data every time it's received (every 30 minutes) and the data is retain in the database for a week, however     this can be extended by upgrading the subscription.

**Visual representation**

![Dashboard](https://user-images.githubusercontent.com/81012809/177156358-3f476c2a-44c3-48b3-99d8-1eea66b0dbf1.png)


# Finalizing the design

**Project impression:**

- Connecting devices to the internet, even as simple as this one has been really interesting and eye opening. The possibilities of IoT devices are endless.  

**Future improvements**

- In future iterations, this device could be improved by connecting the sensor to a LTE compatible microcontroller which would allow the device to connect and send       data from a much wider range of areas. As well as connect the device using a battery.

**Image of actual device:**

![1656938915527](https://user-images.githubusercontent.com/81012809/177158343-9b0a2f56-63ef-40c9-b226-6bba0c1ab072.jpg)
