SHELL = bash
.ONESHELL:

monitor:
	. ./t.sh
	idf.py -p /dev/ttyUSB0 monitor

build1:
	. ./t.sh
	cd esp_http_client
	idf.py fullclean
	idf.py build
	idf.py -p /dev/ttyUSB0 flash
	idf.py -p /dev/ttyUSB0 monitor

build:
	. ./t.sh
	cd esp_http_client
	idf.py build
	idf.py -p /dev/ttyUSB0 flash
	idf.py -p /dev/ttyUSB0 monitor

ch:
	sudo chmod 777 /dev/ttyUSB0

menu:
	. ./t.sh
	cd esp_http_client
	idf.py -p /dev/ttyUSB0 menuconfig
