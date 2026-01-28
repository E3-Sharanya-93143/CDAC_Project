#include "adc.h"
#include "stm32f4xx.h"

/* ================= USER CALIBRATION ================= */
#define ADC_SOIL_DRY   3100   // Dry soil
#define ADC_SOIL_WET   1200   // Wet soil
#define MOISTURE_LOW_THRESHOLD   40   // Pump ON
#define MOISTURE_HIGH_THRESHOLD  50   // Pump OFF

/* ================= INITIALIZATION ================= */
void AdcInit(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // PA0 → Soil, PA1 → MQ135
    GPIOA->MODER |= (3U << (0 * 2)) | (3U << (1 * 2)); // Analog mode

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    ADC1->CR1 &= ~ADC_CR1_RES;   // 12-bit
    ADC1->CR2 &= ~ADC_CR2_CONT;  // Single conversion

    ADC1->SMPR2 |= (7U << (3*0)) | (7U << (3*1)); // long sampling

    ADC1->CR2 |= ADC_CR2_ADON;

    // PA5 → Pump relay output
    GPIOA->MODER &= ~(3U << (5*2));
    GPIOA->MODER |=  (1U << (5*2));
    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->OSPEEDR |=  (3U << (5*2));
    GPIOA->PUPDR &= ~(3U << (5*2));
    GPIOA->ODR |= (1U << 5); // Pump OFF initially
}

/* ================= GENERIC ADC READ ================= */
uint16_t AdcReadChannel(uint8_t channel)
{
    ADC1->SQR1 = 0;        // 1 conversion
    ADC1->SQR3 = channel;  // select channel

    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;

    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

/* ================= SOIL MOISTURE CONTROL ================= */
uint8_t SoilMoistureControl(uint8_t *moisture_percent)
{
    uint16_t adc_value = AdcReadChannel(0); // PA0
    int moisture = ((int)adc_value - ADC_SOIL_WET) * 100 / (ADC_SOIL_DRY - ADC_SOIL_WET);

    if (moisture < 0) moisture = 0;
    if (moisture > 100) moisture = 100;

    *moisture_percent = (uint8_t)moisture;

    // Pump logic (active LOW)
    if (moisture < MOISTURE_LOW_THRESHOLD)
    {
        GPIOA->ODR &= ~(1U << 5); // Pump ON
        return 1;
    }
    else if (moisture > MOISTURE_HIGH_THRESHOLD)
    {
        GPIOA->ODR |= (1U << 5);  // Pump OFF
        return 0;
    }
    else
    {
        return (GPIOA->ODR & (1U << 5)) ? 0 : 1;
    }
}

/* ================= MQ135 READ ================= */
uint8_t MQ135_GetValue(uint16_t *mq135_adc)
{
    *mq135_adc = AdcReadChannel(1); // PA1 → MQ135
    return 1;
}
