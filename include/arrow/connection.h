/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_CONNECTION_H_
#define ARROW_CONNECTION_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "gateway.h"
#include "device.h"
#include "request.h"

#include <bsd/socket.h>

typedef int (*get_data_cb)(void *);

int arrow_prepare_gateway(arrow_gateway_t *gateway);
int arrow_register_gateway(arrow_gateway_t *gateway);
int arrow_prepare_device(arrow_gateway_t *gateway, arrow_device_t *device);
int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device);
int arrow_heartbeat(arrow_gateway_t *gateway);
int arrow_checkin(arrow_gateway_t *gateway);
int arrow_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config);

int arrow_send_telemetry(arrow_device_t *device, void *data);

int arrow_connect_gateway(arrow_gateway_t *gateway);
int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device);

int arrow_initialize_routine();
int arrow_update_state(const char *name, const char *value);
int arrow_send_telemetry_routine(void *data);
int arrow_mqtt_connect_routine();

void arrow_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data);
void arrow_close();

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_CONNECTION_H_ */
