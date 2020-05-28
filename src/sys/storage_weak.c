/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <konexios_config.h>
#include "arrow/storage.h"

void __attribute_weak__ save_key_setting(const char *api_key, const char *sec_key) {
    SSP_PARAMETER_NOT_USED(api_key);
    SSP_PARAMETER_NOT_USED(sec_key);
}

int __attribute_weak__ restore_key_setting(char *api, char *sec) {
    if (api) strcpy(api, DEFAULT_API_KEY);
    if (sec) strcpy(sec, DEFAULT_SECRET_KEY);
    return 0;
}

void __attribute_weak__ save_api_address(arrow_host_t *host) {
    SSP_PARAMETER_NOT_USED(host);
}

int __attribute_weak__ restore_api_address(arrow_host_t *host) {
    host->host   = iotClientInitApiDefault.apihost;
    host->port   = iotClientInitApiDefault2.apiport;
    host->scheme = iotClientInitApiDefault2.api_scheme;
    return 0;
}

void __attribute_weak__ save_mqtt_address(arrow_host_t *host) {
    SSP_PARAMETER_NOT_USED(host);
}

int __attribute_weak__ restore_mqtt_address(arrow_host_t *host) {
    host->host = MQTT_TELEMETRY_ADDR;
    host->port = MQTT_PORT;
    host->scheme = MQTT_SCH;
    return 0;
}

void __attribute_weak__ save_vhost(property_t vh) {
    SSP_PARAMETER_NOT_USED(vh);
}

int __attribute_weak__ restore_vhost(property_t *vh) {
    property_copy(vh, p_const(VHOST));
    return 0;
}

void __attribute_weak__ save_apphid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
}

int __attribute_weak__ restore_apphid_address(char *hid) {
#if defined(DEFAULT_APP_HID)
    if (hid) strcpy(hid, DEFAULT_APP_HID);
    return 0;
#else
    return -1;
#endif
}

void __attribute_weak__ save_userhid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
}

int __attribute_weak__ restore_userhid_address(char *hid) {
#if defined(DEFAULT_USER_HID)
    if (hid) strcpy(hid, DEFAULT_USER_HID);
    return 0;
#else
    return -1;
#endif
}
