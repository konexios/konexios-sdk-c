/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_STORAGE_H_
#define ARROW_STORAGE_H_

#if defined (__cplusplus)
extern "C" {
#endif

#include <arrow/device.h>
#include <arrow/gateway.h>

#if !defined(FLASH_MAGIC_NUMBER)
  #if defined(DEV_ENV)
    #define FLASH_MAGIC_NUMBER 0xdea1beac
  #else
    #define FLASH_MAGIC_NUMBER 0xdeadbeaf
  #endif
#endif

// default structure for the device information
typedef struct store_host {
    char host[52];
    uint16_t scheme;
    uint16_t port;
} store_host_t;

typedef struct _api_keys_ {
    char api[72];
    char secret[56];
} api_keys_t;

typedef struct {
  int magic;
  char ssid[32];
  char pass[64];
  char vhost[32];
  int sec;
  int type;
  store_host_t api;
  store_host_t mqtt;
  int padding;
  char gateway_hid[64];
  char device_hid[64];
  char app_hid[64];
  char user_hid[64];
  api_keys_t keys;
} flash_mem_t;

// restore the gateway information (hid)
int restore_gateway_info(arrow_gateway_t *gateway);

// save the gateway information (hid)
void save_gateway_info(const arrow_gateway_t *gateway);

// restore the device information (hid for all and eid for IBM account)
int restore_device_info(arrow_device_t *device);

// save the device information (hid for all and eid for IBM account)
void save_device_info(arrow_device_t *device);

// restore the wifi settings (SSID, password, secure mode)
int restore_wifi_setting(char *ssid, char *pass, int *sec);

// save the wifi settings (SSID, password, secure mode)
void save_wifi_setting(const char *ssid, const char *pass, int sec);

void save_key_setting(const char *api_key, const char *sec_key);
int restore_key_setting(char *api, char *sec);

#include <arrow/credentials.h>
void save_api_address(arrow_host_t *host);
int restore_api_address(arrow_host_t *host);
void save_mqtt_address(arrow_host_t *host);
int restore_mqtt_address(arrow_host_t *host);

void save_vhost(property_t vh);
int restore_vhost(property_t *vh);

void save_apphid_address(char *hid);
int restore_apphid_address(char *hid);

void save_userhid_address(char *hid);
int restore_userhid_address(char *hid);

#if defined (__cplusplus)
}
#endif

#endif
