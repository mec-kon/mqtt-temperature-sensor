#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "LM75A.h"
#include "PubSubClient.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "WiFiManager.h"
#include "EEPROM.h"




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

struct {
    char mqtt_server_address[16] = MQTT_SERVER_ADDRESS;
    char mqtt_server_port[5] = MQTT_SERVER_PORT;

    char mqtt_username[40] = MQTT_CLIENT;
    char mqtt_password[40] = MQTT_CLIENT_PASSWORD;


    char mqtt_publish_topic_lm75_temperature[55] = MQTT_PUBLISH_TOPIC_LM75_TEMPERATURE;
    char mqtt_publish_topic_bme280_temperature[60] = MQTT_PUBLISH_TOPIC_BME280_TEMPERATURE;
    char mqtt_publish_topic_bme280_pressure[55] = MQTT_PUBLISH_TOPIC_BME280_PRESSURE;
    char mqtt_publish_topic_bme280_altitude[55] = MQTT_PUBLISH_TOPIC_BME280_ALTITUDE;
    char mqtt_publish_topic_bme280_humidity[55] = MQTT_PUBLISH_TOPIC_BME280_HUMIDITY;
    char mqtt_publish_topic_battery_level[55] = MQTT_PUBLISH_TOPIC_BATTERY_LEVEL;
    char mqtt_publish_topic_battery_raw_data[55] = MQTT_PUBLISH_TOPIC_BATTERY_RAW_DATA;
} mqtt_config;



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
        if (client.connect(MQTT_CLIENT_ID, mqtt_config.mqtt_username, mqtt_config.mqtt_password)) {
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
        client.publish(mqtt_config.mqtt_publish_topic_lm75_temperature, String(lm75_temperature_in_degrees).c_str());
    }

    if (!bme280_status) {
        Serial.println("Error while getting bme280 temperature");
        String error_message = "bme280_error";
        error_message.toCharArray(error_msg, 12);
    }
    else {
        client.publish(mqtt_config.mqtt_publish_topic_bme280_temperature, String(bme280_temperature_in_degrees).c_str());
        client.publish(mqtt_config.mqtt_publish_topic_bme280_pressure, String(bme280_pressure).c_str());
        client.publish(mqtt_config.mqtt_publish_topic_bme280_altitude, String(bme280_altitude).c_str());
        client.publish(mqtt_config.mqtt_publish_topic_bme280_humidity, String(bme280_humidity).c_str());
    }
}

void publish_battery_level(){
    int battery_voltage = analogRead(A0);
    float battery_level = (battery_voltage - BATTERY_MIN) * (100.0/ (BATTERY_MAX - BATTERY_MIN));
    client.publish(mqtt_config.mqtt_publish_topic_battery_level, String(battery_level).c_str());
    client.publish(mqtt_config.mqtt_publish_topic_battery_raw_data, String(battery_voltage).c_str());

}





bool config_changed = false;
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  config_changed = true;
}

