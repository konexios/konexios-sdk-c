/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/storage.h"
#include <arrow/utf8.h>

int restore_gateway_info(arrow_gateway_t *gateway) {
    // FIXME arrow_gateway_add_hid(gateway, gateway_hid);
    return -1;
}

void save_gateway_info(const arrow_gateway_t *gateway) {
  // FIXME save gateway->hid
}

int restore_device_info(arrow_device_t *device) {
  // FIXME arrow_device_set_hid(device, device_hid);
  // and arrow_device_set_eid(device, device_eid);
  // if defined(__IBM__)
  return -1;
}

void save_device_info(arrow_device_t *device) {
  // FIXME save device->hid
  // and device->eid
  // if defined(__IBM__)
}

void save_wifi_setting(const char *ssid, const char *pass, int sec) {
  // FIXME save this wifi settings
}

int restore_wifi_setting(char *ssid, char *pass, int *sec) {
    // FIXME restore this wifi settings
    return 0;
}
