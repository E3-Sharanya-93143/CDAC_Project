/*
 * bme280.h
 *
 *  Created on: Apr 2, 2024
 *      Author: Nilesh
 */

#ifndef BME280_H_
#define BME280_H_

#include "stm32f4xx.h"
#include <stdint.h>

/* --------------------------------------
 * I2C address
 * -------------------------------------- */
#define BME280_I2C_ADDR     (0x76 << 1)   // shifted for STM32

/* --------------------------------------
 * Register Map (only required registers)
 * -------------------------------------- */
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5

#define BME280_REG_TEMP_MSB     0xFA
#define BME280_REG_HUM_MSB      0xFD

/* Calibration registers */
#define BME280_REG_CALIB_00     0x88
#define BME280_REG_CALIB_26     0xE1

/* Reset value */
#define BME280_RESET_CMD        0xB6

/* --------------------------------------
 * Oversampling options
 * -------------------------------------- */
#define BME280_OSRS_SKIP   0x00
#define BME280_OSRS_1X     0x01
#define BME280_OSRS_2X     0x02
#define BME280_OSRS_4X     0x03
#define BME280_OSRS_8X     0x04
#define BME280_OSRS_16X    0x05

/* --------------------------------------
 * Power modes
 * -------------------------------------- */
#define BME280_MODE_SLEEP   0x00
#define BME280_MODE_FORCED  0x01
#define BME280_MODE_NORMAL  0x03

/* --------------------------------------
 * Calibration data structure
 * (Temperature + Humidity only)
 * -------------------------------------- */
typedef struct
{
    /* Temperature calibration */
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    /* Humidity calibration */
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

} BME280_CalibData;

/* --------------------------------------
 * Public APIs
 * -------------------------------------- */

// Initialize BME280 (reset + calibration + config)
int BME280_Init(void);

// Read compensated temperature (°C) and humidity (%RH)
void BME280_ReadTempHum(float *temp, float *hum);

// Optional: raw data read (temperature & humidity only)
void BME280_ReadRaw(uint32_t *rawT, uint16_t *rawH);
void delay(volatile uint32_t count);
void BME280_Task(float *t, float *h);

#endif /* BME280_H_ */
