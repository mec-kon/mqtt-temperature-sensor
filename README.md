# mqtt-temperature-sensor
#### This is a simple program for an ESP8266 to publish a temperature measured by an LM75 sensor via mqtt

## Download ##
Download the server ```git clone https://github.com/mec-kon/mqtt-temperature-sensor.git```  
Then navigate into the server folder with ```cd mqtt-temperature-sensor```.

Config
--------
```platformio init --board esp12e```  
```pio lib install "adafruit/Adafruit BME280 Library```  
```pio lib install "knolleary/PubSubClient"```
