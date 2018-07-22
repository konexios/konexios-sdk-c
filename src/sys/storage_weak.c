/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/storage.h"

void __attribute_weak__ save_api_address(arrow_host_t *host) {
    SSP_PARAMETER_NOT_USED(host);
}

int __attribute_weak__ restore_api_address(arrow_host_t *host) {
    host->host = ARROW_ADDR;
    host->port = ARROW_PORT;
    host->scheme = ARROW_SCH;
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

void __attribute_weak__ save_apphid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
}

int __attribute_weak__ restore_apphid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
    return -1;
}

void __attribute_weak__ save_userhid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
}

int __attribute_weak__ restore_userhid_address(char *hid) {
    SSP_PARAMETER_NOT_USED(hid);
    return -1;
}
