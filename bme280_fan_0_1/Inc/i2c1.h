#ifndef I2C1_H_
#define I2C1_H_

#include "stm32f4xx.h"
#include "stm32f407xx.h"

// I2C2 pins on PORTB
#define I2C2_SCL   10   // PB10 = SCL
#define I2C2_SDA   11   // PB11 = SDA

void I2CStart1();
void I2CRepeatStart1();
void I2CStop1();
void I2CSendSlaveAddress1(uint8_t slaveaddr);
void I2CSendData1(uint8_t val);
uint8_t I2CRecvDataAck1(void);
uint8_t I2CRecvDataNAck1(void);
int I2CIsDeviceReady1(uint8_t slaveaddr);
void I2CInit1();

#endif /* I2C_H_ */
