#include "bme280.h"
#include "i2c1.h"
#include "uart.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include <stdint.h>

/* ================= USER SETTINGS ================= */

#define RELAY_PIN        5        // PB5
#define TEMP_THRESHOLD  32.1f     // °C (fan ON temp)

/* ================= GLOBALS ================= */

static int32_t t_fine;
static BME280_CalibData cal;

/* ================= LOW LEVEL I2C ================= */

static void BME280_WriteReg(uint8_t reg, uint8_t val)
{
    I2CStart1();
    I2CSendSlaveAddress1(BME280_I2C_ADDR);
    I2CSendData1(reg);
    I2CSendData1(val);
    I2CStop1();
}

static uint8_t BME280_ReadReg(uint8_t reg)
{
    uint8_t val;

    I2CStart1();
    I2CSendSlaveAddress1(BME280_I2C_ADDR);
    I2CSendData1(reg);

    I2CRepeatStart1();
    I2CSendSlaveAddress1(BME280_I2C_ADDR | 1);
    val = I2CRecvDataNAck1();
    I2CStop1();

    return val;
}

static void BME280_ReadMulti(uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2CStart1();
    I2CSendSlaveAddress1(BME280_I2C_ADDR);
    I2CSendData1(reg);

    I2CRepeatStart1();
    I2CSendSlaveAddress1(BME280_I2C_ADDR | 1);

    for(uint8_t i = 0; i < len - 1; i++)
        buf[i] = I2CRecvDataAck1();

    buf[len - 1] = I2CRecvDataNAck1();
    I2CStop1();
}

/* ================= CALIBRATION ================= */

static void BME280_ReadCalibrationData(void)
{
    uint8_t buf[26], buf2[7];

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

/* ================= COMPENSATION ================= */

static int32_t BME280_Compensate_Temp(int32_t adc_T)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * cal.dig_T2) >> 11;
    int32_t var2 = (((((adc_T >> 4) - cal.dig_T1) *
                      ((adc_T >> 4) - cal.dig_T1)) >> 12) *
                      cal.dig_T3) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

static uint32_t BME280_Compensate_Humidity(int32_t adc_H)
{
    int32_t v = t_fine - 76800;

    v = (((((adc_H << 14) - (cal.dig_H4 << 20) -
             (cal.dig_H5 * v)) + 16384) >> 15) *
         (((((((v * cal.dig_H6) >> 10) *
              (((v * cal.dig_H3) >> 11) + 32768)) >> 10)
              + 2097152) * cal.dig_H2 + 8192) >> 14));

    if(v < 0) v = 0;
    if(v > 419430400) v = 419430400;

    return (uint32_t)(v >> 12);
}

/* ================= RELAY (ACTIVE-HIGH) ================= */

static void Relay_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER &= ~(3U << (RELAY_PIN * 2));
    GPIOB->MODER |=  (1U << (RELAY_PIN * 2));

    GPIOB->OTYPER &= ~(1U << RELAY_PIN);
    GPIOB->PUPDR  &= ~(3U << (RELAY_PIN * 2));
    GPIOB->OSPEEDR &= ~(3U << (RELAY_PIN * 2));

    GPIOB->ODR &= ~(1U << RELAY_PIN);   // Fan OFF (0)
}

static void Relay_On(void)
{
    GPIOB->ODR |= (1U << RELAY_PIN);    // Fan ON (1)
}

static void Relay_Off(void)
{
    GPIOB->ODR &= ~(1U << RELAY_PIN);   // Fan OFF (0)
}

/* ================= BME280 INIT ================= */

int BME280_Init(void)
{
    if(BME280_ReadReg(BME280_REG_ID) != 0x60)
        return -1;

    BME280_WriteReg(BME280_REG_RESET, BME280_RESET_CMD);
    while(BME280_ReadReg(BME280_REG_STATUS) & 0x01);

    BME280_ReadCalibrationData();

    BME280_WriteReg(BME280_REG_CTRL_HUM,  0x01);
    BME280_WriteReg(BME280_REG_CTRL_MEAS, 0x27);
    BME280_WriteReg(BME280_REG_CONFIG,    0x00);

    return 0;
}

/* ================= READ TEMP & HUM ================= */

void BME280_ReadTempHum(float *temp, float *hum)
{
    uint8_t data[6];

    BME280_ReadMulti(BME280_REG_TEMP_MSB, data, 6);

    int32_t adc_T = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t adc_H = (data[4] << 8)  | data[5];

    *temp = BME280_Compensate_Temp(adc_T) / 100.0f;
    *hum  = BME280_Compensate_Humidity(adc_H) / 1024.0f;
}

/* ================= MAIN TASK ================= */

uint8_t FanControl(float *temperature, float *humidity)
{
    static uint8_t init_done = 0;
    uint8_t fan_state;

    if (!init_done)
    {
        I2CInit1();
        Relay_Init();

        if (BME280_Init() != 0)
            return 0;   // sensor fail → fan OFF

        init_done = 1;
    }

    BME280_ReadTempHum(temperature, humidity);

    if (*temperature >= TEMP_THRESHOLD)
    {
        Relay_On();
        fan_state = 1;
    }
    else
    {
        Relay_Off();
        fan_state = 0;
    }

    return fan_state;
}

