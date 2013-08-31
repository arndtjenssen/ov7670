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
#include "OV7670.h"

#define SERIAL_INFO
#define SERIAL_DEBUG
#define USE_TFT
#define USE_SD

//
// time definitions against ms
//
const long DAY = 86400000; // 86400000 milliseconds in a day
const long HOUR = 3600000; // 3600000 milliseconds in an hour
const long MINUTE = 60000; // 60000 milliseconds in a minute
const long SECOND =  1000; // 1000 milliseconds in a second


void serialTimeStamp();
void checkSerialInput();
void write_reset();

#endif /* MAIN_H_ */
