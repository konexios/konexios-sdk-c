/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef KONEXIOS_ROUTINE_H_
#define KONEXIOS_ROUTINE_H_

#if defined(__cplusplus)
extern "C" {
#endif
#include <konexios/gateway.h>
#include <konexios/device.h>

typedef enum {
#if defined(VALGRIND_TEST)
    ROUTINE_TEST_DONE   = -1000,
#endif
    ROUTINE_SUCCESS               = 0,
    ROUTINE_RECEIVE_EVENT         = 1,
    ROUTINE_ERROR                 = -300,
    ROUTINE_NOT_INITIALIZE        = -301,
    ROUTINE_GET_TELEMETRY_FAILED  = -302,
    ROUTINE_MQTT_PUBLISH_FAILED   = -303,
    ROUTINE_MQTT_CONNECT_FAILED   = -304,
    ROUTINE_MQTT_SUBSCRIBE_FAILED = -305
} konexios_routine_error_t;

typedef int (*get_data_cb)(void *);

// There is only one gateway and device for this space
// to get these static objects use the functions:
konexios_device_t *konexios_get_current_device(void);
konexios_gateway_t *konexios_get_current_gateway(void);
konexios_gateway_config_t *konexios_get_current_gateway_config(void);

// Initialize the gateway object and device object as well
// This function implemented the algorithm to get complete information about a gateway and device.
// The WDT function is used.
konexios_routine_error_t konexios_init(void) __attribute_warn_unused_result__;
konexios_routine_error_t konexios_deinit(void);

konexios_routine_error_t konexios_initialize_routine(bool update_gateway_info) __attribute_warn_unused_result__;

konexios_routine_error_t konexios_initialize_routine(bool update_gateway_info);

int konexios_startup_sequence(bool update_gateway_info);

// Routine function for terminating current connections with the cloud
// and terminate all gateway/device information.
void konexios_close(void);

// If there is no the stored gateway information
// form the register gateway request and save a taken information
// In other case just form the gateway checkin request
int konexios_connect_gateway(konexios_gateway_t *gateway,bool update_gateway_info);

// If there is no any device information form the device register request
// and save the taken information
int konexios_connect_device(konexios_gateway_t *gateway, konexios_device_t *device, bool update_device_info);

// Routine function for telemetry sending to the cloud
// there is extremely needed the telemetry_serialize function implementation to serealize 'data' correctly
konexios_routine_error_t konexios_send_telemetry_routine(void *data);

// Funtion set the new state for this device
konexios_routine_error_t konexios_device_states_sync();
konexios_routine_error_t konexios_device_states_update();

// Routine for MQTT connection establishing
// Automatically prepare needed information and send it to the cloud MQTT
konexios_routine_error_t konexios_mqtt_connect_routine(void);
konexios_routine_error_t konexios_mqtt_disconnect_routine(void);
konexios_routine_error_t konexios_mqtt_terminate_routine(void);
konexios_routine_error_t konexios_mqtt_pause_routine(int pause);

// telemetry specific
konexios_routine_error_t konexios_mqtt_connect_telemetry_routine(void);
konexios_routine_error_t konexios_mqtt_disconnect_telemetry_routine(void);
konexios_routine_error_t konexios_mqtt_terminate_telemetry_routine(void);

// command specific
konexios_routine_error_t konexios_mqtt_connect_event_routine(void);
konexios_routine_error_t konexios_mqtt_subscribe_event_routine(void);
konexios_routine_error_t konexios_mqtt_disconnect_event_routine(void);
konexios_routine_error_t konexios_mqtt_terminate_event_routine(void);

// This routine send the telemetry data every TELEMETRY_DELAY msec
// using the data_cb function for forming current telemetry values
konexios_routine_error_t konexios_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data);

konexios_routine_error_t konexios_mqtt_telemetry_routine(get_data_cb data_cb, void *data);
konexios_routine_error_t konexios_mqtt_telemetry_once_routine(get_data_cb data_cb, void *data);
konexios_routine_error_t konexios_mqtt_event_receive_routine(void);
konexios_routine_error_t konexios_mqtt_check_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* KONEXIOS_ROUTINE_H_ */
