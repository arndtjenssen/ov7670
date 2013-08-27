/*
 * SimpleI2c.h
 *
 *  Created on: Jul 23, 2013
 *      Author: arndtjenssen
 */

#ifndef SIMPLEI2C_H_
#define SIMPLEI2C_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


#define TWSR_START                  ( 0x08 )
#define TWSR_RESTART                ( 0x10 )
#define TWSR_CONFLICT               ( 0x38 )

#define TWSR_SEND_SLA_W_ACK         ( 0x18 )
#define TWSR_SEND_SLA_W_NACK        ( 0x20 )
#define TWSR_SEND_DATA_ACK          ( 0x28 )
#define TWSR_SEND_DATA_NACK         ( 0x30 )

#define TWSR_SEND_SLA_R_ACK         ( 0x40 )
#define TWSR_SEND_SLA_R_NACK        ( 0x48 )
#define TWSR_RECV_DATA_ACK          ( 0x50 )
#define TWSR_RECV_DATA_NACK         ( 0x58 )

class SimpleI2CClass {
public:
	SimpleI2CClass();
	void init();
	uint8_t read_byte(uint8_t slave_address, uint8_t *data);
	uint8_t write_byte(uint8_t slave_address, uint8_t data);
	uint8_t sccb_read(uint8_t slave_address, uint8_t address, uint8_t *data);
	uint8_t sccb_write(uint8_t slave_address, uint8_t address, uint8_t data);
private:
	uint8_t initialized;
};

extern SimpleI2CClass SimpleI2C;
#endif /* SIMPLEI2C_H_ */
