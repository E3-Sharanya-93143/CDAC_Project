/*
 * esp8266.c
 *
 *  Created on: Jan 26, 2026
 *      Author: sunbeam
 */

/*
 * esp8266.c
 *
 *  Created on: Jan 26, 2026
 *      Author: sunbeam
 */

#include "esp8266.h"
#include "uart.h"
#include <string.h>   // For strstr

#define MAX_RETRIES 3

// Helper function: send command and check "OK" response
static int ESP8266_SendCmd(const char *cmd, uint32_t delay_ms)
{
    char resp[100];
    int retries = 0;

    while (retries < MAX_RETRIES)
    {
        UartPuts(UART_ESP, (char *)cmd);   // Send command
        DelayMs(delay_ms);          // Wait for ESP to respond

        // Read response from ESP8266
        UartGets(UART_ESP, resp);

        // Debug: print response to PC
        UartPuts(UART_PC, resp);

        // Check if response contains "OK"
        if (strstr(resp, "OK") != NULL)
        {
            return 1;   // Success
        }

        retries++;
    }

    return 0;   // Failed after retries
}

void ESP8266_Init(void)
{
    // 1. Check AT communication
    if (!ESP8266_SendCmd("AT\r\n", 500))
        UartPuts(UART_PC, "ERROR: ESP AT command failed!\r\n");

    // 2. Set Wi-Fi mode to Station
    if (!ESP8266_SendCmd("AT+CWMODE=1\r\n", 500))
        UartPuts(UART_PC, "ERROR: ESP CWMODE failed!\r\n");

    // 3. Connect to Wi-Fi network
    if (!ESP8266_SendCmd("AT+CWJAP=\"Redmi\",\"1234567890@\"\r\n", 8000))
        UartPuts(UART_PC, "ERROR: ESP Wi-Fi connect failed!\r\n");

    // 4. Configure MQTT parameters (client ID = "stm32")
    if (!ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\"stm32\",\"gajjelasharanya5@gmail.com\",\"R1IWMM0QFFAD724F\",0,0,\"\"\r\n", 500))
        UartPuts(UART_PC, "ERROR: ESP MQTT config failed!\r\n");

    // 5. Connect to ThingSpeak MQTT broker
    if (!ESP8266_SendCmd("AT+MQTTCONN=0,\"mqtt3.thingspeak.com\",1883,0\r\n", 3000))
        UartPuts(UART_PC, "ERROR: ESP MQTT connect failed!\r\n");

    UartPuts(UART_PC, "ESP8266 initialization completed.\r\n");
}
