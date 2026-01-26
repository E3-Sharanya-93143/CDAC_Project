
#include "adc.h"
#include "uart.h"
#include "stm32f4xx.h"
#include <stdio.h>

/* ---------- ADC INIT ---------- */
void AdcInit(void)
{
    /* GPIOA clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 (Soil), PA1 (MQ135) as Analog */
    GPIOA->MODER |= (3U << (0 * 2)) | (3U << (1 * 2));

    /* ADC1 clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC config: 12-bit, single conversion */
    ADC1->CR1 &= ~(ADC_CR1_RES);
    ADC1->CR2 &= ~ADC_CR2_CONT;

    /* Long sampling time (important for MQ135) */
    ADC1->SMPR2 |= (7 << (3 * 0)); // Channel 0
    ADC1->SMPR2 |= (7 << (3 * 1)); // Channel 1

    ADC1->CR2 |= ADC_CR2_ADON;

    /* PA5 as output (Relay) */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));

    GPIOA->OTYPER &= ~(1U << 5);
    GPIOA->OSPEEDR |=  (3U << (5 * 2));
    GPIOA->PUPDR   &= ~(3U << (5 * 2));
}

/* ---------- GENERIC ADC READ ---------- */
uint16_t AdcReadChannel(uint8_t channel)
{
    ADC1->SQR1 = 0;          // 1 conversion
    ADC1->SQR3 = channel;   // Select channel

    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));

    return ADC1->DR;
}

int Moisture_Read(void)
{
    uint16_t adc_val = AdcReadChannel(0);
    int moisture = (adc_val * 100) / 4095;

    if (moisture < 40)
        GPIOA->ODR &= ~(1U << 5); // Pump ON
    else
        GPIOA->ODR |= (1U << 5);  // Pump OFF

    return moisture;
}

uint16_t MQ135_Read(void)
{
    return AdcReadChannel(1);
}

///* ---------- SOIL MOISTURE ---------- */
//void MoistureControl(void)
//{
//    uint16_t adc_val = AdcReadChannel(0); // FIXED
//
//    int moisturePercent = (adc_val * 100) / 4095;
//
//    int lower = 40;
//    int upper = 50;
//
//    if (moisturePercent < lower) {
//        GPIOA->ODR &= ~(1U << 5);    // Pump ON
//    } else if (moisturePercent > upper) {
//        GPIOA->ODR |= (1U << 5);     // Pump OFF
//    }
//
//    char output[60];
//    sprintf(output,
//            "Soil ADC=%u  Moisture=%d%%  Pump:%s\r\n",
//            adc_val,
//            moisturePercent,
//            (moisturePercent < lower) ? "ON" : "OFF");
//
//    //UartPuts(output);
//}
//
///* ---------- MQ135 SENSOR ---------- */
//void MQ135Read(void)
//{
//    uint16_t adc_val = AdcReadChannel(1); // PA1
//
//    char output[60];
//    sprintf(output, "MQ135 ADC=%u\r\n", adc_val);
//    //UartPuts(output);
//}

