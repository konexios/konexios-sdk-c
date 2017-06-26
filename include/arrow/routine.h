/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_ROUTINE_H_
#define ARROW_ROUTINE_H_

#if defined(__cplusplus)
extern "C" {
#endif
#include <http/routine.h>
#include <arrow/connection.h>

typedef int (*get_data_cb)(void *);

arrow_device_t *current_device(void);
arrow_gateway_t *current_gateway(void);

int arrow_connect_gateway(arrow_gateway_t *gateway);
int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device);

int arrow_mqtt_connect_routine(void);
int arrow_initialize_routine(void);
int arrow_send_telemetry_routine(void *data);
int arrow_update_state(const char *name, const char *value);
void arrow_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data);
void arrow_close(void);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_ROUTINE_H_ */
