
## Displaino - An arduino based wireless display.

### Abstract

This software runs on Arduino, communicates using MQTT
protocol and controls an attached display.

We are using a Wemos D1 mini powered by ESP8266 and talk
i2c to an attached OLED display.
Displays:

	* currently: 160x128 color oled (NHD).
	* previously: 128x64 oled via i2c

This software uses some dependencies:

	(usually a zip-download from the repo is sufficient
	as added library)

	https://github.com/squix78/esp8266-oled-ssd1306
		oled driver

	https://github.com/knolleary/pubsubclient
		mqtt library

	for use with PlatformIO:
    	pio lib install PubSubClient ESP8266_SSD1306

### Building

The code in this project can be built with platform io (pio).
Instead of downloading and installing dependencies you can use
docker:

	docker run --name pio --rm -ti -v $HOME:/home/displaino eclipse/platformio

Then ssh or exec into the container and compile:

	docker exec -ti -w /home/displaino pio bash
		pio run

