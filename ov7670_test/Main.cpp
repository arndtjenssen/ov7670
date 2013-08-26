/*
 * main.c
 *
 * Code taken from a couple of sources, but esp from: http://mbed.org/users/diasea/notebook/ov7670_FIFO_AL422B/
 *
 *  Created on: Jul 21, 2013
 *      Author: arndtjenssen
 */

#include "Main.h"

#define SIZEX (160)
#define SIZEY (120)

uint8_t camera_init_success = false;
uint8_t camera_mode = 0;

volatile uint8_t capture_request = false;
volatile uint8_t capture_done = true;
volatile uint8_t busy = false;
volatile uint16_t line_counter = 0;
volatile uint16_t last_line_counter = 0;

void vsync_handler() {
	if (capture_request) {
		WRITE_RESET;
		WRITE_ENABLE;
		capture_request = false;
		capture_done = false;
	} else {
		WRITE_DISABLE;
		WRITE_RESET;
		if (busy) {
			busy = false;
			capture_done = true;
		}
	}
	last_line_counter = line_counter;
	line_counter = 0;
}

void href_handler() {
	line_counter++;
}

void capture_next() {
	capture_request = true;
	busy = true;
}

uint8_t captured() {
	uint8_t result;
   if (busy) {
       result = false ;
   } else {
       result = capture_done;
       capture_done = false;
   }
   return result ;
}

uint8_t read_one_byte() {
	uint8_t b;

	READ_CLOCK_HIGH;
	b = DATA_PIN;
	READ_CLOCK_LOW;

	return b;
}

void read_stop() {
	read_one_byte();
	READ_CLOCK_HIGH;
}

void capture_image() {
  capture_next();
  while(captured() == false);
  _delay_us(10);

	#ifdef SERIAL_DEBUG
  Serial << F("image captured: ") << last_line_counter << endl;
	#endif
  READ_RESET;

  int r=0, g=0, b=0, d1 = 0, d2 = 0, d3 = 0, d4 = 0;
  uint8_t index = 0;

  // read image
  for (int y = 0; y < SIZEY; y++) {
  	for (int x = 0; x < SIZEX; x++) {

  		if (camera_mode != MODE_YUV) {
    		d1 = read_one_byte();
    		d2 = read_one_byte();
  		}

  		switch (camera_mode) {
  			case MODE_RGB444:
  	  		b = (d1 & 0x0F) << 4;
  				g = (d2 & 0xF0);
  				r = (d2 & 0x0F) << 4;
  				break;
  			case MODE_RGB555:
  	      b = (d1 & 0x1F) << 3;
  	      g = (((d1 & 0xE0) >> 2) | ((d2 & 0x03) << 6));
  	      r = (d2 & 0x7c) << 1;
  				break;
  			case MODE_RGB565:
  	      b = (d1 & 0x1F) << 3;
  	      g = (((d1 & 0xE0) >> 3) | ((d2 & 0x07) << 5));
  	      r = (d2 & 0xF8);
  				break;
  			case MODE_YUV:
  				if (index % 2 == 0) {
  	    		d1 = read_one_byte(); // U0
  	    		d2 = read_one_byte(); // Y0
    	  		d3 = read_one_byte(); // V0
    	  		d4 = read_one_byte(); // Y1

            b = d2 + 1.77200 * (d1 - 128);
            g = d2 - 0.34414 * (d1 - 128) - 0.71414 * (d3 - 128);
            r = d2 + 1.40200 * (d3 - 128);
  				} else {
            b = d4 + 1.77200 * (d1 - 128);
            g = d4 - 0.34414 * (d1 - 128) - 0.71414 * (d3 - 128);
            r = d4 + 1.40200 * (d3 - 128);
  				}

          b = min(max(b, 0), 255);
          g = min(max(g, 0), 255);
          r = min(max(r, 0), 255);

          index++;
  				break;

  		}

			#ifdef RGB888_SERIAL_OUT
  		Serial << _BYTE(r) << _BYTE(g) << _BYTE(b);
			#endif
  	}
  }

  read_stop();
}


/**
 * transfers a regval list via SCCB to camera
 *
 * 1: success
 * other values: failure
 */
uint8_t transfer_regvals(struct regval_list *list) {
	uint8_t ret = 0;
	uint8_t i = 0;

	for(;;) {
		// end marker check
		if ((list[i].reg_num == END_MARKER) && (list[i].value == END_MARKER)) {
			return 1;
		}

		ret = SimpleI2C.sccb_write(OV7670_I2C_ADDR, list[i].reg_num, list[i].value);
		if (!ret) {
			return i;
		}

		// delay for reset command
		if ((list[i].reg_num == REG_COM7) && (list[i].value == COM7_RESET)) {
			delay(200);
		} else {
			delay(10);
		}

		i++;
	}

	return 0;
}


uint8_t init_rgb444_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	delay(10);
	ret = transfer_regvals(ov7670_fmt_rgb444);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t init_rgb555_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	delay(10);
	ret = transfer_regvals(ov7670_fmt_rgb555);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t init_rgb565_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	delay(10);
	ret = transfer_regvals(ov7670_fmt_rgb565);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t init_yuv_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_YUV);
	delay(10);
	ret = transfer_regvals(ov7670_fmt_yuv422);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}


void init_negative_vsync() {
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM10, COM10_VS_NEG);
	delay(10);
}

void init_camera_reset() {
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RESET);
	delay(200);
}

uint8_t init_default_values() {
	return transfer_regvals(ov7670_default);
}


/**
 * returns 1 if camera was initialized succesful
 * mode: MODE_RGB444, MODE_RGB555, MODE_RGB565, MODE_YUV
 */
uint8_t init_camera(uint8_t mode) {
	uint8_t ret = 0;
	camera_mode = mode;

	init_camera_reset();

	switch (camera_mode) {
	case MODE_RGB444:
		ret = init_rgb444_qqvga();
		if (ret != 1) return ret;
		break;
	case MODE_RGB555:
		ret = init_rgb555_qqvga();
		if (ret != 1) return ret;
		break;
	case MODE_RGB565:
		ret = init_rgb565_qqvga();
		if (ret != 1) return ret;
		break;
	case MODE_YUV:
		ret = init_yuv_qqvga();
		if (ret != 1) return ret;
		break;

	}

	init_negative_vsync();
	ret = init_default_values();

	return ret;
}


void setup() {
	Serial.begin(115200);

	#ifdef SERIAL_DEBUG
	Serial << F("setup ports") << endl;
	#endif
	setup_ports();

	#ifdef SERIAL_DEBUG
	Serial << F("i2c init") << endl;
	#endif
	SimpleI2C.init();

	// init camera via i2c and remember success
	camera_init_success = init_camera(MODE_YUV);
	#ifdef SERIAL_DEBUG
	Serial << F("camera init: ") << camera_init_success << endl;
	#endif

	attachInterrupt(VSYNC_INT, vsync_handler, FALLING);
	attachInterrupt(HREF_INT, href_handler, RISING);
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
    	capture_image();
    	break;

    case '0':
    	init_camera(MODE_RGB444);
    	break;

    case '1':
    	init_camera(MODE_RGB555);
    	break;

    case '2':
    	init_camera(MODE_RGB565);
    	break;

    case '3':
    	init_camera(MODE_YUV);
    	break;

    }

	}
}
#endif
