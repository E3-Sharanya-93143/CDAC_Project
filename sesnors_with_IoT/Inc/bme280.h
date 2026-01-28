#ifndef BME280_H_
#define BME280_H_

#include "stm32f4xx.h"
#include <stdint.h>

/* I2C address */
#define BME280_I2C_ADDR     (0x76 << 1)

/* Registers */
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

#define BME280_RESET_CMD        0xB6

/* Calibration data structure (Temp + Hum only) */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_CalibData;

/* Public APIs */
int BME280_Init(void);
void BME280_ReadTempHum(float *temp, float *hum);
uint8_t FanControl(float *temperature, float *humidity);
#endif /* BME280_H_ */
