# ESP32-DHT22-Mosquitto-Client-Server-System
This README is intended to be a brief tutorial on how to get a sensor running on an ESP32 Dev Board platform or similar to transmit time series data using Mosquitto MQTT to a Raspberry Pi or other suitable Linux-based web server and plot the data using Grafana.  This repo includes the ESP32 client code written using the Arduino IDE platform and the JSON file that generates the Grafana Dashboard that plots temperature, relative humidity, and temperature-dewpoint spread.  (The latter parameter is of interest to pilots and weather guessers for predicting haze and fog.)  This tutorial assumes you have at least a basic familiarity with the Raspberry Pi OS.  It assumes you have either have a computer setup to SSH into the Pi or you have a keyboard, mouse, and monitor connected to the Pi.  You will also need to use a web browser either running on the Pi or on another computer to complete the software setup on the Pi.

## ESP32 Hardware Setup
The hardware setup I used is an ESP32-DevKitC-32E Development Board, which I purchased from Amazon.com for about US$11.00.  The sensor I used is a HiLetgo 2pcs DHT22/AM2302 Digital Temperature And Humidity Sensor Module Temperature Humidity Monitor Sensor, which comes with cables to connect it to the ESP32 board, and which I purchased on Amazon.com for about US$14.00 for two.  I connected 3.3V on the ESP32 board to 3.3V on the DHT22, GND on the ESP32 board to GND on the DHT22, and pin 4 on the ESP32 board to Out (which might be called Data or something else) on the DHT22.
## ESP32 Dev Board software
The software source code is contained in ESP32-DHT22_Mosquitto_Client_v0.2.ino.  The header file secrets.h contains, *inter alia*, the Wi-Fi access point SSID and password and the MQTT server's IP address.

Comments in the code should help make it readable by experienced Arduino IDE users.  Consequently, I won't elaborate on the code in detail here.

## Raspberry Pi IoT Server using Docker Containers for MQTT, NodeRED, InfluxDB, and Grafana
In 2022, Albert Harmon published a [video](https://www.youtube.com/watch?v=_DO2wHI6JWQ) and [tutorial](https://learnembeddedsystems.co.uk/easy-raspberry-pi-iot-server) on how to put together a Raspberry Pi-based IoT server that:
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
     - From "storage," choose "influxdb out" (not "influxdb in") and drag it onto the Flow grid to the right of the "function" node.
     - Connect the "mqtt in" node to the "function" node by dragging the dot on the right end of the "mqtt in" node over to the dot on the left end of the "function" node.  There should now be a line connecting those two 
       nodes together.
     - Connect the "function" node to the "influxdb out" node in a similar manner.
15.  Next, we need to configure the "mqtt in" node.  When you double-click on the "mqtt in" node, you should see a form come up on the screen that looks like the screenshow immediately following this line.  Fill yours in similarly to the way I did mine.  You may need to change the Topic if you changed it in your ESP32 sketch.
  
![Screenshot 2025-06-07 151050](https://github.com/user-attachments/assets/46bdc05b-0487-43d1-b304-df7245d188b8)

To the right of the "Server" box in the form above, you should see a pencil icon.  Click on it, and the following form should come up.  Fill it in the way I did mine.  Mosquitto defaults to the MQTT V3.1.1 protocol.  Once you're done filling in the form, click on the red "Update" button.  Then click on the red "Done" button on the previous form.

![Screenshot 2025-06-07 151801](https://github.com/user-attachments/assets/c6e794b6-9421-409b-a0c1-d37dd98e79d2)

16.  Next we'll work on the "formula" node.  The "formula" node reformats the MQTT data from Mosquitto into the format we need for InfluxDB. In the form below, I only changed things in the "On Message" tab. Here's the way I filled out this form:     

![Screenshot 2025-06-07 153031](https://github.com/user-attachments/assets/4cf9ae72-3d72-45fe-8369-f9bc2d9f59c9)

     Once you've filled out the form, click on the red "Done" button.

17.  The final step in the NodeRED setup is to configure the "influxdb out" node.  My form looks like this:

![Screenshot 2025-06-07 153420](https://github.com/user-attachments/assets/72dc5a5d-dd1d-473e-9959-71ee098ea356)

Note that the Server must be of the v1.8-flux variety, based on the version of InfluxDB that IOTStack installed on my Pi.

18.  Now navigate your browser to <your_Raspberry_Pi_IP_Address>:3000 to bring up Grafana.  You can login using `admin` as the username and `admin` as the password.  Then you'll be prompted to change the password.  During the initial Grafana setup, you'll have to add a data source, specifically, InfluxDB.  
19.  To do this, you'll need to go into the InfluxDB Data source configuration page and tell InfluxDB where to get the data.  You specifiy that in the URL box of the HTTP section.  Then scroll down on the InfluxDB setup page.  Under theInfluxDB Details heading, in the Datbase box, enter the name of your database.  In my case, it's "enviro_data," which is what I called it in Step 17 above.  At this point, you'll want to have your ESP32 up and running to feed data to the Pi.  After you've done this, click on the "Save & test" button at the bottom of the page.  If everything has worked properly, a green box should appear and say something like "datasource is working.  3 measurements found" (_e.g._, temperature, humidity, and dewpoint).
20.  Once you have your data source up and running all the way from your ESP32 through the Mosquitto broker, NodeRED, and InfluxDB to Grafana, you can proceed to building a dashboard in Grafana to display your data.  So,
     - Create a new Dashboard,
     - Click on the blue "Add visualization" button,
     - Select "influxdb" as your data source,
     - In the Queries section of the data source setup, on the "From" line, click on "select measurement" and then click on the name of your database,
     - on the "Select" line, click on the field you want to chart, _e.g._, "temperature".
You should now see a plot of your temeperature data, like the screenshot below (click on the figure to enlarge it in a new tab):

![Screenshot 2025-06-07 163704](https://github.com/user-attachments/assets/b082994a-a04d-4fb1-bcff-d60ffd5ff15c)

21.  Now, in the Visualization panel on the right-hand side of the screen depicted above, you can configure how you want your data displayed.  I don't have any sage advice on that topic, other than explore the various settings and settle on what pleases you (or your customer).

### Troubleshooting
If your data are getting lost somewhere between your ESP32 and Grafana, the first step is to find out how far your data goes before it falls into a bit bucket somewhere.  Mr. Harmon's website referenced above has some useful hints.  One thing I found useful when I was struggling to get this working last night is to use "dewbug" nodes in NodeRED.  They're in the "common" section of nodes on the left-hand side of the NodeRED screen.  You can connect them to the outputs of the MQTT node and the function node and see what data are flowing through NodeRED using the NodeRED debug panel, which you have to bring up, because it isn't there by default.  To bring up the debug panel, click on the Debug messages button (bug icon) in the upper right corner of the NodeRED screen, or press '<CTRL>-g', followed by 'd'.

Mr. Harmon suggests that to check to see if data is being written into your InfluxDB database, you can use the following commands in a Raspberry Pi terminal window:
```
docker exec -it influxdb influx
USE sensor_data
show measurements
select * from sensor_data
quit
```
