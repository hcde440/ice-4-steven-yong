//HEAVY commenting is on . . .
//In this program we are publishing and subscribing to a MQTT server that requires a login/password
//authentication scheme. We are connecting with a unique client ID, which is required by the server.
//This unique client ID is derived from our device's MAC address, which is unique to the device, and
//thus unique to the universe.
//
//We are publishing with a generic topic ("theTopic") which you should change to ensure you are publishing
//to a known topic (eg, if everyone uses "theTopic" then everyone would be publishing over everyone else, which
//would be a mess). So, create your own topic channel.
//
//We have hardcoded the topic and the subtopics in the mqtt.publish() function, because those topics and sub
//topics are never going to change. We have subscribed to the super topic using the directory-like addressing
//system MQTT provides. We subscribe to 'theTopic/+' which means we are subscribing to 'theTopic' and every
//sub-topic that might come after the main topic. We denote this with a '+' symbol.
//
//Please change your super topic and don't use 'theTopic'.
/////

//Include all the libraries for the microcontroller, sensors, display, and MQTT.
#include <ESP8266WiFi.h>
#include "Wire.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DATA_PIN 12
DHT_Unified dht(DATA_PIN, DHT22);

//Identifiers and passwords to connect to the internet and MQTT.
#define wifi_ssid ""
#define wifi_password ""

#define mqtt_server ""
#define mqtt_user ""
#define mqtt_password ""

//Execute the clients.
WiFiClient espClient;
PubSubClient mqtt(espClient);

//Use the unique MAC address of the microcontroller as an identifier when connecting to MQTT.
char mac[6];

//Create a character array to store the JSON data to be sent to MQTT.
char message[201];

//Create timers to keep track of when to collect data.
unsigned long currentMillis, timer;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);
  timer = millis();
}

//Connect to the internet and display messages to help debug issues.
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println(WiFi.macAddress());
}

//Reconnect to MQTT if it gets disconnected; subscribe to a topic.
void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      mqtt.subscribe("stevenyong/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//Keep looping to get values from the sensors and maintain connection to MQTT. Once every 15 seconds, publish the sensor values to MQTT in JSON.
void loop() {
  sensors_event_t event;
  
  dht.temperature().getEvent(&event);
  float celsius = event.temperature;
  float temp = (celsius * 1.8) + 32;

  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop();

  if (millis() - timer > 15000) {
    char str_temp[6];
    char str_humd[6];
    
    dtostrf(temp, 5, 2, str_temp);
    dtostrf(humidity, 5, 2, str_humd);

    sprintf(message, "{\"temp\":\"%s\", \"humd\":\"%s\"}", str_temp, str_humd);
    mqtt.publish("stevenyong/tempHum", message);
    timer = millis();
  }
}

//Receive data from MQTT as a subscriber and process the JSON data.
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success()) {
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
  
  if (strcmp(topic, "stevenyong/pressure") == 0) {
    Serial.println("A message from Batman . . .");
  }
  else if (strcmp(topic, "stevenyong/tempHum") == 0) {
    Serial.println("Some weather info has arrived . . .");
  }
  else if (strcmp(topic, "stevenyong/switch") == 0) {
    Serial.println("The switch state is being reported . . .");
  }

  root.printTo(Serial);
  Serial.println();
}
