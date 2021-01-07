#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "LM75A.h"
#include "PubSubClient.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"

#define SECOND 1000000UL
#define MINUTE 60000000UL
#define HOUR 3600000000UL

static WiFiClient wifiClient;
static PubSubClient client(wifiClient);

static char error_msg[12];


// Create I2C LM75A instance
static LM75A lm75a_sensor;

// Create I2C BME280 instance
static TwoWire i2c_bme280_wire;
static Adafruit_BME280 bme280_sensor;

// Time to sleep (in seconds):
const int sleepTimeS = 10;


void on_message(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

bool reconnect() {
    // Loop until we're reconnected
    for(int i=0; (i<10) && (!client.connected()); i++) {
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
    if(!client.connected()){
        return false;
    }
    else {
        return true;
    }
}

void publish_sensor_data(){
    // setup bme280
    unsigned int bme280_status = 0;
    i2c_bme280_wire.begin(I2C_SDA, I2C_SCL);

    float lm75_temperature_in_degrees = INVALID_LM75A_TEMPERATURE;


    for (int i=0; i<5 && (lm75_temperature_in_degrees == INVALID_LM75A_TEMPERATURE); i++) {
        lm75_temperature_in_degrees = lm75a_sensor.getTemperatureInDegrees();
    }
    for (int i=0; i<5 && !bme280_status; i++) {
        bme280_status = bme280_sensor.begin(BME280_BASE_ADDRESS, &i2c_bme280_wire);
    }
    float bme280_temperature_in_degrees = bme280_sensor.readTemperature();
    float bme280_pressure = bme280_sensor.readPressure()/100.0;
    float bme280_altitude = bme280_sensor.readAltitude(SENSORS_PRESSURE_SEALEVELHPA);
    float bme280_humidity = bme280_sensor.readHumidity();

    Serial.println("Temperature:");
    Serial.println(bme280_temperature_in_degrees);
    Serial.println(lm75_temperature_in_degrees);

    if (lm75_temperature_in_degrees == (float) INVALID_LM75A_TEMPERATURE) {
        Serial.println("Error while getting lm75 temperature");
        String error_message = "lm75_error";
        error_message.toCharArray(error_msg, 10);
    }
    else {
        client.publish(MQTT_PUBLISH_TOPIC_LM75_TEMPERATURE, String(lm75_temperature_in_degrees).c_str());
    }

    if (!bme280_status) {
        Serial.println("Error while getting bme280 temperature");
        String error_message = "bme280_error";
        error_message.toCharArray(error_msg, 12);
    }
    else {
        client.publish(MQTT_PUBLISH_TOPIC_BME280_TEMPERATURE, String(bme280_temperature_in_degrees).c_str());
        client.publish(MQTT_PUBLISH_TOPIC_BME280_PRESSURE, String(bme280_pressure).c_str());
        client.publish(MQTT_PUBLISH_TOPIC_BME280_ALTITUDE, String(bme280_altitude).c_str());
        client.publish(MQTT_PUBLISH_TOPIC_BME280_HUMIDITY, String(bme280_humidity).c_str());
    }
}

void publish_battery_level(){
    int battery_voltage = analogRead(A0);
    float battery_level = (battery_voltage - BATTERY_MIN) * (100.0/ (BATTERY_MAX - BATTERY_MIN));
    client.publish(MQTT_PUBLISH_TOPIC_BATTERY_LEVEL, String(battery_level).c_str());
    client.publish(MQTT_PUBLISH_TOPIC_BATTERY_RAW_DATA, String(battery_voltage).c_str());

}

bool setup_wifi() {
    // We start by connecting to a WiFi network
    Serial.println("Wifi init");
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);  //Connect to the WiFi network
    for(int i=0; (i<40) && (WiFi.status() != WL_CONNECTED); i++) {  //Wait for connection
        delay(500);
        Serial.println("Waiting to connect...");
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP()); //Print the local IP
        return true;
    }
    else{
        return false;
    }
}

void setup(void)
{
    Serial.begin(115200);

    // enable LM75 and BME280/BME680 if you use mec-kon's weather station (https://github.com/mec-kon/weather_station)
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    if(setup_wifi()){
        client.setServer(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT);
        client.setCallback(on_message);

        bool mqtt_connected = true;

        if (client.connect(MQTT_CLIENT_ID, MQTT_CLIENT, MQTT_CLIENT_PASSWORD)) {
            Serial.println("mqtt connected");
        }


        if (!client.connected()) {
            mqtt_connected = reconnect();
        }

        if(mqtt_connected){
            client.loop();
            publish_sensor_data();
            publish_battery_level();

        }
    }

    // disable LM75 and BME280/BME680
    digitalWrite(12, LOW);

    delay(1000);
    ESP.deepSleep(10 * MINUTE);

}


void loop(){

}
