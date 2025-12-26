#include "bme280.h"
#include "i2c.h"

#define I2C_DEVICE_ADDR     (0x76 << 1)     // BME280 default I2C address (0x76 or 0x77)

// BME280 Registers
#define BME280_REG_CHIPID       0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7

// Calibration registers
#define BME280_REG_CALIB_00     0x88
#define BME280_REG_CALIB_26     0xE1

// Reset command
#define RESET_CMD               0xB6

// Compensation variables
static int32_t t_fine;

// Calibration structure
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_Calib;

static BME280_Calib cal;


/* ------------------------------------------------------------------
   Low-level I2C Helpers
-------------------------------------------------------------------*/

static void BME280_WriteReg(uint8_t reg, uint8_t value)
{
    I2CStart();
    I2CSendSlaveAddress(I2C_DEVICE_ADDR);
    I2CSendData(reg);
    I2CSendData(value);
    I2CStop();
}

static uint8_t BME280_ReadReg(uint8_t reg)
{
    uint8_t val;

    I2CStart();
    I2CSendSlaveAddress(I2C_DEVICE_ADDR);
    I2CSendData(reg);

    I2CRepeatStart();
    I2CSendSlaveAddress(I2C_DEVICE_ADDR | 1);    // read

    val = I2CRecvDataNAck();
    I2CStop();

    return val;
}

static void BME280_ReadMulti(uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2CStart();
    I2CSendSlaveAddress(I2C_DEVICE_ADDR);
    I2CSendData(reg);

    I2CRepeatStart();
    I2CSendSlaveAddress(I2C_DEVICE_ADDR | 1);

    for (int i = 0; i < len - 1; i++)
        buf[i] = I2CRecvDataAck();

    buf[len - 1] = I2CRecvDataNAck();
    I2CStop();
}


/* ------------------------------------------------------------------
   Read Calibration Data
-------------------------------------------------------------------*/
void BME280_ReadCalibrationData()
{
    uint8_t buf[26];

    // Read first calibration block (0x88 - 0xA1)
    BME280_ReadMulti(BME280_REG_CALIB_00, buf, 24);

    cal.dig_T1 = (buf[1] << 8) | buf[0];
    cal.dig_T2 = (buf[3] << 8) | buf[2];
    cal.dig_T3 = (buf[5] << 8) | buf[4];

    cal.dig_P1 = (buf[7] << 8) | buf[6];
    cal.dig_P2 = (buf[9] << 8) | buf[8];
    cal.dig_P3 = (buf[11] << 8) | buf[10];
    cal.dig_P4 = (buf[13] << 8) | buf[12];
    cal.dig_P5 = (buf[15] << 8) | buf[14];
    cal.dig_P6 = (buf[17] << 8) | buf[16];
    cal.dig_P7 = (buf[19] << 8) | buf[18];
    cal.dig_P8 = (buf[21] << 8) | buf[20];
    cal.dig_P9 = (buf[23] << 8) | buf[22];

    cal.dig_H1 = buf[24];  // 0xA1

    // Read second humidity calibration block (0xE1 - 0xE7)
    uint8_t buf2[7];
    BME280_ReadMulti(BME280_REG_CALIB_26, buf2, 7);

    cal.dig_H2 = (buf2[1] << 8) | buf2[0];
    cal.dig_H3 = buf2[2];
    cal.dig_H4 = (buf2[3] << 4) | (buf2[4] & 0x0F);
    cal.dig_H5 = ((buf2[4] >> 4) & 0x0F) | (buf2[5] << 4);
    cal.dig_H6 = buf2[6];
}


/* ------------------------------------------------------------------
   Compensation Formulas (from datasheet)
-------------------------------------------------------------------*/

int32_t BME280_Compensate_Temp(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * cal.dig_T2) >> 11;
    var2 = (((((adc_T >> 4) - cal.dig_T1) * ((adc_T >> 4) - cal.dig_T1)) >> 12) *
             cal.dig_T3) >> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;
    return T; // temperature in °C x100
}

uint32_t BME280_Compensate_Pressure(int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * cal.dig_P6;
    var2 = var2 + ((var1 * cal.dig_P5) << 17);
    var2 = var2 + (((int64_t)cal.dig_P4) << 35);
    var1 = ((var1 * var1 * cal.dig_P3) >> 8) +
           ((var1 * cal.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) *
           (cal.dig_P1) >> 33;

    if (var1 == 0) return 0; // avoid division by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (cal.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = (cal.dig_P8 * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)cal.dig_P7) << 4);
    return (uint32_t)p;   // Pa x256
}

uint32_t BME280_Compensate_Humidity(int32_t adc_H)
{
    int32_t v_x1;

    v_x1 = (t_fine - 76800);
    v_x1 = (((((adc_H << 14) - (cal.dig_H4 << 20) -
                (cal.dig_H5 * v_x1)) + 16384) >> 15) *
             (((((((v_x1 * cal.dig_H6) >> 10) *
                   (((v_x1 * cal.dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
                cal.dig_H2 + 8192) >> 14));

    v_x1 = v_x1 -
           (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) *
              cal.dig_H1) >> 4);

    v_x1 = (v_x1 < 0 ? 0 : v_x1);
    v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);

    return (uint32_t)(v_x1 >> 12); // humidity x1024
}


/* ------------------------------------------------------------------
   Initialize BME280
-------------------------------------------------------------------*/

int BME280_Init()
{
    uint8_t id = BME280_ReadReg(BME280_REG_CHIPID);
    if (id != 0x60) return -1;   // Wrong chip

    // Reset
    BME280_WriteReg(BME280_REG_RESET, RESET_CMD);

    // Wait for reset completion
    while (BME280_ReadReg(BME280_REG_STATUS) & 0x01);

    // Read calibration
    BME280_ReadCalibrationData();

    // Configure:
    BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01); // Humidity oversampling ×1
    BME280_WriteReg(BME280_REG_CTRL_MEAS, 0x27); // Temp ×1, Press ×1, Normal mode
    BME280_WriteReg(BME280_REG_CONFIG, 0x00);   // No filter, standby 0.5ms

    return 0;
}


/* ------------------------------------------------------------------
   Read Data
-------------------------------------------------------------------*/

void BME280_ReadAll(float *temp, float *press, float *hum)
{
    uint8_t data[8];
    BME280_ReadMulti(BME280_REG_PRESS_MSB, data, 8);

    int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    int32_t adc_H = (data[6] << 8)  | data[7];

    int32_t t = BME280_Compensate_Temp(adc_T);
    uint32_t p = BME280_Compensate_Pressure(adc_P);
    uint32_t h = BME280_Compensate_Humidity(adc_H);

    *temp = t / 100.0f;
    *press = p / 256.0f;
    *hum  = h / 1024.0f;
}
