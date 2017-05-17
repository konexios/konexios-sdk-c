/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/devicecommand.h"
#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>
#include <arrow/events.h>

static cmd_handler *__handlers = NULL;

static void __create_cmd_handler(cmd_handler *hd, const char *name, fp callback) {
  hd->name = malloc(strlen(name)+1);
  strcpy(hd->name, name);
  hd->callback = callback;
  hd->next = NULL;
}

// handlers
int has_cmd_handler() {
	if ( __handlers ) return 0;
	return -1;
}

int add_cmd_handler(const char *name, fp callback) {
  cmd_handler *h = malloc(sizeof(cmd_handler));
  __create_cmd_handler(h, name, callback);
  if ( !__handlers ) {
    __handlers = h;
  } else {
    cmd_handler *last = __handlers;
    while( last->next ) last = last->next;
    last->next = h;
  }
  return 0;
}

void free_cmd_handler() {
  cmd_handler *curr = __handlers;
  while( curr ) {
    cmd_handler *rm = curr;
    curr = curr->next;
    free(rm->name);
    free(rm);
  }
}


// events
static char *form_evetns_url(const char *hid, cmd_type ev) {
    char *uri = (char *)malloc(strlen(ARROW_API_EVENTS_ENDPOINT) + strlen(hid) + 15);
    strcpy(uri, ARROW_API_EVENTS_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, hid);
    switch(ev) {
        case failed:    strcat(uri, "/failed"); break;
        case received:  strcat(uri, "/received"); break;
        case succeeded: strcat(uri, "/succeeded"); break;
    }
    return uri;
}

int arrow_send_event_ans(const char *hid, cmd_type ev, const char *payload) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    
    char *uri = form_evetns_url(hid, ev);
    http_client_init( &cli );
    http_request_init(&request, PUT, uri);
    if ( payload ) {
        http_request_set_payload(&request, (char *)payload);
    }    
    sign_request(&request);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);
    
    http_client_free(&cli);
    free(uri);
    if ( response.m_httpResponseCode != 200 ) {
        http_response_free(&response);
        return -1;
    }
    http_response_free(&response);
    return 0;
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) __attribute__((used));
static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
  JsonNode *tmp = json_find_member(_node, name);
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = malloc(strlen(tmp->string_)+1);
  strcpy(*str, tmp->string_);
  return 0;
}

static fp find_cmd_handler(const char *cmd) {
  if ( __handlers ) {
    cmd_handler *h = __handlers;
    while(h) {
      if ( strcmp(h->name, cmd) == 0 ) return h->callback;
      h = h->next;
    }
  } else {
    DBG("No cmd handlers");
  }
  return NULL;
}

int command_handler(const char *name,
                    JsonNode *payload,
                    JsonNode **error) {
  int ret;
  fp callback = find_cmd_handler(name);
  if ( callback ) {
    ret = callback(payload->string_);
    if ( ret < 0 ) {
      *error = json_mkobject();
      json_append_member(*error, "error", json_mkstring("Something went wrong"));
    }
  } else {
    *error = json_mkobject();
    json_append_member(*error, "error", json_mkstring("there is no a command handler"));
  }
  return ret;
} __attribute__((weak))

int ev_DeviceCommand(void *_ev, JsonNode *_parameters) {
  int ret = -1;
  JsonNode *_error = NULL;
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  arrow_send_event_ans(ev->gateway_hid, received, NULL);
  DBG("start device command processing");

  JsonNode *tmp = json_find_member(_parameters, "deviceHid");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;

  JsonNode *cmd = json_find_member(_parameters, "command");
  if ( !cmd || cmd->tag != JSON_STRING ) return -1;
  DBG("ev cmd: %s", cmd->string_);

  JsonNode *pay = json_find_member(_parameters, "payload");
  if ( !pay || pay->tag != JSON_STRING ) return -1;
  DBG("ev msg: %s", pay->string_);

  ret = command_handler(cmd->string_, pay, &_error);

  if ( _error ) {
    arrow_send_event_ans(ev->gateway_hid, failed, json_encode(_error));
    json_delete(_error);
  } else {
    arrow_send_event_ans(ev->gateway_hid, succeeded, NULL);
  }

  return 0;
}
