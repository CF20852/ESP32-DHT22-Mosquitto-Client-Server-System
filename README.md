# ESP32-DHT22-Mosquitto-Client-Server-System
This README is intended to be a brief tutorial on how to get a sensor running on an ESP32 Dev Board platform or similar to transmit time series data using Mosquitto MQTT to a Raspberry Pi or other suitable Linux-based web server and plot the data using Grafana.  This repo includes the ESP32 client code written using the Arduino IDE platform and the JSON file that generates the Grafana Dashboard that plots temperature, relative humidity, and temperature-dewpoint spread.  (The latter parameter is of interest to pilots and weather guessers for predicting haze and fog.)  This tutorial assumes you have at least a basic familiarity with the Raspberry Pi OS.  It assumes you have either have a computer setup to SSH into the Pi or you have a keyboard, mouse, and monitor connected to the Pi.  You will also need to use a web browser either running on the Pi or on another computer to complete the software setup on the Pi.

## ESP32 Hardware Setup
The hardware setup I used is an ESP32-DevKitC-32E Development Board, which I purchased from Amazon.com for about US$11.00.  The sensor I used is a HiLetgo 2pcs DHT22/AM2302 Digital Temperature And Humidity Sensor Module Temperature Humidity Monitor Sensor, which comes with cables to connect it to the ESP32 board, and which I purchased on Amazon.com for about US$14.00 for two.  I connected 3.3V on the ESP32 board to 3.3V on the DHT22, GND on the ESP32 board to GND on the DHT22, and pin 4 on the ESP32 board to Out (which might be called Data or something else) on the DHT22.
## ESP32 Dev Board software
The software source code is contained in ESP32-DHT22_Mosquitto_Client_v0.2.ino.  The header file secrets.h contains, *inter alia*, the Wi-Fi access point SSID and password and the MQTT server's IP address.

Comments in the code should help make it readable by experienced Arduino IDE users.  Consequently, I won't elaborate on the code in detail here.

## Raspberry Pi IoT Server using Docker Containers for MQTT, NodeRED, InfluxDB, and Grafana
In 2022, Albert Harmon published a [tutorial](https://learnembeddedsystems.co.uk/easy-raspberry-pi-iot-server) on how to put together a Raspberry Pi-based IoT server that:
- Uses **Mosquitto** as an MQTT broker;
- Uses **NodeRED** to set up how data published to Mosquitto are formatted and routed to **InfluxDB**, a time-series database program; and
- Uses **Grafana** to pull data from the InfluxDB database and display the data in charts.

The installation procedure uses **IOTStack** to create **Docker** containers for Mosquitto, NodeRED, InfluxDB, and Grafana and facilitate the installation process.
The installation process detailed below follows that of Mr. Harmon and expands upon his process where necessary.
### Installation process
1.  Begin by installing the Raspberry Pi OS on a suitable SD card.
2.  If you're using a headless (no keyboard/mouse/monitor) Pi, open a terminal window using PuTTY or the SSH tool of your choice.  If you're using a full OS installation with a keyboard, mouse, and monitor, open a terminal window.  Then update the OS using the following commands:
```
sudo apt get
sudo apt full-upgrade -y
```
3.  Next, download and install IOTStack, and reboot the Pi:
```
curl -fsSL https://raw.githubusercontent.com/SensorsIot/IOTstack/master/install.sh | bash
sudo reboot
```
4.  Now assuming you're in the home/pi folder on the Pi, start IOTStack using the following commands:
```
cd IOTstack/
./menu.sh
```
5.  The IOTStack Main Menu should now be displayed.  Select the top menu item, `->Build Stack<-` and press [Enter] to open the next-level menu.
6.  You'll be presented with a box with the heading `Select containers to build`.  The Controls are listed at the bottom of that box.  Scroll down (and up if necessary) and select:
```
- grafana
- influxdb
- mosquitto
- nodered
```
7.  Start the build of the docker-compose.yml fileby pressing [Enter].
8.  Now select `->Docker Commands<-` in the IOTStack main menu, and press [Enter] to install the Docker containers and the four programs you selected in step 6.  The Pi will download and install those four programs, which may take some time.
9.  Once installation is complete, you can use `docker-compose ps` to see that the four programs are running.
10.  The next step is to create a timeseries database in InfluxDB.  Because InfluxDB is running in a Docker container, you'll need to start Influx through Docker, as follow:
```
docker exec -it influxdb influx
```
To understand this command, you can refer to the `docker exec` documentation [here](https://docs.docker.com/reference/cli/docker/container/exec/).
11. Now you should have a command prompt from Influx.  Use the following commands to create the database, and then quit Influx:
```
CREATE DATABASE <your_database_name>
quit
```
12.  For the remainder of the setup, you'll use a browser to connect to NodeRED and Grafana.  NodeRED is at <your_Raspberry_Pi_IP_Address>:1880, and Grafana is at <your_Raspberry_Pi_IP_Address>:3000.  NodeRED is used to set up the data transfer from Mosquitto to InfluxDB.  Grafana is used to display the data in table, time series, bar chart, or whatever format that Grafana provides and you like.  Let's start with NodeRED.
13.  In your browser, navigate to <your_Raspberry_Pi_IP_Address>:1880.  (That's port 1880 on your Raspberry Pi IP address.)  Let's hope there isn't a firewall between your browser and that port on the Pi.  If there is, you might have to reconfigure your firewall to let traffic through that port.  How to do that is _way_ beyond the scope of this document.
14.  The GUI for NodeRED should now be in your browser window.  On the left-hand side, you'll see a lengthy list of "nodes," grouped under headings like "common," "function," "network," "sequence," "parser," "storage," etc.
     - From "network," choose "mqtt in" and drag it onto the Flow grid.
     - From "function," choose "function" and drag it onto the Flow grid to the right of the "mqtt in" node.
     - From "storage," choose "influxdb in" (not "influxdb out") and drag it onto the Flow grid to the right of the "function" node.
     - Connect the "mqtt in" node to the "function" node by dragging the dot on the right end of the "mqtt in" node over to the dot on the left end of the "function" node.  There should now be a line connecting those two 
       nodes together.
     - Connect the "function" node to the "influxdb in" node in a similar manner.
15.  Next, we need to configure the "mqtt in" node:
     - 
