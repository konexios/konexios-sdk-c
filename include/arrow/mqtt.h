/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
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
int mqtt_yield(int timeout_ms);
int mqtt_publish(arrow_device_t *device, void *data);



#endif /* ARROW_MQTT_H_ */
