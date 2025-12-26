/*
 * i2c.h
 *
 *  Created on: Apr 2, 2024
 *      Author: Nilesh
 */

#ifndef I2C_H_
#define I2C_H_

#include "stm32f4xx.h"
#include "stm32f407xx.h"

// I2C2 pins on PORTB
#define I2C2_SCL   10   // PB10 = SCL
#define I2C2_SDA   11   // PB11 = SDA

void I2CStart();
void I2CRepeatStart();
void I2CStop();
void I2CSendSlaveAddress(uint8_t slaveaddr);
void I2CSendData(uint8_t val);
uint8_t I2CRecvDataAck(void);
uint8_t I2CRecvDataNAck(void);
int I2CIsDeviceReady(uint8_t slaveaddr);
void I2CInit();

#endif /* I2C_H_ */
