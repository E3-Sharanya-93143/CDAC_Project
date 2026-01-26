
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void AdcInit(void);          // Initialize ADC and relay
uint16_t AdcRead(void);      // Read ADC value
int Moisture_Read(void);  // Control pump, print ADC & moisture %
uint16_t MQ135_Read(void);

#endif /* ADC_H */
