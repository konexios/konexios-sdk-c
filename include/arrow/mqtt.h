/*
 * mqtt.h
 *
 *  Created on: 29 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef ARROW_MQTT_H_
#define ARROW_MQTT_H_

#include "gateway.h"
#include "device.h"

int mqtt_connect(arrow_gateway_t *gateway,
                 arrow_device_t *device,
                 arrow_gateway_config_t *config);
void mqtt_disconnect();

int mqtt_subscribe();
int mqtt_publish(arrow_device_t *device, void *data);



#endif /* ARROW_MQTT_H_ */
