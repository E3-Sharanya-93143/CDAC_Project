/*
 * uart.h
 *
 *  Created on: Apr 1, 2024
 *      Author: Nilesh
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"

/* Baud rate options */
#define BAUD_9600        9600
#define BAUD_38400       38400
#define BAUD_115200      115200

/* BRR values (for 16 MHz APB clock) */
#define BAUD_BRR_9600    0x683
#define BAUD_BRR_38400   0x1A1
#define BAUD_BRR_115200  0x8B

/* UART selection */
typedef enum
{
    UART_PC = 0,     // USART2 (PA2, PA3)
    UART_ESP         // USART1 (PA9, PA10)
} uart_t;

/* APIs */
void UartInit(uart_t uart, uint32_t baud);
void UartPutch(uart_t uart, uint8_t ch);
uint8_t UartGetch(uart_t uart);
void UartPuts(uart_t uart, char str[]);
void UartGets(uart_t uart, char str[]);

#endif /* UART_H_ */

