#include "stm32f4xx.h"
#include "bme280.h"
#include "i2c.h"
#include "uart.h"
#include <stdint.h>
#include<stdio.h>

#define RELAY_PIN      5       // PB5
#define TEMP_THRESHOLD 30.0f   // °C

void Relay_Init(void) {
    RCC->AHB1ENR |= (1 << 1);  // GPIOB clock
    GPIOB->MODER &= ~(0x3 << (RELAY_PIN*2));
    GPIOB->MODER |=  (0x1 << (RELAY_PIN*2));
    GPIOB->OTYPER &= ~(1<<RELAY_PIN);
    GPIOB->PUPDR &= ~(0x3 << (RELAY_PIN*2));
    GPIOB->OSPEEDR &= ~(0x3 << (RELAY_PIN*2));
    GPIOB->ODR &= ~(1<<RELAY_PIN); // OFF initially
}

void Relay_On(void)  { GPIOB->ODR |= (1<<RELAY_PIN); }
void Relay_Off(void) { GPIOB->ODR &= ~(1<<RELAY_PIN); }

void delay(volatile uint32_t count) { while(count--); }

int main(void) {
    float temperature, pressure, humidity;

    // 1. Initialize I2C (for BME280)
    I2CInit();

    // 2. Initialize UART
    UartInit(BAUD_115200);

    // 3. Initialize relay
    Relay_Init();

    // 4. Initialize BME280
    if(BME280_Init() != 0) {
        UartPuts("BME280 Init Failed!\r\n");
        while(1);
    }
    UartPuts("BME280 Init Successful\r\n");

    while(1) {
        // Read sensor
        BME280_ReadAll(&temperature, &pressure, &humidity);

        // Print values via UART
        char buf[64];
        sprintf(buf, "Temp: %.2f C, Hum: %.2f %%\r\n", temperature, humidity); // no variable

        UartPuts(buf);

        // Temperature control
        //if (temperature >= TEMP_THRESHOLD) {
        //  Relay_On();
        //    UartPuts("Fan ON\r\n");
        //} else {
        //      Relay_Off();
        //    UartPuts("Fan OFF\r\n");
    //    }

        delay(500000); // crude delay
    }
}
