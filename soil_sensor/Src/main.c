#include "adc.h"
#include "uart.h"
#include "delay.h"

int main(void) {

    UartInit(9600);
    UartPuts("Soil Moisture Monitoring\r\n");

    AdcInit();

    while (1) {
        MoistureControl();        // Reads ADC, controls pump, prints via UART
        DelayMs(500);             // 500 ms delay
    }
}



