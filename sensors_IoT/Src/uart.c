/*
 * uart.c
 *
 *  Created on: Apr 1, 2024
 *      Author: Nilesh
 */
#include "uart.h"

/* Alternate Function number for USART on STM32F4 */
#define USART_AF   7

/* --------------------------------------------------
   UART INITIALIZATION FUNCTION
   - Supports two UARTs:
     UART_PC  -> USART2 (PA2, PA3) for PC debugging
     UART_ESP -> USART1 (PA9, PA10) for ESP8266
-------------------------------------------------- */
void UartInit(uart_t uart, uint32_t baud)
{
    uint32_t brr = 0;

    /* Select correct BRR value based on baud rate */
    switch (baud)
    {
        case BAUD_9600:
            brr = BAUD_BRR_9600;
            break;

        case BAUD_38400:
            brr = BAUD_BRR_38400;
            break;

        case BAUD_115200:
            brr = BAUD_BRR_115200;
            break;

        default:
            brr = BAUD_BRR_115200;   // Default safe baud
    }

    /* ---------------- USART2 → PC DEBUG ---------------- */
    if (uart == UART_PC)
    {
        /* Enable GPIOA and USART2 clocks */
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

        /* Configure PA2 (TX) and PA3 (RX) as Alternate Function */
        GPIOA->MODER |= (2 << (2*2)) | (2 << (3*2));

        /* Select AF7 (USART) for PA2 and PA3 */
        GPIOA->AFR[0] |= (USART_AF << (2*4)) | (USART_AF << (3*4));

        /* Enable Transmitter and Receiver */
        USART2->CR1 = USART_CR1_TE | USART_CR1_RE;

        /* Set baud rate */
        USART2->BRR = brr;

        /* Enable USART */
        USART2->CR1 |= USART_CR1_UE;
    }

    /* ---------------- USART1 → ESP8266 ---------------- */
    else if (uart == UART_ESP)
    {
        /* Enable GPIOA and USART1 clocks */
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

        /* Configure PA9 (TX) and PA10 (RX) as Alternate Function */
        GPIOA->MODER |= (2 << (9*2)) | (2 << (10*2));

        /* Select AF7 (USART) for PA9 and PA10 */
        GPIOA->AFR[1] |= (USART_AF << ((9-8)*4)) |
                         (USART_AF << ((10-8)*4));

        /* Enable Transmitter and Receiver */
        USART1->CR1 = USART_CR1_TE | USART_CR1_RE;

        /* Set baud rate */
        USART1->BRR = brr;

        /* Enable USART */
        USART1->CR1 |= USART_CR1_UE;
    }
}

/* --------------------------------------------------
   SEND A SINGLE CHARACTER OVER UART
-------------------------------------------------- */
void UartPutch(uart_t uart, uint8_t ch)
{
    /* Select correct USART peripheral */
    USART_TypeDef *U = (uart == UART_PC) ? USART2 : USART1;

    /* Wait until transmit data register is empty */
    while (!(U->SR & USART_SR_TXE));

    /* Write data to transmit register */
    U->DR = ch;
}

/* --------------------------------------------------
   RECEIVE A SINGLE CHARACTER FROM UART
-------------------------------------------------- */
uint8_t UartGetch(uart_t uart)
{
    /* Select correct USART peripheral */
    USART_TypeDef *U = (uart == UART_PC) ? USART2 : USART1;

    /* Wait until data is received */
    while (!(U->SR & USART_SR_RXNE));

    /* Read received data */
    return (uint8_t)U->DR;
}

/* --------------------------------------------------
   SEND A STRING OVER UART
-------------------------------------------------- */
void UartPuts(uart_t uart, char str[])
{
    int i = 0;

    /* Transmit characters until NULL terminator */
    while (str[i] != '\0')
    {
        UartPutch(uart, str[i]);
        i++;
    }
}

/* --------------------------------------------------
   RECEIVE A STRING OVER UART
   - Reads until carriage return '\r'
-------------------------------------------------- */
void UartGets(uart_t uart, char str[])
{
    char ch;
    int i = 0;

    /* Read characters until Enter key */
    do
    {
        ch = UartGetch(uart);
        str[i++] = ch;
    }
    while (ch != '\r');

    /* Append newline and string terminator */
    str[i++] = '\n';
    str[i] = '\0';
}
