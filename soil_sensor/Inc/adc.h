#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void AdcInit(void);          // Initialize ADC and relay
uint16_t AdcRead(void);      // Read ADC value
void MoistureControl(void);  // Control pump, print ADC & moisture %

#endif /* ADC_H */