bool setup_wifi() {
    WiFiManager wifi_manager;
    wifi_manager.setDebugOutput(false);
    wifi_manager.setSaveConfigCallback(saveConfigCallback);

    //wifi_manager.resetSettings();

    // add custom mqtt parameters
    // id/name, placeholder/prompt, default, length
    WiFiManagerParameter mqtt_server_address_parameter("mqtt_server_address","mqtt ip address", mqtt_config.mqtt_server_address, 16);
    WiFiManagerParameter mqtt_server_port_parameter("mqtt_server_port", "mqttt port", mqtt_config.mqtt_server_port, 5);
    WiFiManagerParameter mqtt_username_parameter("mqtt_username", "mqtt username", mqtt_config.mqtt_username, 40);
    WiFiManagerParameter mqtt_password_parameter("mqtt_password", "mqtt password", mqtt_config.mqtt_password, 40);

    WiFiManagerParameter mqtt_publish_topic_lm75_temperature_parameter("lm75_temperature_topic", "lm75 temperature topic", mqtt_config.mqtt_publish_topic_lm75_temperature, 55);
    WiFiManagerParameter mqtt_publish_topic_bme280_temperature_parameter("bme280_temperature_topic", "bme280 temperature topic", mqtt_config.mqtt_publish_topic_bme280_temperature, 60);
    WiFiManagerParameter mqtt_publish_topic_bme280_pressure_parameter("bme280_pressure_topic", "bme280 pressure topic", mqtt_config.mqtt_publish_topic_bme280_pressure, 55);
    WiFiManagerParameter mqtt_publish_topic_bme280_altitude_parameter("bme280_altitude_topic", "bme280 altitude topic", mqtt_config.mqtt_publish_topic_bme280_altitude, 55);
    WiFiManagerParameter mqtt_publish_topic_bme280_humidity_parameter("bme280_humidity_topic", "bme280 humidity topic", mqtt_config.mqtt_publish_topic_bme280_humidity, 55);
    WiFiManagerParameter mqtt_publish_topic_battery_level_parameter("battery_level_topic", "battery level topic", mqtt_config.mqtt_publish_topic_battery_level, 55);
    WiFiManagerParameter mqtt_publish_topic_battery_raw_data_parameter("battery_raw_data_topic", "battery raw data topic", mqtt_config.mqtt_publish_topic_battery_raw_data, 55);


    wifi_manager.addParameter(&mqtt_server_address_parameter);
    wifi_manager.addParameter(&mqtt_server_port_parameter);
    wifi_manager.addParameter(&mqtt_username_parameter);
    wifi_manager.addParameter(&mqtt_password_parameter);

    wifi_manager.addParameter(&mqtt_publish_topic_lm75_temperature_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_bme280_temperature_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_bme280_pressure_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_bme280_altitude_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_bme280_humidity_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_battery_level_parameter);
    wifi_manager.addParameter(&mqtt_publish_topic_battery_raw_data_parameter);

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "weather_station_wifi"
    // and goes into a blocking loop awaiting configuration
    wifi_manager.autoConnect("weather_station_wifi");



    strcpy(mqtt_config.mqtt_server_address, mqtt_server_address_parameter.getValue());
    strcpy(mqtt_config.mqtt_server_port, mqtt_server_port_parameter.getValue());
    strcpy(mqtt_config.mqtt_username, mqtt_username_parameter.getValue());
    strcpy(mqtt_config.mqtt_password, mqtt_password_parameter.getValue());

    strcpy(mqtt_config.mqtt_publish_topic_lm75_temperature, mqtt_publish_topic_lm75_temperature_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_bme280_temperature, mqtt_publish_topic_bme280_temperature_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_bme280_pressure, mqtt_publish_topic_bme280_pressure_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_bme280_altitude, mqtt_publish_topic_bme280_altitude_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_bme280_humidity, mqtt_publish_topic_bme280_humidity_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_battery_level, mqtt_publish_topic_battery_level_parameter.getValue());
    strcpy(mqtt_config.mqtt_publish_topic_battery_raw_data, mqtt_publish_topic_battery_raw_data_parameter.getValue());

    EEPROM.begin(512);

    if(config_changed){
        Serial.println("saving EEPROM");
        EEPROM.put(0, mqtt_config);
        EEPROM.commit();
        config_changed = false;
    }
    else{
        EEPROM.get(0, mqtt_config);
        Serial.println("config not changed");
    }

    return true;
}





void setup(void)
{
    Serial.begin(115200);

    // enable LM75 and BME280/BME680 if you use mec-kon's weather station (https://github.com/mec-kon/weather_station)
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    if(setup_wifi()){

        Serial.println("wifi connected");

        client.setServer(mqtt_config.mqtt_server_address, atoi(mqtt_config.mqtt_server_port));
        client.setCallback(on_message);

        bool mqtt_connected = true;

        if (client.connect(MQTT_CLIENT_ID, mqtt_config.mqtt_username, mqtt_config.mqtt_password)) {
            Serial.println("mqtt connected");
        }


        if (!client.connected()) {
            Serial.println("mqtt not connected");
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
