/*
 * ov7670_ports.h
 *
 * Defines ports and pins uses by OV7670 fifo camera module
 *
 *  Created on: Aug 18, 2013
 *      Author: arndtjenssen
 */

#ifndef OV7670_PORTS_H_
#define OV7670_PORTS_H_

#define DATA_DDR						DDRA
#define DATA_PORT						PORTA
#define DATA_PIN						PINA

#define AL422_DO0           _BV(PINA0)          // Data Output 0
#define AL422_DO1           _BV(PINA1)          // Data Output 1
#define AL422_DO2           _BV(PINA2)          // Data Output 2
#define AL422_DO3           _BV(PINA3)          // Data Output 3
#define AL422_DO4           _BV(PINA4)          // Data Output 4
#define AL422_DO5           _BV(PINA5)          // Data Output 5
#define AL422_DO6           _BV(PINA6)          // Data Output 6
#define AL422_DO7           _BV(PINA7)          // Data Output 7


#define CONTROL_DDR					DDRC
#define CONTROL_PORT				PORTC
#define CONTROL_PIN					PINC

#define AL422_WRST          _BV(PINC7)          // Write Reset (active low)
#define AL422_RCK           _BV(PINC6)          // Read clock
#define AL422_RRST          _BV(PINC5)          // Read Reset (active low)
#define HREF                _BV(PINC4)					// HREF
#define AL422_WEN           _BV(PINC3)          // Write Enable (active low) - labeled WR on camera pin
#define VSYNC               _BV(PINC2)					// VSYNC
// attach OE and PWDN to ground
// attach RST (camera reset) to FTDI/Serial reset line


void static inline setup_ports() {
	DATA_DDR = 0b00000000;
	DATA_PORT = 0b11111111;

	CONTROL_DDR = AL422_RCK | AL422_WEN | AL422_RRST | AL422_WRST;
	CONTROL_PORT = AL422_RRST | AL422_WRST;
}

#endif /* OV7670_PORTS_H_ */
