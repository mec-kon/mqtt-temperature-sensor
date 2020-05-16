#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "LM75A.h"
#include "../lib/pubsubclient/src/PubSubClient.h"

#define SECOND 1000000UL
#define MINUTE 60000000UL
#define HOUR 3600000000UL

static WiFiClient wifiClient;
static PubSubClient client(wifiClient);

static char msg[6];


// Create I2C LM75A instance
static LM75A lm75a_sensor;
// Equivalent to "LM75A lm75a_sensor;"

// Time to sleep (in seconds):
const int sleepTimeS = 10;

void setup_wifi() {
    // We start by connecting to a WiFi network
    Serial.println("Wifi init");
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);  //Connect to the WiFi network
    while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
        delay(500);
        Serial.println("Waiting to connect...");
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //Print the local IP
}

void on_message(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");

        // Attempt to connect
        if (client.connect(MQTT_CLIENT_ID, MQTT_CLIENT, MQTT_CLIENT_PASSWORD)) {
            Serial.println("connected");
        }
        else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}



void setup(void)
{
    Serial.begin(115200);

    setup_wifi();
    client.setServer(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT);
    client.setCallback(on_message);

    if (client.connect(MQTT_CLIENT_ID, MQTT_CLIENT, MQTT_CLIENT_PASSWORD)) {
        Serial.println("mqtt connected");
    }


    if (!client.connected()) {
        reconnect();
    }
    client.loop();


    float temperature_in_degrees = lm75a_sensor.getTemperatureInDegrees();

    for (int i=0; i<5 && temperature_in_degrees != INVALID_LM75A_TEMPERATURE; i++) {
        temperature_in_degrees = lm75a_sensor.getTemperatureInDegrees();
    }

    if (temperature_in_degrees == INVALID_LM75A_TEMPERATURE) {
        Serial.println("Error while getting temperature");
        String error_message = "error";
        error_message.toCharArray(msg, 6);
    }
    else {
        Serial.print("Temperature: ");
        Serial.print(temperature_in_degrees);
        Serial.print(" degrees.");

        dtostrf(temperature_in_degrees, 6, 2, msg);
    }


    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(MQTT_PUBLISH_TOPIC, String(temperature_in_degrees).c_str());

    Serial.println("published!");

    delay(1000);

    ESP.deepSleep(HOUR);
}


void loop(){
}
