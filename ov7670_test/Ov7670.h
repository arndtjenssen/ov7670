/*
 * Ov7670.h
 *
 *  Created on: Aug 26, 2013
 *      Author: arndtjenssen
 */

#ifndef OV7670_H_
#define OV7670_H_

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include "fifo.h"
#include "camera_config.h"
#include "SimpleI2C.h"
#include <HardwareSerial.h>

#define OV_SERIAL_DEBUG

#define VSYNC_INT 2
#define HREF_INT 1

#define MODE_RGB444 0
#define MODE_RGB555 1
#define MODE_RGB565 2
#define MODE_YUV 		3

#define SIZEX (160)
#define SIZEY (120)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

class Ov7670 {
public:
	Ov7670();
	void init();
	uint8_t reset(uint8_t mode);
	void capture_image();
	void setSerial(HardwareSerial *s);
	void vsync_handler();
	void href_handler();
	void serialOutput(bool enabled);

	uint8_t init_success;
	void (*bufferFullFunctionPtr)(uint8_t *);
	static const uint16_t BUFFER_SIZE = SIZEY * 3;

private:
	volatile bool capture_request;
	volatile bool capture_done;
	volatile bool busy;
	volatile uint16_t line_counter;
	volatile uint16_t last_line_counter;
	volatile uint8_t camera_mode;
	volatile bool serialOutputEnabled;

	HardwareSerial *serial;
	uint8_t buffer[BUFFER_SIZE]; // rgb * sizey
	uint16_t bufferPos;

	void read_stop();
	void capture_next();
	uint8_t captured();
	uint8_t read_one_byte();
	uint8_t transfer_regvals(struct regval_list *list);
	uint8_t init_rgb444_qqvga();
	uint8_t init_rgb555_qqvga();
	uint8_t init_rgb565_qqvga();
	uint8_t init_yuv_qqvga();
	void init_negative_vsync();
	void init_camera_reset();
	uint8_t init_default_values();

};

#endif /* OV7670_H_ */
