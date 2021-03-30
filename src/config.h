#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


#define WLAN_SSID "ssid"
#define WLAN_PASSWORD "passw0rd"

#define MQTT_SERVER_ADDRESS "192.168.1.10"
#define MQTT_SERVER_PORT "1883"
#define MQTT_CLIENT_ID "weather_station"
#define MQTT_CLIENT "client-name"
#define MQTT_CLIENT_PASSWORD "passw0rd"

// default mqtt topics of the individual seonsors
#define MQTT_PUBLISH_TOPIC_LM75_TEMPERATURE "/home/balcony/weather_station/lm75_sensor/temperature"
#define MQTT_PUBLISH_TOPIC_BME280_TEMPERATURE "/home/balcony/weather_station/bme280_sensor/temperature"
#define MQTT_PUBLISH_TOPIC_BME280_PRESSURE "/home/balcony/weather_station/bme280_sensor/pressure"
#define MQTT_PUBLISH_TOPIC_BME280_ALTITUDE "/home/balcony/weather_station/bme280_sensor/altitude"
#define MQTT_PUBLISH_TOPIC_BME280_HUMIDITY "/home/balcony/weather_station/bme280_sensor/humidity"
#define MQTT_PUBLISH_TOPIC_BATTERY_LEVEL "/home/balcony/weather_station/battery_level"
#define MQTT_PUBLISH_TOPIC_BATTERY_RAW_DATA "/home/balcony/weather_station/battery_raw_data"

static const uint8_t I2C_SDA = 4;
static const uint8_t I2C_SCL = 5;

/*
 * Accordingto I2C bus specifications,the LM75Ahas a 7-bitslaveaddress.
 * The four mostsignificantbits of the slaveaddressare hardwiredinsidethe LM75Aand are“1001”.
 * The threeleastsignificantbits of the addressare assigned to pins A2–A0,
 * and are set by connectingthesepins to groundfor a low, (0); or to +VS for a high.
 *
 * 0x48 = 1001000 (A2=0, A1=0, A0=0)
*/
const int LM75A_BASE_ADDRESS = 0x48;

const int BME280_BASE_ADDRESS = 0x76;

#define BATTERY_MIN 770
#define BATTERY_MAX 1024


#endif // CONFIG_H
