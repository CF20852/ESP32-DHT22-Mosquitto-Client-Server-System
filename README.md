# ESP32-DHT22-Mosquitto-Client-Server-System
This README is intended to be a brief tutorial on how to get a sensor running on an ESP32 Dev Board platform or similar to transmit time series data using Mosquitto MQTT to a Raspberry Pi or other suitable Linux-based web server and plot the data using Grafana.  This repo includes the ESP32 client code written using the Arduino IDE platform and the JSON file that generates the Grafana Dashboard that plots temperature, relative humidity, and temperature-dewpoint spread.  (The latter parameter is of interest to pilots and weather guessers for predicting haze and fog.)  This tutorial assumes you have at least a basic familiarity with the Raspberry Pi OS.  It assumes you have either have a computer set up to SSH into the Pi or you have a keyboard, mouse, and monitor connected to the Pi.  You will also need to use a web browser either running on the Pi or on another computer to complete the software setup on the Pi.

## _Caveat Lector_ (Reader Beware)
I wrote this README on 7 June 2025.  It is based on the hardware and software available during the past few days.  These instructions may need to be updated if updates in the hardware and software occur, but I do not commit myself to keeping this README up-to-date.

## ESP32 Hardware Setup
The hardware setup I used is an ESP32-DevKitC-32E Development Board, which I purchased from Amazon.com for about US$11.00.  The sensor I used is a HiLetgo 2pcs DHT22/AM2302 Digital Temperature And Humidity Sensor Module Temperature Humidity Monitor Sensor, which comes with cables to connect it to the ESP32 board, and which I purchased on Amazon.com for about US$14.00 for two.  I connected 3.3V on the ESP32 board to 3.3V on the DHT22, GND on the ESP32 board to GND on the DHT22, and pin 4 on the ESP32 board to Out (which might be called Data or something else) on the DHT22.

You can also use this with a BME280 or similar sensor.  Connect [ESP32 pin] - [BME280 pin] as follows:  3.3V to 3.3V, GND to GND, GPIO21/I2C SDA to SDA, and GPIO22/I2C SCL to SCL.

## ESP32 Dev Board software
The software source code for the DHT22 version is contained in ESP32-DHT22_Mosquitto_Client_v0.2.ino.  The software source code for the BME280 version is contained in ESP32-BME280_Mosquitto_Client_v0.2.ino.  The header file secrets.h contains, *inter alia*, the Wi-Fi access point SSID and password and the MQTT server's IP address.

Comments in the code should help make it readable by experienced Arduino IDE users.  Consequently, I won't elaborate on the code in detail here.

