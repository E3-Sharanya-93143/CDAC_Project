
#include "adc.h"
#include "uart.h"
#include "stm32f4xx.h"
#include <stdio.h>

/* =========================================================
   USER CALIBRATION VALUES
   ========================================================= */

/* ---- Soil Sensor Calibration ---- */
#define ADC_SOIL_DRY   3100   // Measure in dry soil
#define ADC_SOIL_WET   1200   // Measure in water

#define MOISTURE_LOW_THRESHOLD   40   // Pump ON
#define MOISTURE_HIGH_THRESHOLD  50   // Pump OFF

/* ---- MQ135 Calibration ---- */
#define ADC_MAX        4095.0f
#define RL_VALUE       10000.0f   // 10k load resistor
#define CLEAN_AIR_RATIO 3.6f      // Rs/R0 in clean air

static float MQ135_R0 = 0.0f;    // Stored after calibration

/* =========================================================
   ADC INITIALIZATION
   ========================================================= */
void AdcInit(void)
{
    /* GPIOA clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 (Soil), PA1 (MQ135) analog mode */
    GPIOA->MODER |= (3U << (0 * 2)) | (3U << (1 * 2));

    /* ADC1 clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* 12-bit resolution, single conversion */
    ADC1->CR1 &= ~ADC_CR1_RES;
    ADC1->CR2 &= ~ADC_CR2_CONT;

    /* Long sampling time */
    ADC1->SMPR2 |= (7U << (3 * 0));
    ADC1->SMPR2 |= (7U << (3 * 1));

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* PA5 as Relay output */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->OSPEEDR |=  (3U << (5 * 2));
    GPIOA->PUPDR &= ~(3U << (5 * 2));
}

/* =========================================================
   GENERIC ADC READ
   ========================================================= */
uint16_t AdcReadChannel(uint8_t channel)
{
    ADC1->SQR1 = 0;
    ADC1->SQR3 = channel;

    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;

    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

/* =========================================================
   SOIL MOISTURE CONTROL
   ========================================================= */
void MoistureControl(void)
{
    uint16_t adc = AdcReadChannel(0);

    int moisture =
        (ADC_SOIL_DRY - adc) * 100 /
        (ADC_SOIL_DRY - ADC_SOIL_WET);

    if (moisture < 0)   moisture = 0;
    if (moisture > 100) moisture = 100;

    if (moisture < MOISTURE_LOW_THRESHOLD)
        GPIOA->ODR &= ~(1U << 5);   // Pump ON (active LOW)
    else if (moisture > MOISTURE_HIGH_THRESHOLD)
        GPIOA->ODR |= (1U << 5);    // Pump OFF

    char buf[80];
    sprintf(buf,
        "Soil ADC=%u Moisture=%d%% Pump=%s\r\n",
        adc,
        moisture,
        (moisture < MOISTURE_LOW_THRESHOLD) ? "ON" : "OFF");

    UartPuts(buf);
}

/* =========================================================
   MQ135 HELPER FUNCTIONS
   ========================================================= */
static float MQ135_CalcRs(uint16_t adc)
{
    if (adc == 0) adc = 1;
    return RL_VALUE * (ADC_MAX - adc) / adc;
}

/* Run once in clean air */
void MQ135_Calibrate(void)
{
    uint16_t adc = AdcReadChannel(1);
    float Rs = MQ135_CalcRs(adc);

    MQ135_R0 = Rs / CLEAN_AIR_RATIO;

    char buf[80];
    sprintf(buf,
        "MQ135 Calibrated: ADC=%u R0=%.2f\r\n",
        adc, MQ135_R0);

    UartPuts(buf);
}

/* =========================================================
   MQ135 READ
   ========================================================= */
void MQ135Read(void)
{
    if (MQ135_R0 == 0.0f)
    {
        UartPuts("MQ135 not calibrated!\r\n");
        return;
    }

    uint16_t adc = AdcReadChannel(1);
    float Rs = MQ135_CalcRs(adc);
    float ratio = Rs / MQ135_R0;

    char buf[80];
    sprintf(buf,
        "MQ135 ADC=%u Rs=%.2f Rs/R0=%.2f\r\n",
        adc, Rs, ratio);

    UartPuts(buf);
}
