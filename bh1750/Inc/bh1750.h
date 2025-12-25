/*
 * bh1750.h
 *
 *  Created on: Nov 27, 2025
 *      Author: Sharanya Gajjela
 */

#ifndef BH1750_H_
#define BH1750_H_

#include "i2c.h"

// BH1750 Slave Address (ADDR pin connected to GND or floating)
#define BH1750_ADDR_WRITE   0x46    // (0x23 << 1) | 0 (Write bit)
#define BH1750_ADDR_READ    0x47    // (0x23 << 1) | 1 (Read bit)

// BH1750 Instruction Commands
#define BH1750_POWER_DOWN           0x00 //Sleep mode
#define BH1750_POWER_ON             0x01 //Wakes sensor from power down
#define BH1750_RESET                0x07 //clear registers

// Measurement Modes (High Resolution Mode is most common)
#define BH1750_CONT_HR_MODE         0x10    // Continuous High Resolution Mode(recommend)
#define BH1750_CONT_HR_MODE2        0x11    // Continuous High Resolution Mode 2 (0.5 lux)
#define BH1750_CONT_LR_MODE         0x13    // Continuous Low Resolution Mode (4 lux)
#define BH1750_ONE_TIME_HR_MODE     0x20    // One-Time High Resolution Mode
#define BH1750_ONE_TIME_HR_MODE2    0x21    // One-Time High Resolution Mode 2
#define BH1750_ONE_TIME_LR_MODE     0x23    // One-Time Low Resolution Mode

// Function Prototypes
void BH1750_WriteCommand(uint8_t command);
void BH1750_Init(void);
uint16_t BH1750_ReadLux(void);

#endif /* BH1750_H_ */

