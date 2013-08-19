/*
 * Main.h
 *
 *  Created on: Jul 23, 2013
 *      Author: arndtjenssen
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <util/delay.h>
#include "Streaming.h"
#include "camera_config.h"
#include "fifo.h"
#include "SimpleI2C.h"

#define VSYNC_INT 2
#define HREF_INT 1


#define SERIAL_INFO
//#define SERIAL_DEBUG
#define RGB888_SERIAL_OUT


#define MODE_RGB444 0
#define MODE_RGB555 1
#define MODE_RGB565 2
#define MODE_YUV 		3


//
// time definitions against ms
//
const long DAY = 86400000; // 86400000 milliseconds in a day
const long HOUR = 3600000; // 3600000 milliseconds in an hour
const long MINUTE = 60000; // 60000 milliseconds in a minute
const long SECOND =  1000; // 1000 milliseconds in a second



void checkSerialInput();
void write_reset();

#endif /* MAIN_H_ */
