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
#include <data/ringbuffer.h>
#include <debug.h>

static http_client_t _cli;

http_client_t *current_client(void) {
  return &_cli;
}

int __http_init(void) {
    return http_client_init(&_cli);
}

#include <json/json.h>
#include <arrow/gateway.h>

static int headbyname( property_map_t *h, const char *name ) {
    if ( strcmp(P_VALUE(h->key), name) == 0 ) return 0;
    return -1;
}

int mqtt_client_do(http_client_t *cli, http_response_t *res) {
    int ret = 0;
    http_request_t *req = cli->request;
    if ( !req ) return -1;
    http_response_init(res, &req->_response_payload_meth);

    JsonNode *_node = json_mkobject();
    json_append_member(_node,
                       p_const("requestId"),
                       json_mkstring("ABC-123"));
    json_append_member(_node,
                       p_const("eventName"),
                       json_mkstring("GatewayToServer_ApiRequest"));
    json_append_member(_node,
                       p_const("encrypted"),
                       json_mkbool(false));
    //add parameter
    JsonNode *_parameters = json_mkobject();
    json_append_member(_parameters,
                       p_const("uri"),
                       json_mkstring(P_VALUE(req->uri)));
    json_append_member(_parameters,
                       p_const("method"),
                       json_mkstring(P_VALUE(req->meth)));
    json_append_member(_parameters,
                       p_const("apiKey"),
                       json_mkstring(get_api_key()));
    json_append_member(_parameters,
                       p_const("body"),
                       json_mkstring(P_VALUE(req->payload)));
    property_map_t *tmp = NULL;
    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-signature");
    if ( tmp ) {
        json_append_member(_parameters,
                           p_const("apiRequestSignature"),
                           json_mkstring(P_VALUE(tmp->value)));
    }

    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-version");
    if ( tmp ) {
        json_append_member(_parameters,
                           p_const("apiRequestSignature"),
                           json_mkstring(P_VALUE(tmp->value)));
    }

    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-date");
    if ( tmp ) {
        json_append_member(_parameters,
                           p_const("timestamp"),
                           json_mkstring(P_VALUE(tmp->value)));
    }
    char sig[65] = {0};
    // FIXME bad function externing
    arrow_gateway_t *current_gateway(void);
    extern int as_event_sign(char *signature,
                             property_t ghid,
                             const char *name,
                             int encrypted,
                             JsonNode *_parameters);
    as_event_sign(sig,
                  current_gateway()->hid,
                  "GatewayToServer_ApiRequest",
                  0,
                  _parameters );
    json_append_member(_node,
                       p_const("signature"),
                       json_mkstring(sig));
    json_append_member(_node,
                       p_const("signatureVersion"),
                       json_mkstring("1"));
    json_delete(_node);
    return ret;
}

int __http_routine(response_init_f req_init, void *arg_init,
                   response_proc_f resp_proc, void *arg_proc) {
  int ret = 0;
  http_request_t request;
  http_response_t response;
  req_init(&request, arg_init);
  sign_request(&request);
  if ( http_client_open(&_cli, &request) < 0 ) return -1;
  if ( _cli.via_mqtt ) {
      ret = mqtt_client_do(&_cli, &response);
  } else {
      ret = http_client_do(&_cli, &response);
  }
  http_request_close(&request);
  http_client_close(&_cli);
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
  _cli.via_mqtt = 0; // FIXME workaround end
  if ( ret < 0 ) http_session_close_now(&_cli);
  return ret;
}

int __http_done(void) {
    return http_client_free(&_cli);
}