## Raspberry Pi IoT Server using Docker Containers for MQTT, Node-RED, InfluxDB, and Grafana
In 2022, Albert Harmon published a [video](https://www.youtube.com/watch?v=_DO2wHI6JWQ) and [tutorial](https://learnembeddedsystems.co.uk/easy-raspberry-pi-iot-server) on how to put together a Raspberry Pi-based IoT server that:
- Uses **Mosquitto** as an MQTT broker;
- Uses **Node-RED** to set up how data published to Mosquitto are formatted and routed to **InfluxDB**, a time-series database program; and
- Uses **Grafana** to pull data from the InfluxDB database and display the data in charts.

The installation procedure uses **IOTStack** to create **Docker** containers for Mosquitto, Node-RED, InfluxDB, and Grafana and facilitate the installation process.
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
7.  Start the build of the docker-compose.yml file by pressing [Enter].
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
12.  For the remainder of the setup, you'll use a browser to connect to Node-RED and Grafana.  Node-RED is at <your_Raspberry_Pi_IP_Address>:1880, and Grafana is at <your_Raspberry_Pi_IP_Address>:3000.  Node-RED is used to set up the data transfer from Mosquitto to InfluxDB.  Grafana is used to display the data in table, time series, bar chart, or whatever format that Grafana provides and you like.  Let's start with Node-RED.
13.  In your browser, navigate to <your_Raspberry_Pi_IP_Address>:1880.  (That's port 1880 on your Raspberry Pi IP address.)  Let's hope there isn't a firewall between your browser and that port on the Pi.  If there is, you might have to reconfigure your firewall to let traffic through that port.  How to do that is _way_ beyond the scope of this document.
14.  The GUI for Node-RED should now be in your browser window.  On the left-hand side, you'll see a lengthy list of "nodes," grouped under headings like "common," "function," "network," "sequence," "parser," "storage," etc.
     - From "network," choose "mqtt in" and drag it onto the Flow grid.
     - From "function," choose "function" and drag it onto the Flow grid to the right of the "mqtt in" node.
     - From "storage," choose "influxdb out" (not "influxdb in") and drag it onto the Flow grid to the right of the "function" node.
     - Connect the "mqtt in" node to the "function" node by dragging the dot on the right end of the "mqtt in" node over to the dot on the left end of the "function" node.  There should now be a line connecting those two 
       nodes together.
     - Connect the "function" node to the "influxdb out" node in a similar manner.
15.  Next, we need to configure the "mqtt in" node.  When you double-click on the "mqtt in" node, you should see a form come up on the screen that looks like the screenshow immediately following this paragraph.  Fill yours in similarly to the way I did mine.  You may need to change the Topic if you changed it in your ESP32 sketch.
  
![Screenshot 2025-06-07 151050](https://github.com/user-attachments/assets/46bdc05b-0487-43d1-b304-df7245d188b8)

To the right of the "Server" box in the form above, you should see a pencil icon.  Click on it, and the following form should come up.  Fill it in the way I did mine.  Mosquitto defaults to the MQTT V3.1.1 protocol.  Once you're done filling in the form, click on the red "Update" button.  Then click on the red "Done" button on the previous form.

![Screenshot 2025-06-07 151801](https://github.com/user-attachments/assets/c6e794b6-9421-409b-a0c1-d37dd98e79d2)

16.  Next we'll work on the "formula" node.  The "formula" node reformats the MQTT data from Mosquitto into the format we need for InfluxDB. In the form below, I only changed things in the "On Message" tab. Here's the way I filled out this form:     

![Screenshot 2025-06-07 153031](https://github.com/user-attachments/assets/4cf9ae72-3d72-45fe-8369-f9bc2d9f59c9)

Once you've filled out the form, click on the red "Done" button.

17.  The final step in the Node-RED setup is to configure the "influxdb out" node.  My form looks like this:

![Screenshot 2025-06-07 153420](https://github.com/user-attachments/assets/72dc5a5d-dd1d-473e-9959-71ee098ea356)

Note that the Server must be of the v1.8-flux variety, based on the version of InfluxDB that IOTStack installed on my Pi.

When you're done, your Node-RED flow should look like the figure below.  Note that I added a couple of "debug" nodes while I was trying to get this to work:

![Screenshot 2025-06-07 165735](https://github.com/user-attachments/assets/e3c711df-2494-462f-838a-3b9ca8e1ddd1)

I've included in this repo the JSON file I exported from Node-RED for this system.  I don't know if it will do you any good, because there are a lot of IDs that I don't understand, and they may be unique to my setup.  The file may, however, provide a useful point of comparison if you need to debug your setup.

18.  Now navigate your browser to <your_Raspberry_Pi_IP_Address>:3000 to bring up Grafana.  You can login using `admin` as the username and `admin` as the password.  Then you'll be prompted to change the password.  During the initial Grafana setup, you'll have to add a data source, specifically, InfluxDB.  
19.  To do this, you'll need to go into the InfluxDB Data source configuration page and tell InfluxDB where to get the data.  You specifiy that in the URL box of the HTTP section.  Then scroll down on the InfluxDB setup page.  Under the InfluxDB Details heading, in the Datbase box, enter the name of your database.  In my case, it's "enviro_data," which is what I called it in Step 17 above.  At this point, you'll want to have your ESP32 up and running to feed data to the Pi.  After you've done this, click on the "Save & test" button at the bottom of the page.  If everything has worked properly, a green box should appear and say something like "datasource is working.  3 measurements found" (_e.g._, temperature, humidity, and dewpoint).
20.  Once you have your data source up and running all the way from your ESP32 through the Mosquitto broker, Node-RED, and InfluxDB to Grafana, you can proceed to building a dashboard in Grafana to display your data.  So,
     - Create a new Dashboard,
     - Click on the blue "Add visualization" button,
     - Select "influxdb" as your data source,
     - In the Queries section of the data source setup, on the "From" line, click on "select measurement" and then click on the name of your database,
     - on the "Select" line, click on the field you want to chart, _e.g._, "temperature".
You should now see a plot of your temeperature data, like the screenshot below (click on the figure to enlarge it in a new tab):

![Screenshot 2025-06-07 163704](https://github.com/user-attachments/assets/f98ab658-6097-4b4e-a101-c35a708cabdc)


21.  Now, in the Visualization panel on the right-hand side of the screen depicted above, you can configure how you want your data displayed.  I don't have any sage advice on that topic, other than explore the various settings and settle on what pleases you (or your customer).  Included in this repo is a copy of my Grafana visualization JSON code, which you could try importing if all else fails.

### Troubleshooting
If your data are getting lost somewhere between your ESP32 and Grafana, the first step is to find out how far your data goes before it falls into a bit bucket somewhere.  Mr. Harmon's website referenced above has some useful hints.  One thing I found useful when I was struggling to get this working last night is to use "debug" nodes in Node-RED.  They're in the "common" section of nodes on the left-hand side of the Node-RED screen.  You can connect them to the outputs of the MQTT node and the function node and see what data are flowing through Node-RED using the Node-RED debug panel, which you have to bring up, because it isn't there by default.  To bring up the debug panel, click on the Debug messages button (bug icon) in the upper right corner of the Node-RED screen, or press '<CTRL>-g', followed by 'd'.

Mr. Harmon suggests that to check to see if data is being written into your InfluxDB database, you can use the following commands in a Raspberry Pi terminal window:
```
docker exec -it influxdb influx
USE sensor_data
show measurements
select * from sensor_data
quit
```
### Cloning with Changes
I decided I wanted another temperature and humidity monitoring system in my garage.  I was able to quickly clone the hangar system using the following steps:
1.  Change "hangar" in the ESP32 sketch to "garage" in six places.  Flash a new ESP32 with a DHT22 attached.
2.  Export Flow 1 in Node-RED to the clipboard.  Create a new flow, and paste the flow exported from Flow 1 into it.  Rename Flow 1 as Hangar Flow and Flow 2 as Garage Flow.
3.  In Garage Flow, edit the MQTT node to replace "hangar" with "garage".
4.  In Garage Flow, edit the influxdb out node to replace "enviro" with "garage".
5.  Create a new database called "garage_data" in InfluxDB, see Steps 10 and 11 above.
6.  In Grafana, create a new data source.  I called it "influxdb-garage".
7.  In Grafana, make a copy of the hangar dashboard, and rename it "Garage Temperature and Humidity".  In each panel, edit the data source to be "influxdb-garage," and in each query, change "enviro_data" to "garage_data".

That's all it took.  Took me less than half an hour

### Moving the InfluxDB Database Files to a SSD
You might start this project storing the database files on the Raspberry Pi microSD card, and then become concerned about wearing out the microSD card prematurely due to the number of writes to the database.  Modern SD cards use wear-leveling algorithms to spread writes evenly across the memory, which helps extend their usable life.  But there are several grades of SD cards with different write cycle endurance levels.  If you must leave your database on the SD card, consider a high-endurance SD card such as one designed for a dashcam, where the video gets recorded over many times.  And you can use a card with higher capacity (_e.g._, 64 GB even if you only need 8 GB for the Raspberry Pi OS and the IOTstack programs) to spread the writes out over a larger address space.  Remember that even if your data only occupies one byte, an entire sector, _e.g._, 512 bytes, will be written to the card or drive at each update.

Alternatively, you can move the database files onto a solid-state drive (SSD).  The SSD I chose has a capacity of 128 GB and a life expectancy of 50 terabytes written.  Even if I update two sector's worth of data twice a second, the drive should last over 773 years, which is probably longer than it will take the plastic parts of the Pi and its case to turn to dust.

The following procedure was developed in a conversation between the author and Claude.AI, in which the AI suggested steps and I gave it feedback on what didn't work as it expected.  During this conversation, I shared my /home/pi/IOTstack/docker-compose.yml file with it.  This file contains the environment variables set up during the IOTstack installation process, and was key to figuring out where the InfluxDB database files were stored.  My setup uses bind mounts (./volumes/influxdb/data) rather than named Docker volumes, so the data is directly in my IOTstack directory structure.

Here's the procedure:
#### Installing and Setting Up the SSD
If you have already installed your SSD, you can proceed to the next section, but you might want to review this section first.

1.  If you have just installed a blank, unformatted SSD, first check to see if the Raspberry Pi OS recognizes it, and find out what it's name is:
```
sudo lsblk
```
You should see something like this:
```
mmcblk0     179:0    0  14.8G  0 disk
├─mmcblk0p1 179:1    0   512M  0 part /boot/firmware
└─mmcblk0p2 179:2    0  14.3G  0 part /
nvme0n1     259:0    0 119.2G  0 disk
```
In my case, the SSD is /dev/nvme0n1.  The 16 GB SD card is /dev/mmcblk0, and has two partitions, p1 and p2.

2.  Create a partition table for the SSD:
```
# Create a partition table (this will erase everything!)
sudo fdisk /dev/nvme0n1
```
In fdisk:

Press n for new partition

Press p for primary

Press 1 for partition number

Press Enter twice to use default start/end

Press w to write and exit

3.  Create a filesystem on the SSD, preferably using the ext4 format:
```
# Format as ext4
sudo mkfs.ext4 /dev/nvme0n1p1

# Add a label (optional but helpful)
sudo e2label /dev/nvme0n1p1 "InfluxDB-SSD"
```
4.  Mount a partition
```
# Mount the first partition (note the p1)
sudo mount /dev/nvme0n1p1 /mnt/ssd

# Verify it worked
df -h | grep ssd
ls -la /mnt/ssd/
```
5.  Get UUID for fstab
```
sudo blkid /dev/nvme0n1p1
```
You should see something like:
```
/dev/nvme0n1p1: LABEL="Transcend_128GB" UUID="86de2fd8-0e72-4f41-aa7a-3f2bd68bd273" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="c10adaeb-01"
```
6.  Use your favorite editor to add this line to /etc/fstab:
```
UUID=your-actual-uuid-here /mnt/ssd ext4 defaults,nofail 0 2
```

#### Moving the InfluxDB Database Files to the SSD
1.  Stop IOTstack
```
cd /home/pi/IOTstack
docker-compose down
```
2.  Verify Current Data Location
```
ls -la /home/pi/IOTstack/volumes/influxdb/data/
```
You should see the InfluxDB files and directories here.

3.  Copy Data to SSD
```
# Make sure SSD is mounted and directory exists
sudo mkdir -p /mnt/ssd/influxdb/data
sudo mkdir -p /mnt/ssd/influxdb/backup

# Copy the data (this is the correct path from your compose file)
sudo cp -r /home/pi/IOTstack/volumes/influxdb/data/* /mnt/ssd/influxdb/data/
sudo cp -r /home/pi/IOTstack/backups/influxdb/db/* /mnt/ssd/influxdb/backup/ 2>/dev/null || echo "No backup files to copy"
```
4.  Set Permissions
```
# InfluxDB runs as user influxdb (usually UID 999)
sudo chown -R 999:999 /mnt/ssd/influxdb/
```
5.  Update docker-compose.yml
Edit your /home/pi/IOTstack/docker-compose.yml and change the InfluxDB volumes section:
```
volumes:
- /mnt/ssd/influxdb/data:/var/lib/influxdb
- /mnt/ssd/influxdb/backup:/var/lib/influxdb/backup
```
6.  Start IOTstack
```
cd /home/pi/IOTstack
docker-compose up -d
```
7.  Verify
```
# Check if InfluxDB started correctly
docker logs influxdb

# Check if data is accessible
docker exec influxdb influx -execute 'SHOW DATABASES'
```
8.  Finally, check to make sure your data is getting from your sensor device(s) to Grafana.
   
9.  OPTIONALLY... you can remove the InfluxDB database files from your SD card, after you make sure everything is working properly with the files on the SSD:
```
# Remove old InfluxDB data (be very careful with this!)
# Only do this after confirming everything works
sudo rm -rf /home/pi/IOTstack/volumes/influxdb/data/*
```
