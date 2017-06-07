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
#include <arrow/storage.h>
#include <arrow/mem.h>
#include <arrow/device_api.h>
#include <arrow/gateway_api.h>

static int __close_session = 1;
static int __newsession = 1;

void dont_close_session(void) {
	__close_session = 0;
}

void do_close_session(void) {
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
    DBG("gateway checkin hid %s", P_VALUE(gateway->hid));
    return arrow_gateway_checkin(gateway);
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
