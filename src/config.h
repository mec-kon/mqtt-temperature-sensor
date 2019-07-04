#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define WLAN_SSID "ssid"
#define WLAN_PASSWORD "passw0rd"

#define MQTT_SERVER_ADDRESS "192.168.1.10"
#define MQTT_SERVER_PORT 1883
#define MQTT_CLIENT_ID "lm75_temperature_sensor"
#define MQTT_CLIENT "client-name"
#define MQTT_CLIENT_PASSWORD "passw0rd"

#define MQTT_PUBLISH_TOPIC "/home/temperature"

static const uint8_t I2C_SDA = D6;
static const uint8_t I2C_SCL = D7;


#endif // CONFIG_H
