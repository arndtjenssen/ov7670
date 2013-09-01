/*
 * Ov7670.cpp
 *
 *  Created on: Aug 26, 2013
 *      Author: arndtjenssen
 */

#include "Ov7670.h"


Ov7670::Ov7670() {
	capture_request = false;
	capture_done = true;
	busy = false;
	init_success = false;
	line_counter = 0;
	last_line_counter = 0;
	camera_mode = MODE_YUV;
	serial = 0;
	bufferPos = 0;
	bufferFullFunctionPtr = 0;
	readImageStartFunctionPtr = 0;
	readImageStopFunctionPtr = 0;
	edgeEnhacementEnabled = false;
	denoiseEnabled = false;
}

void Ov7670::setSerial(HardwareSerial *s) {
	serial = s;
}

void Ov7670::nightMode(bool enable) {
	if (enable) {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM11, COM11_EXP|COM11_HZAUTO);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM11, COM11_EXP|COM11_HZAUTO|COM11_NIGHT|COM11_NIGHT_FR8);
	} else {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM11, COM11_EXP|COM11_HZAUTO|COM11_NIGHT|COM11_NIGHT_FR8);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM11, COM11_EXP|COM11_HZAUTO);
	}
}

// -2 (low contrast ) to +2 (high contrast)
void Ov7670::contrast(int8_t value) {
	static const uint8_t values[] = {0x60, 0x50, 0x40, 0x38, 0x30};

	value = min(max((value + 2), 0), 4);
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_CONTRAST, values[value]);
}

// -2 (dark) to +2 (bright)
void Ov7670::brightness(int8_t value) {
	static const uint8_t values[] = {0xb0, 0x98, 0x00, 0x18, 0x30};

	value = min(max((value + 2), 0), 4);
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_BRIGHT, values[value]);
}

// 0 - Normal, 1 - Antique, 2 - Bluish, 3 - Greenish
// 4 - Redish, 5 - B&W, 6 - Negative, 7 - B&W negative
void Ov7670::specialEffect(uint8_t value) {
	value = min(max((value + 2), 0), 4);
	transfer_regvals(ov7670_effects[value]);
}

// 0 to disable, > 0 enable and set edge enhancement factor
void Ov7670::edgeEnhancement(uint8_t value) {
	uint8_t v = COM16_AWBGAIN | (denoiseEnabled ? COM16_DENOISE : 0);

	if (value == 0) {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM16, v);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_EDGE, 0);
	} else {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM16, v | COM16_EDGE);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_EDGE, value);
	}
	edgeEnhacementEnabled = (value > 0);
}

// 0 to disable, > 0 enable and set denoise factor
void Ov7670::denoise(uint8_t value) {
	uint8_t v = COM16_AWBGAIN | (edgeEnhacementEnabled ? COM16_EDGE : 0);

	if (value == 0) {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM16, v);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_EDGE, 0);
	} else {
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM16, v | COM16_DENOISE);
		SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_DENOISE_STRENGTH, value);
	}
	denoiseEnabled = (value > 0);
}


void Ov7670::init() {
	if (serial)
		serial->println(F("setup ports"));
	setup_ports();

	if (serial)
		serial->println(F("i2c init"));
	SimpleI2C.init();

	init_success = reset(MODE_YUV);
	if (serial) {
		serial->print(F("init: "));
		serial->println(init_success);
	}
}

/**
 * returns 1 if camera was initialized succesful
 * mode: MODE_RGB444, MODE_RGB555, MODE_RGB565, MODE_YUV
 */
