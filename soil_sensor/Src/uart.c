/*
 * uart.c
 *  Register-level UART2 driver for STM32F4
 */

#include "uart.h"
#include "stm32f4xx.h"

void UartInit(uint32_t baud) {

    /* GPIOA clock enable */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Configure PA2, PA3 as Alternate Function (AF7) */
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2))); // clear mode
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2))); // AF mode

    GPIOA->AFR[0] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4))); // clear AF
    GPIOA->AFR[0] |=  ((7 << (2 * 4)) | (7 << (3 * 4)));     // AF7 = USART2

    /* Enable USART2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* UART settings: 8N1, TE, RE */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;

    /* Baud rate setup */
    if (baud == 9600)
        USART2->BRR = 0x0683;
    else if (baud == 38400)
        USART2->BRR = 0x01A1;
    else if (baud == 115200)
        USART2->BRR = 0x008B;

    /* Enable UART */
    USART2->CR1 |= USART_CR1_UE;
}

void UartPutch(int ch) {
    USART2->DR = (uint16_t)ch;
    while (!(USART2->SR & USART_SR_TXE)); // wait until empty
}

void UartPuts(char *str) {
    while (*str) {
        UartPutch(*str++);
    }
}

int UartGetch(void) {
    while (!(USART2->SR & USART_SR_RXNE)); // wait for data
    return USART2->DR;
}
