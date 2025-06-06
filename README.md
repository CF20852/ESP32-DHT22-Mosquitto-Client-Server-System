# ESP32-DHT22-Mosquitto-Client-Server-System
This README is intended to be a brief tutorial on how to get a sensor running on an ESP32 Dev Board platform or similar to transmit time series data using Mosquitto MQTT to a Raspberry Pi or other suitable Linux-based web server and plot the data using Grafana.  This repo includes the ESP32 client code written using the Arduino IDE platform and the JSON file that generates the Grafana Dashboard that plots temperature, relative humidity, and temperature-dewpoint spread.  (The latter parameter is of interest to pilots and weather guessers for predicting haze and fog.)
## ESP32 Dev Board software
The software source code is contained in ESP32-DHT22_Mosquitto_Client_v0.2.ino.  The header file secrets.h contains, *inter alia*, the Wi-Fi access point SSID and password and the MQTT server's IP address.

Comments in the code should help make it readable by experienced Arduino IDE users.  Consequently, I won't elaborate on the code in detail here.
