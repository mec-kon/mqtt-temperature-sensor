# mqtt-weather_station
#### This is a simple program for an ESP8266 to publish a temperature measured by an LM75 sensor via mqtt.
#### In addition, temperature, humidity, altitude and air pressure can be published, which are detected by a BME280 sensor.

### This program code can be used with <a href="https://github.com/mec-kon/weather_station" target="_blank">mec-kon's weather station</a>.

## Download ##
Download the server ```git clone https://github.com/mec-kon/mqtt-weather_station.git```  
Then navigate into the server folder with ```cd mqtt-weather_station```.

Config
--------
```platformio init --board esp12e```  
```pio lib install "adafruit/Adafruit BME280 Library"```
```pio lib install "knolleary/PubSubClient"```  
```pio lib install "tzapu/WiFiManager"```
