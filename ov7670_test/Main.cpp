/*
 * main.c
 *
 *  Code was inspired from a couple of sources, esp from: http://mbed.org/users/diasea/notebook/ov7670_FIFO_AL422B/
 *
 *  Created on: Jul 21, 2013
 *      Author: arndtjenssen
 */

#include "Main.h"

Ov7670 camera;

uint8_t camera_init_success = false;

void vsync_handler() {
	camera.vsync_handler();
}

void href_handler() {
	camera.href_handler();
}

void cameraBufferFull(uint8_t * buffer) {
	for (uint16_t i = 0; i<camera.BUFFER_SIZE; i+=3) {
		Serial << _BYTE(buffer[i]) << _BYTE(buffer[i+1]) << _BYTE(buffer[i+2]);
	}
}

void setup() {
	Serial.begin(115200);

	// camera
	camera.setSerial(&Serial);
	camera.bufferFullFunctionPtr = &cameraBufferFull;
	camera.init();
	camera_init_success = camera.init_success;

	// TODO: attach interrupt w/o arduino code
	attachInterrupt(VSYNC_INT, vsync_handler, FALLING);
	attachInterrupt(HREF_INT, href_handler, RISING); // not really needed
}

void loop() {
	#ifdef SERIAL_INFO
	checkSerialInput();
	#endif
}


#if defined(SERIAL_INFO) || defined(SIMULATE)
void serialTimeStamp() {
	char buffer[20];

	uint32_t timeNow = millis();

	uint8_t days = timeNow / DAY ;
	uint8_t hours = (timeNow % DAY) / HOUR;
	uint8_t minutes = ((timeNow % DAY) % HOUR) / MINUTE;
	uint8_t seconds = (((timeNow % DAY) % HOUR) % MINUTE) / SECOND;

	// WARN: sprintf takes nearly 1.8K of flash memory
	sprintf(buffer, "%02d:%02d:%02d:%02d", days, hours, minutes, seconds);

	Serial << buffer << "::  ";
}
#endif

#ifdef SERIAL_INFO
int readIntFromSerial() {
	int temp = 0;
	char buf[8];
	uint8_t bufCount = 0;

	for (;;) {
		while (!Serial.available());
		temp = Serial.read();
		Serial << _BYTE(temp);
		if (temp == 13 || bufCount > 6)
			break;
  	buf[bufCount] = temp;
  	bufCount++;
	}

	return atoi(buf);
}

void checkSerialInput() {
	int serial = 0;

	if( Serial.available() ) {
    serial = Serial.read();

    switch (serial) {
    case 'c':
    	camera.capture_image();
    	break;

    case '0':
    	camera.reset(MODE_RGB444);
			#ifdef SERIAL_INFO
    	Serial << F("mode: RGB444") << endl;
			#endif
    	break;

    case '1':
    	camera.reset(MODE_RGB555);
    	Serial << F("mode: RGB555") << endl;
    	break;

    case '2':
    	camera.reset(MODE_RGB565);
    	Serial << F("mode: RGB565") << endl;
    	break;

    case '3':
    	camera.reset(MODE_YUV);
    	Serial << F("mode: YUV") << endl;
    	break;

    }

	}
}
#endif
