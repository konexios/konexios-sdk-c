/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_STORAGE_H_
#define ARROW_STORAGE_H_

#include <arrow/device.h>
#include <arrow/gateway.h>

int restore_gateway_info(arrow_gateway_t *gateway);
void save_gateway_info(const arrow_gateway_t *gateway);

int restore_device_info(arrow_device_t *device);
void save_device_info(arrow_device_t *device);

void save_wifi_setting(const char *ssid, const char *pass, int sec);
int restore_wifi_setting(char *ssid, char *pass, int *sec);

#endif
