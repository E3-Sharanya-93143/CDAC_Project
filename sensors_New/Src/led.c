#include "led.h"

void LedInit(void)
{
    // Enable GPIOE clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // Set PE0–PE3 as output mode
    LED_PORT->MODER &= ~(
        BV(LED1_PIN * 2 + 1) |
        BV(LED2_PIN * 2 + 1) |
        BV(LED3_PIN * 2 + 1) |
        BV(LED4_PIN * 2 + 1)
    );
    LED_PORT->MODER |= (
        BV(LED1_PIN * 2 + 0) |
        BV(LED2_PIN * 2 + 0) |
        BV(LED3_PIN * 2 + 0) |
        BV(LED4_PIN * 2 + 0)
    );

    // Push-pull output
    LED_PORT->OTYPER &= ~(
        BV(LED1_PIN) |
        BV(LED2_PIN) |
        BV(LED3_PIN) |
        BV(LED4_PIN)
    );

    // Low speed
    LED_PORT->OSPEEDR &= ~(
        (3 << (LED1_PIN * 2)) |
        (3 << (LED2_PIN * 2)) |
        (3 << (LED3_PIN * 2)) |
        (3 << (LED4_PIN * 2))
    );

    // No pull-up/down
    LED_PORT->PUPDR &= ~(
        (3 << (LED1_PIN * 2)) |
        (3 << (LED2_PIN * 2)) |
        (3 << (LED3_PIN * 2)) |
        (3 << (LED4_PIN * 2))
    );

    //LedsOff();
}
/*
void LedsOn(void)
{
    LED_PORT->BSRR =
        BV(LED1_PIN) |
        BV(LED2_PIN) |
        BV(LED3_PIN) |
        BV(LED4_PIN);
}

void LedsOff(void)
{
    LED_PORT->BSRR =
        BV(LED1_PIN + 16) |
        BV(LED2_PIN + 16) |
        BV(LED3_PIN + 16) |
        BV(LED4_PIN + 16);
}

void LedsToggle(void)
{
    LED_PORT->ODR ^= (
        BV(LED1_PIN) |
        BV(LED2_PIN) |
        BV(LED3_PIN) |
        BV(LED4_PIN)
    );
}
*/
