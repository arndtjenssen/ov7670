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

#ifdef USE_SD
#include <SdFat.h>
#define SD_CS 14 // PD6
SdFat sd;
SdFile file;
#endif

#ifdef USE_TFT
#include "SPI.h"
#include "TFT.h"

#define CS		0 // PB0
#define DC   	1 // PB1
#define RESET 3 // PB3

TFT screen = TFT(CS, DC, RESET);
volatile uint8_t linesRead = 0;
volatile uint16_t snap_counter = 0;
#endif

void vsync_handler() {
	camera.vsync_handler();
}

void href_handler() {
	camera.href_handler();
}

char * nrToPictureString(uint8_t dir, uint16_t nr) {
	static char buffer[20];

	if (dir == 0) {
		sprintf(buffer, "%s%05u%s", "/snaps/", nr, ".rgb");
	} else {
		sprintf(buffer, "%s%04u%s", "/photos/", nr, ".bmp");
	}

	return buffer;
}


void cameraReadImageStart() {
	#ifdef USE_TFT
	linesRead = 0;
	#endif
	#ifdef USE_SD
	file.open(nrToPictureString(0, snap_counter++), O_WRITE | O_CREAT | O_TRUNC);
	#endif
}

void cameraReadImageStop() {
	#ifdef USE_TFT
	Serial << linesRead << endll;
	#endif
	#ifdef USE_SD
	//file.flush();
	file.close();
	#endif
}

void cameraBufferFull(uint8_t * buffer) {
	#ifdef USE_SD
	file.write(buffer, camera.BUFFER_SIZE);
	#endif

	#ifdef USE_TFT
	uint8_t x = 0;
	for (uint16_t i = 0; i<camera.BUFFER_SIZE; i+=3) {
		screen.drawPixel(x++, linesRead, screen.newColor(buffer[i], buffer[i+1], buffer[i+2]));
	}
	linesRead++;
	#else
	for (uint16_t i = 0; i<camera.BUFFER_SIZE; i+=3) {
		Serial << _BYTE(buffer[i]) << _BYTE(buffer[i+1]) << _BYTE(buffer[i+2]);
	}
	#endif
}

void displayPicture(const char * filename) {
}

void setup() {
	#ifdef USE_TFT
	screen.begin();
	screen.setRotation(3);
	screen.background(0, 0, 0);
	screen.stroke(255,255,255);
	screen.setTextSize(2);
	#endif

	#ifdef USE_SD
	if (!sd.begin(SD_CS, SPI_FULL_SPEED)) sd.initErrorHalt();
	sd.chdir("snaps", false);
	// sd.begin(SD_CS, SPI_HALF_SPEED);
	#endif

	// Serial.begin(115200);
	Serial.begin(57600);

	// camera
	camera.setSerial(&Serial);

	camera.bufferFullFunctionPtr = &cameraBufferFull;
	camera.readImageStartFunctionPtr = &cameraReadImageStart;
	camera.readImageStopFunctionPtr = &cameraReadImageStop;

	camera.init();
	camera.brightness(2);
	camera_init_success = camera.init_success;

	#ifdef USE_TFT
	screen.text("init done", 0, 0);
	#endif

	// TODO: attach interrupt w/o arduino code
	attachInterrupt(VSYNC_INT, vsync_handler, FALLING);
	// attachInterrupt(HREF_INT, href_handler, RISING); // not really needed
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
		//Serial << _BYTE(temp);
		if (temp == '+' || bufCount > 6)
			break;
  	buf[bufCount] = temp;
  	bufCount++;
	}

	return atoi(buf);
}

void checkSerialInput() {
	int serial = 0;
	int v = 0;

	if( Serial.available() ) {
    serial = Serial.read();

    switch (serial) {
    case 'i':
    	camera.capture_image();
    	break;

    case '0':
    	camera.reset(MODE_RGB444);
    	Serial << F("mode: RGB444 - ") << camera.reset(MODE_RGB444) << endll;
    	break;

    case '1':
    	Serial << F("mode: RGB555 - ") << camera.reset(MODE_RGB555) << endll;
    	break;

    case '2':
    	Serial << F("mode: RGB565 - ") <<  camera.reset(MODE_RGB565) << endll;
    	break;

    case '3':
    	Serial << F("mode: YUV - ") << camera.reset(MODE_YUV) << endll;
    	break;

    case 'n':
    	if (readIntFromSerial() == 0) {
      	camera.nightMode(true);
      	Serial << F("night mode enabled") << endll;
    	} else {
      	camera.nightMode(false);
      	Serial << F("night mode disabled") << endll;
    	}
    	break;

    case 'c':
    	Serial << F("contrast: ");
    	v = readIntFromSerial();
    	camera.contrast(v);
    	Serial << v << endll;
    	break;

    case 'b':
    	Serial << F("brightness: ");
    	v = readIntFromSerial();
    	camera.brightness(v);
    	Serial << v << endll;
    	break;

    case 's':
    	Serial << F("effect: ");
    	v = readIntFromSerial();
    	camera.specialEffect(v);
    	Serial << v << endll;
    	break;

    }

	}
}
#endif

extern "C" void __cxa_pure_virtual() { while (1); }
