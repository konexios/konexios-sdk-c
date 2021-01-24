/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "http/routine.h"
#include <http/client.h>
#include <konexios/sign.h>
#include <debug.h>
#include <http/client_mqtt.h>

static http_client_t _client;

http_client_t *current_client(void) {
  return &_client;
}

int http_init(void) {
    return http_client_init(&_client);
}

void http_session_keep_active(bool active)
{
    _client.flags.close_socket = (!active);
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
#if defined(HTTP_VIA_MQTT)
    { http_mqtt_client_open,
      http_mqtt_client_close,
      http_mqtt_client_do }
#endif
};

#define client_protocol_size (sizeof(client_protocols)/sizeof(protocol_handler_t))


void http_set_recv_timeout_ms(int ms)
{
    _client.timeout = ms;
}

int http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc) {
  int ret = 0;
  http_request_t request;
  http_response_t response;
  DBG("client protocol: %u", (unsigned int)_client.protocol);
  if ( _client.protocol > client_protocol_size ) {
      DBG("Unknown client protocol %u", (unsigned int)_client.protocol);
      return -2;
  }
  // Call the init callback
  req_init(&request, arg_init);
  sign_request(&request);
  http_response_init(&response, &request._response_payload_meth);

  protocol_handler_t *ph = &client_protocols[_client.protocol];
  if ( (ret = ph->client_open(&_client, &request)) >= 0 ) {
      ret = ph->client_do(&_client, &response);
  } else {
      DBG("client open error %d", ret);
  }
  http_request_close(&request);
  // End the session
  ph->client_close(&_client);

  // Handle error
  if ( ret < 0 ) {
      DBG("client error %d", ret);
      goto http_error;
  }

  // call the response callback
  if ( resp_proc ) {
    ret = resp_proc(&response, arg_proc);
  } else {
    if ( response.m_httpResponseCode != 200 ) {
      ret = -1;
      DBG("client response %d", response.m_httpResponseCode);
      goto http_error;
    }
  }
http_error:
  http_response_free(&response);
  if ( http_session_is_open(&_client) && ret < 0 ) {
      http_session_close_set(&_client, true);
      ph->client_close(&_client);
      http_session_set_protocol(&_client, api_via_http);
  }
  return ret;
}

int http_last_response_code()
{

    printf("Ret code %d\n",_client.response_code);
    //if(_client==NULL) return -1;
    return _client.response_code;
}

int http_end(void)
{
    // End the socket
    http_session_keep_active(false);
    http_client_close(&_client);
    return 0;
}

int http_done(void) {
    return http_client_free(&_client);
}
