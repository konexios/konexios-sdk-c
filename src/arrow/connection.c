/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/connection.h"
#include <config.h>
#include <debug.h>
#include <http/client.h>
#include <json/json.h>
#include <json/telemetry.h>
#include <arrow/net.h>
#include <arrow/storage.h>
#include <arrow/mem.h>
#include <arrow/device_api.h>
#include <arrow/gateway_api.h>

static int __close_session = 1;
static int __newsession = 1;

void dont_close_session() {
	__close_session = 0;
}

void do_close_session() {
	__close_session = 1;
}

static http_client_t _cli;

int __http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc) {
  int ret = 0;
  http_request_t request;
  http_response_t response;
  http_client_init(&_cli, __newsession);
  req_init(&request, arg_init);
  sign_request(&request);
  ret = http_client_do(&_cli, &request, &response);
  http_request_close(&request);
  if (__close_session) {
	  http_client_free(&_cli);
	  __newsession = 1;
  } else {
	  __newsession = 0;
  }
  if ( ret < 0 ) goto http_error;
  if ( resp_proc ) {
    ret = resp_proc(&response, arg_proc);
  } else {
    if ( response.m_httpResponseCode != 200 ) {
    	ret = -1;
    	goto http_error;
    }
  }
http_error:
  http_response_free(&response);
  return ret;
}

int arrow_prepare_gateway(arrow_gateway_t *gateway) {
  arrow_gateway_init(gateway);
  arrow_gateway_add_name(gateway, GATEWAY_NAME);
  arrow_gateway_add_os(gateway, GATEWAY_OS);
  arrow_gateway_add_software_name(gateway, GATEWAY_SOFTWARE_NAME);
  arrow_gateway_add_software_version(gateway, GATEWAY_SOFTWARE_VERSION);
  arrow_gateway_add_type(gateway, GATEWAY_TYPE);
  arrow_gateway_add_sdkVersion(gateway, xstr(SDK_VERSION));
  char *uid = (char*)malloc(strlen(GATEWAY_UID_PREFIX) + 20);
  strcpy(uid, GATEWAY_UID_PREFIX);
  strcat(uid, "-");
  uint32_t uidlen = strlen(uid);
  char mac[7];
  get_mac_address(mac);
  int i;
  for(i=0; i<6; i++) sprintf(uid+uidlen+2*i, "%02x", (uint8_t)(mac[i]));
  uidlen += 12;
  uid[uidlen] = '\0';
  DBG("uid: [%s]", uid);
  arrow_gateway_add_uid(gateway, uid);
  free(uid);
  return 0;
}

int arrow_prepare_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_device_init(device);
  arrow_device_add_gateway_hid(device, gateway->hid);
  arrow_device_add_name(device, DEVICE_NAME);
  arrow_device_add_type(device, DEVICE_TYPE);
//    FIXME info property extra param?
//    arrow_device_add_info(device, "info1", "value1");
//    arrow_device_add_info(device, "info2", "value2");
//    arrow_device_add_property(device, "prop1", "value1");
//    arrow_device_add_property(device, "prop2", "value2");
  if ( !gateway->uid ) return -1;
  char *uid = (char*)malloc(strlen(gateway->uid)+strlen(DEVICE_UID_SUFFIX)+2);
  strcpy(uid, gateway->uid);
  strcat(uid, "-");
  strcat(uid, DEVICE_UID_SUFFIX);
  arrow_device_add_uid(device, uid);
  free(uid);
  return 0;
}

static void _gateway_heartbeat_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  char *uri = (char *)malloc(sizeof(ARROW_API_GATEWAY_ENDPOINT) + 50);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, gateway->hid);
  strcat(uri, "/heartbeat");
  http_request_init(request, PUT, uri);
  free(uri);
}

int arrow_heartbeat(arrow_gateway_t *gateway) {
  int ret = 0;
  ret = __http_routine(_gateway_heartbeat_init, gateway, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway heartbeat failed...");
  }
  return ret;
}

static void _gateway_checkin_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  char *uri = (char *)malloc(sizeof(ARROW_API_GATEWAY_ENDPOINT) + strlen(gateway->hid) + 20);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, gateway->hid);
  strcat(uri, "/checkin");
  http_request_init(request, PUT, uri);
  free(uri);
}

int arrow_checkin(arrow_gateway_t *gateway) {
  int ret = 0;
  ret = __http_routine(_gateway_checkin_init, gateway, NULL, NULL);
  DBG("routine done");
  if ( ret < 0 ) {
    DBG("Arrow Gateway checkin failed...");
  }
  return ret;
}

int arrow_connect_gateway(arrow_gateway_t *gateway){
  arrow_prepare_gateway(gateway);
  int ret = restore_gateway_info(gateway);
  if ( ret < 0 ) {
    // new registration
    if ( arrow_register_gateway(gateway) < 0 ) {
      return -1;
    }
    save_gateway_info(gateway);
  } else {
    // hid already set
    DBG("gateway checkin hid %s", gateway->hid);
    return arrow_checkin(gateway);
  }
  return 0;
}

int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_prepare_device(gateway, device);
  if ( restore_device_info(device) < 0 ) {
    if ( arrow_register_device(gateway, device) < 0 ) return -1;
    save_device_info(device);
  }
  return 0;
}
