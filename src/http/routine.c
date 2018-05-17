/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#define MODULE_NAME "HTTP_Request"

#include "http/routine.h"
#include <http/client.h>
#include <arrow/sign.h>
#include <debug.h>
#include <http/client_mqtt.h>

static http_client_t _cli;

http_client_t *current_client(void) {
  return &_cli;
}

int __http_init(void) {
    return http_client_init(&_cli);
}

typedef struct protocol_handler_ {
    int (*client_open) (http_client_t *cli, http_request_t *req);
    int (*client_close)(http_client_t *cli);
    int (*client_do)   (http_client_t *cli, http_response_t *res);
} protocol_handler_t;

protocol_handler_t client_protocols[] = {
    { http_client_open,
      http_client_close,
      http_client_do },
    { http_mqtt_client_open,
      http_mqtt_client_close,
      http_mqtt_client_do }
};

#define client_protocol_size (sizeof(client_protocols)/sizeof(protocol_handler_t))

int __http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc) {
  int ret = 0;
  http_request_t request;
  http_response_t response;
  if ( _cli.protocol > client_protocol_size ) {
      DBG("Unknown client protocol %d", _cli.protocol);
      return -2;
  }
  req_init(&request, arg_init);
  sign_request(&request);

  protocol_handler_t *ph = &client_protocols[_cli.protocol];
  if ( ph->client_open(&_cli, &request) < 0 ) return -1;
  ret = ph->client_do(&_cli, &response);
  http_request_close(&request);
  ph->client_close(&_cli);
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
  if ( ret < 0 ) http_session_close_now(&_cli);
  return ret;
}

int __http_done(void) {
    return http_client_free(&_cli);
}
