
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void AdcInit(void);
uint16_t AdcReadChannel(uint8_t channel);
uint8_t SoilMoistureControl(uint8_t *moisture_percent);
uint8_t MQ135_GetValue(uint16_t *mq135_adc);

#endif
