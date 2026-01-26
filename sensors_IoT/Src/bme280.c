#include "bme280.h"
#include "i2c1.h"
#include "uart.h"
#include<stdio.h>

#define I2C_DEVICE_ADDR     (0x76 << 1)   // BME280 I2C address

/* Registers */
#define BME280_REG_CHIPID       0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_TEMP_MSB     0xFA
#define BME280_REG_HUM_MSB      0xFD

/* Calibration registers */
#define BME280_REG_CALIB_00     0x88
#define BME280_REG_CALIB_26     0xE1

#define RESET_CMD               0xB6

/* Compensation variable */
static int32_t t_fine;

/* Calibration data (Temp + Hum only) */
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
} BME280_Calib;

static BME280_Calib cal;

/* ----------------------------------------------------
   Low-level I2C helpers
---------------------------------------------------- */
static void BME280_WriteReg(uint8_t reg, uint8_t val)
{
    I2CStart1();
    I2CSendSlaveAddress1(I2C_DEVICE_ADDR);
    I2CSendData1(reg);
    I2CSendData1(val);
    I2CStop1();
}

static uint8_t BME280_ReadReg(uint8_t reg)
{
    uint8_t val;

    I2CStart1();
    I2CSendSlaveAddress1(I2C_DEVICE_ADDR);
    I2CSendData1(reg);

    I2CRepeatStart1();
    I2CSendSlaveAddress1(I2C_DEVICE_ADDR | 1);
    val = I2CRecvDataNAck1();
    I2CStop1();

    return val;
}

static void BME280_ReadMulti(uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2CStart1();
    I2CSendSlaveAddress1(I2C_DEVICE_ADDR);
    I2CSendData1(reg);

    I2CRepeatStart1();
    I2CSendSlaveAddress1(I2C_DEVICE_ADDR | 1);

    for (uint8_t i = 0; i < len - 1; i++)
        buf[i] = I2CRecvDataAck1();

    buf[len - 1] = I2CRecvDataNAck1();
    I2CStop1();
}

/* ----------------------------------------------------
   Read Calibration Data
---------------------------------------------------- */
static void BME280_ReadCalibrationData(void)
{
    uint8_t buf[26];
    uint8_t buf2[7];

    BME280_ReadMulti(BME280_REG_CALIB_00, buf, 26);

    cal.dig_T1 = (buf[1] << 8) | buf[0];
    cal.dig_T2 = (buf[3] << 8) | buf[2];
    cal.dig_T3 = (buf[5] << 8) | buf[4];

    cal.dig_H1 = buf[25];

    BME280_ReadMulti(BME280_REG_CALIB_26, buf2, 7);

    cal.dig_H2 = (buf2[1] << 8) | buf2[0];
    cal.dig_H3 = buf2[2];
    cal.dig_H4 = (buf2[3] << 4) | (buf2[4] & 0x0F);
    cal.dig_H5 = ((buf2[4] >> 4) & 0x0F) | (buf2[5] << 4);
    cal.dig_H6 = buf2[6];
}

/* ----------------------------------------------------
   Compensation formulas
---------------------------------------------------- */
static int32_t BME280_Compensate_Temp(int32_t adc_T)
{
    int32_t var1, var2;

    var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * cal.dig_T2) >> 11;
    var2 = (((((adc_T >> 4) - cal.dig_T1) *
              ((adc_T >> 4) - cal.dig_T1)) >> 12) *
              cal.dig_T3) >> 14;

    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;   // °C ×100
}

static uint32_t BME280_Compensate_Humidity(int32_t adc_H)
{
    int32_t v_x1;

    v_x1 = t_fine - 76800;
    v_x1 = (((((adc_H << 14) - (cal.dig_H4 << 20) -
              (cal.dig_H5 * v_x1)) + 16384) >> 15) *
            (((((((v_x1 * cal.dig_H6) >> 10) *
               (((v_x1 * cal.dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
               cal.dig_H2 + 8192) >> 14));

    v_x1 -= (((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * cal.dig_H1 >> 4;

    if (v_x1 < 0) v_x1 = 0;
    if (v_x1 > 419430400) v_x1 = 419430400;

    return (uint32_t)(v_x1 >> 12);   // %RH ×1024
}

/* ----------------------------------------------------
   Initialize BME280
---------------------------------------------------- */
int BME280_Init(void)
{
    if (BME280_ReadReg(BME280_REG_CHIPID) != 0x60)
        return -1;

    BME280_WriteReg(BME280_REG_RESET, RESET_CMD);
    while (BME280_ReadReg(BME280_REG_STATUS) & 0x01);

    BME280_ReadCalibrationData();

    BME280_WriteReg(BME280_REG_CTRL_HUM, 0x01);   // Humidity oversampling x1
    BME280_WriteReg(BME280_REG_CTRL_MEAS, 0x21);  // Temp x1, Forced mode
    BME280_WriteReg(BME280_REG_CONFIG, 0x00);

    return 0;
}

/* ----------------------------------------------------
   Read Temperature & Humidity
---------------------------------------------------- */
void BME280_ReadTempHum(float *temp, float *hum)
{
    uint8_t data[6];

    BME280_ReadMulti(BME280_REG_TEMP_MSB, data, 6);

    int32_t adc_T = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t adc_H = (data[4] << 8)  | data[5];

    int32_t t = BME280_Compensate_Temp(adc_T);
    uint32_t h = BME280_Compensate_Humidity(adc_H);

    *temp = t / 100.0f;
    *hum  = h / 1024.0f;
}

void delay(volatile uint32_t count)
{
    while (count--);
}

//void BME280_Task(void)
//{
//    float temperature;
//    float humidity;
//    char buf[64];
//
//    /* Initialize I2C for BME280 */
//    I2CInit1();
//
//    /* Initialize BME280 */
//    if (BME280_Init() != 0)
//    {
//        UartPuts("BME280 Init Failed!\r\n");
//        while (1);   // Stop here if sensor init fails
//    }
//    UartPuts("BME280 Init Successful\r\n");
//
//    /* Read temperature and humidity */
//    BME280_ReadTempHum(&temperature, &humidity);
//
//    /* Print values via UART */
//    sprintf(buf,
//                "Temperature: %.2f C, Humidity: %.2f %%\r\n",
//                temperature, humidity);
//        //UartPuts(buf);
//
//    delay(500000);
//}

void BME280_Task(float *t, float *h)
{
    I2CInit1();

    if (BME280_Init() != 0)
        while (1);

    BME280_ReadTempHum(t, h);
}
