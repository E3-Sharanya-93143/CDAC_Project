/*
 * mqtt_thingspeak.h
 *
 *  Created on: Jan 26, 2026
 *      Author: sunbeam
 */

#ifndef MQTT_THINGSPEAK_H_
#define MQTT_THINGSPEAK_H_

#include <stdint.h>

void ThingSpeak_MQTTSend(float temp,
                         float hum,
                         uint16_t lux,
                         int soil,
                         uint16_t gas);


#endif /* MQTT_THINGSPEAK_H_ */