uint8_t Ov7670::reset(uint8_t mode) {
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

void Ov7670::vsync_handler() {
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
	//last_line_counter = line_counter;
	//line_counter = 0;
}

void Ov7670::href_handler() {
	line_counter++;
}

void Ov7670::capture_next() {
	capture_request = true;
	busy = true;
}

uint8_t Ov7670::captured() {
	uint8_t result;
   if (busy) {
       result = false ;
   } else {
       result = capture_done;
       capture_done = false;
   }
   return result ;
}

uint8_t Ov7670::read_one_byte() {
	uint8_t b;

	READ_CLOCK_HIGH;
	b = DATA_PIN;
	READ_CLOCK_LOW;

	return b;
}

void Ov7670::read_stop() {
	read_one_byte();
	READ_CLOCK_HIGH;
	READ_CLOCK_LOW;
}

void Ov7670::capture_image() {
  capture_next();
  while(captured() == false);
  _delay_us(10);

  bufferPos = 0;
  if (readImageStartFunctionPtr) {
  	(*readImageStartFunctionPtr)();
  }

  READ_RESET;

  int r=0, g=0, b=0, d1 = 0, d2 = 0, d3 = 0, d4 = 0;
  uint16_t index = 0;

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

            // b = d2 + 1.77200 * (d1 - 128);
            // g = d2 - 0.34414 * (d1 - 128) - 0.71414 * (d3 - 128);
            // r = d2 + 1.40200 * (d3 - 128);

            b = d2 + 1.4075 * (d1 - 128);
            g = d2 - 0.3455 * (d1 - 128) - 0.7169 * (d3 - 128);
            r = d2 + 1.7790 * (d3 - 128);
  				} else {
            // b = d4 + 1.77200 * (d1 - 128);
            // g = d4 - 0.34414 * (d1 - 128) - 0.71414 * (d3 - 128);
            // r = d4 + 1.40200 * (d3 - 128);

            b = d4 + 1.4075 * (d1 - 128);
            g = d4 - 0.3455 * (d1 - 128) - 0.7169 * (d3 - 128);
            r = d4 + 1.7790 * (d3 - 128);
  				}

          b = min(max(b, 0), 255);
          g = min(max(g, 0), 255);
          r = min(max(r, 0), 255);

          index++;
  				break;
  		}

  		buffer[bufferPos] = r;
  		buffer[bufferPos + 1] = g;
  		buffer[bufferPos + 2] = b;
  		bufferPos += 3;
  		if (bufferPos >= BUFFER_SIZE) {
  			if (bufferFullFunctionPtr) {
  				(*bufferFullFunctionPtr)(buffer);
  			}

  			bufferPos = 0;
  		}
  	}
  }

  read_stop();

  if (readImageStopFunctionPtr) {
  	(*readImageStopFunctionPtr)();
  }

}


/**
 * transfers a regval list via SCCB to camera
 *
 * 1: success
 * other values: failure
 */
uint8_t Ov7670::transfer_regvals(struct regval_list *list) {
	uint8_t ret = 0;
	uint8_t i = 0;

	for(;;) {
		// end marker check
		if ((list[i].reg_num == EM) && (list[i].value == EM)) {
			return 1;
		}

		ret = SimpleI2C.sccb_write(OV7670_I2C_ADDR, list[i].reg_num, list[i].value);
		if (!ret) {
			return i;
		}

		// delay for reset command
		if ((list[i].reg_num == REG_COM7) && (list[i].value == COM7_RESET)) {
			_delay_ms(200);
		}

		i++;
	}

	return 0;
}


uint8_t Ov7670::init_rgb444_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	ret = transfer_regvals(ov7670_fmt_rgb444);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t Ov7670::init_rgb555_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	ret = transfer_regvals(ov7670_fmt_rgb555);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t Ov7670::init_rgb565_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RGB | COM7_QQVGA);
	ret = transfer_regvals(ov7670_fmt_rgb565);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}

uint8_t Ov7670::init_yuv_qqvga() {
	uint8_t ret = 0;

	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_YUV);
	ret = transfer_regvals(ov7670_fmt_yuv422);
	if (ret != 1) return ret;

	return transfer_regvals(ov7670_qqvga);
}


void Ov7670::init_negative_vsync() {
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM10, COM10_VS_NEG);
}

void Ov7670::init_camera_reset() {
	SimpleI2C.sccb_write(OV7670_I2C_ADDR, REG_COM7, COM7_RESET);
	_delay_ms(200);
}

uint8_t Ov7670::init_default_values() {
	return transfer_regvals(ov7670_default);
}
