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

//---< 共通の状態符号 >---------------------------------------------------------------------------
#define TWSR_START                  ( 0x08 )    // 開始条件送信
#define TWSR_RESTART                ( 0x10 )    // 再送開始条件送信
#define TWSR_CONFLICT               ( 0x38 )    // バス競合
//---< 送信主装置動作の状態符号 >------------------------------------------------------------------
#define TWSR_SEND_SLA_W_ACK         ( 0x18 )    // SLA+W送信ACK受信
#define TWSR_SEND_SLA_W_NACK        ( 0x20 )    // SLA+W送信NACK受信
#define TWSR_SEND_DATA_ACK          ( 0x28 )    // データバイト送信ACK受信
#define TWSR_SEND_DATA_NACK         ( 0x30 )    // データバイト送信NACK受信
//---< 受信主装置動作の状態符号 >------------------------------------------------------------------
#define TWSR_SEND_SLA_R_ACK         ( 0x40 )    // SLA+R送信ACK受信
#define TWSR_SEND_SLA_R_NACK        ( 0x48 )    // SLA+R送信NACK受信
#define TWSR_RECV_DATA_ACK          ( 0x50 )    // データバイト受信ACK応答
#define TWSR_RECV_DATA_NACK         ( 0x58 )    // データバイト受信NACK応答

class SimpleI2CClass {
public:
	SimpleI2CClass();
	void init();
	uint8_t read_byte(uint8_t slave_address, uint8_t *data);
	uint8_t write_byte(uint8_t slave_address, uint8_t data);
	uint8_t sccb_read(uint8_t slave_address, uint8_t address, uint8_t *data);
	uint8_t sccb_write(uint8_t slave_address, uint8_t address, uint8_t data);
};

extern SimpleI2CClass SimpleI2C;
#endif /* SIMPLEI2C_H_ */
