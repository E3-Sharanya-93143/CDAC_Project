/*
 * mqtt_thingspeak.c
 *
 *  Created on: Jan 26, 2026
 *      Author: sunbeam
 */

#include "mqtt_thingspeak.h"
#include "uart.h"
#include <stdio.h>

#define CHANNEL_ID   "3239645"
#define MQTT_TOPIC   "channels/" CHANNEL_ID "/publish"

void ThingSpeak_MQTTSend(float temp,
                         float hum,
                         uint16_t lux,
                         int soil,
                         uint16_t gas)
{
    char cmd[200];

    sprintf(cmd,
        "AT+MQTTPUB=\"%s\",\"field1=%.2f&field2=%.2f&field3=%u&field4=%d&field5=%u\",0,0\r\n",
        MQTT_TOPIC,
        temp,
        hum,
        lux,
        soil,
        gas);

    // Send MQTT publish over ESP UART
    UartPuts(UART_ESP, cmd);
}

