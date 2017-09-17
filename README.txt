
playground for a arduino based wifi display

	we are using a 128x64 oled via i2c on a wemos d1 mini
	physical pin-outs match: d3/d4 for SDA/SLC

	this software uses some dependencies:
	(usually a zip-download from the repo is sufficient
	as added library)

	https://github.com/squix78/esp8266-oled-ssd1306
		oled driver

	https://github.com/knolleary/pubsubclient
		mqtt library

	for use with PlatformIO:
    	pio lib install PubSubClient ESP8266_SSD1306
