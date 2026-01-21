#include "adc.h"
#include "uart.h"
#include "stm32f4xx.h"
#include<stdio.h>

void AdcInit(void) {

    /* GPIOA clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 as Analog */
    GPIOA->MODER |= (3U << (0 * 2));

    /* ADC1 clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC config: 12-bit, single conversion */
    ADC1->CR1 &= ~(ADC_CR1_RES);//// 12-bit resolution
    ADC1->CR2 &= ~ADC_CR2_CONT; //// Single conversion
    ADC1->SQR1 &= ~(ADC_SQR1_L);// 1 conversion
    ADC1->SQR3  = 0; // Channel 0

    ADC1->CR2 |= ADC_CR2_ADON; // ADC Enabled

    /* PA5 as output (Relay) */
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |=  (1U << (5 * 2));//output mode

    GPIOA->OTYPER &= ~(1U << 5);//push pull
    GPIOA->OSPEEDR |=  (3U << (5 * 2));//high speed
    GPIOA->PUPDR   &= ~(3U << (5 * 2));//no pull up
}


// Read ADC value
uint16_t AdcRead(void) {

    ADC1->CR2 |= ADC_CR2_SWSTART;//Start conversion
    while (!(ADC1->SR & ADC_SR_EOC));//Wait until conversion completes (EOC flag)
    return ADC1->DR;//Read ADC data register
}

   //MoistureControl()
   // Controls relay (PA5)
   // Uses thresholds lower=40%, upper=50%
void MoistureControl(void)
{
    uint16_t adc_val = AdcRead();

    // Moisture percentage
    int moisturePercent = (adc_val / 4095.0f) * 100.0f;

    // Relay control thresholds
    int lower = 40;
    int upper = 50;

    // Control relay
    if (moisturePercent < lower) {
        GPIOA->ODR &= ~(1U << 5);    // Pump ON
    } else if (moisturePercent > upper) {
        GPIOA->ODR |= (1U << 5);     // Pump OFF
    }

    // Prepare output string
    char output[60];
    if (moisturePercent < lower)
        sprintf(output, "ADC=%u  Moisture=%d%%  PUMP ON\r\n", adc_val, moisturePercent);
    else
        sprintf(output, "ADC=%u  Moisture=%d%%  PUMP OFF\r\n", adc_val, moisturePercent);

    // Send to UART
    UartPuts(output);

    // Small software delay
    for (volatile int i = 0; i < 300000; i++);
}

