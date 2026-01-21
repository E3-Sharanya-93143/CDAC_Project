#ifndef UART_H
#define UART_H

#include <stdint.h>

void UartInit(uint32_t baud);
void UartPutch(int ch);
void UartPuts(char *str);
int UartGetch(void);

#endif
