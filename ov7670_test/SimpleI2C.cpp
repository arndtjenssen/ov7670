/*
 * SimpleI2c.cpp
 *
 * Encapsulates I2C protocol
 *
 *  Created on: Jul 23, 2013
 *      Author: arndtjenssen
 */

#include "SimpleI2C.h"


SimpleI2CClass::SimpleI2CClass() {
	initialized = false;
}

void SimpleI2CClass::init() {
	if (!initialized) {
		TWSR &=~3; //disable prescaler for TWI
		TWBR = 72; //set to 100khz
		initialized = true;
	}
}

uint8_t SimpleI2CClass::read_byte(uint8_t slave_address, uint8_t *data) {
  bool bRet = false;

  // start sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_START )
  {
      goto I2C_MASTER_READ_END_PROC;
  }

  // sla transmission
  TWDR = slave_address | 0x01;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_SLA_R_ACK )
  {
      goto I2C_MASTER_READ_END_PROC;
  }

  // receive data
  //  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_RECV_DATA_NACK )
  {
      goto I2C_MASTER_READ_END_PROC;
  }
  *data = TWDR;
  bRet = true;

I2C_MASTER_READ_END_PROC:
  // stop sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

  return bRet;
}

uint8_t SimpleI2CClass::write_byte(uint8_t slave_address, uint8_t data) {
  bool bRet = false;
  //unsigned char i = 0;

  // start sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_START )
  {
      goto I2C_MASTER_WRITE_END_PROC;
  }

  // send sla + w
  TWDR = slave_address | 0x00;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_SLA_W_ACK )
  {
      goto I2C_MASTER_WRITE_END_PROC;
  }

  // data transmission
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_DATA_ACK )
  {
      goto I2C_MASTER_WRITE_END_PROC;
  }
  bRet = true;

I2C_MASTER_WRITE_END_PROC:
  // stop sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

  return bRet;
}

uint8_t SimpleI2CClass::sccb_read(uint8_t slave_address, uint8_t address, uint8_t *data) {
  bool bRet = false;

  // set address
  bRet = write_byte(slave_address, address);
  if ( bRet == true )
  {
      bRet = read_byte(slave_address, data);
  }

  return bRet;
}

uint8_t SimpleI2CClass::sccb_write(uint8_t slave_address, uint8_t address, uint8_t data) {
  bool bRet = false;

  // start sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_START )
  {
      goto SCCB_WRITE_END_PROC;
  }

  // send sla + w
  TWDR = slave_address | 0x00;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_SLA_W_ACK )
  {
      goto SCCB_WRITE_END_PROC;
  }

  // data transmission
  TWDR = address;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_DATA_ACK )
  {
      goto SCCB_WRITE_END_PROC;
  }

  // data transmission
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)) );
  if ( (TWSR & 0xF8) != TWSR_SEND_DATA_ACK )
  {
      goto SCCB_WRITE_END_PROC;
  }

  bRet = true;

SCCB_WRITE_END_PROC:
  // stop sending conditions
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

  _delay_ms(10);

  return bRet;
}

SimpleI2CClass SimpleI2C;
