/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <konexios_config.h>
#include "konexios/storage.h"

void __attribute_weak__ save_key_setting(const char *api_key, const char *sec_key) {
    SSP_PARAMETER_NOT_USED(api_key);
    SSP_PARAMETER_NOT_USED(sec_key);
}

int __attribute_weak__ restore_key_setting(char *api, char *sec) {
    if (api) strcpy(api, iotClientInitKey.apikey);
    if (sec) strcpy(sec, iotClientInitKey.secretkey);
    return 0;
}

void __attribute_weak__ save_api_address(konexios_host_t *host) {
    SSP_PARAMETER_NOT_USED(host);
}

int __attribute_weak__ restore_api_address(konexios_host_t *host) {
    host->host   = iotClientInitApi.host;
    host->port   = iotClientInitApi.port;
    host->scheme = iotClientInitApi.scheme;
    return 0;
}

void __attribute_weak__ save_mqtt_address(konexios_host_t *host) {
    SSP_PARAMETER_NOT_USED(host);
}

int __attribute_weak__ restore_mqtt_address(konexios_host_t *host) {
	host->host = iotClientInitMqtt.host;
    host->port = iotClientInitMqtt.port;
    host->scheme = iotClientInitMqtt.scheme;
    return 0;
}

void __attribute_weak__ save_vhost(property_t vh) {
    SSP_PARAMETER_NOT_USED(vh);
}

int __attribute_weak__ restore_vhost(property_t *vh) {
    property_copy(vh, p_const(iotClientInitMqtt.virtualhost));
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
    (void)hid;
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
    (void)hid;
    return -1;
#endif
}
