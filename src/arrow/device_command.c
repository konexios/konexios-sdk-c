/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/device_command.h"
#include <http/routine.h>
#include <json/json.h>
#include <sys/mem.h>
#include <arrow/events.h>
#include <debug.h>

static cmd_handler *__handlers = NULL;

// handlers
int has_cmd_handler(void) {
	if ( __handlers ) return 0;
	return -1;
}

int add_cmd_handler(const char *name, fp callback) {
  cmd_handler *h = malloc(sizeof(cmd_handler));
  h->name = strdup(name);
  h->callback = callback;
  linked_list_add_node_last(__handlers, cmd_handler, h);
  return 0;
}

void free_cmd_handler(void) {
  cmd_handler *curr = NULL;
  for_each_node_hard ( curr, __handlers , cmd_handler ) {
      free(curr->name);
      free(curr);
  }
}


// events
static char *form_evetns_url(const char *hid, cmd_type ev) {
    char *uri = (char *)malloc(sizeof(ARROW_API_EVENTS_ENDPOINT) + strlen(hid) + 15);
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

typedef struct _event_data {
	char *hid;
	cmd_type ev;
  char *payload; // FIXME property
} event_data_t;

static void _event_ans_init(http_request_t *request, void *arg) {
    event_data_t *data = (event_data_t *)arg;
	char *uri = form_evetns_url(data->hid, data->ev);
    http_request_init(request, PUT, uri);
	free(uri);
	if ( data->payload ) {
        http_request_set_payload(request, p_stack(data->payload));
	}
}

int arrow_send_event_ans(const char *hid, cmd_type ev, const char *payload) {
  event_data_t edata = {(char*)hid, ev, (char *)payload};
    STD_ROUTINE(_event_ans_init, &edata,
                NULL, NULL,
                "Arrow Event answer failed...");
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) __attribute__((used));
static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
  JsonNode *tmp = json_find_member(_node, name);
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = strdup(tmp->string_);
  return 0;
}

static int cmdeq( cmd_handler *s, const char *name ) {
    if ( strcmp(s->name, name) == 0 ) return 0;
    return -1;
}

static fp find_cmd_handler(const char *cmd) {
  if ( __handlers ) {
    cmd_handler *h = NULL;
    linked_list_find_node ( h, __handlers, cmd_handler, cmdeq, cmd );
    if ( h ) return h->callback;
  } else {
    DBG("No cmd handlers");
  }
  return NULL;
}

int __attribute__((weak)) command_handler(const char *name,
                    JsonNode *payload,
                    JsonNode **error) {
  int ret = -1;
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
}

int ev_DeviceCommand(void *_ev, JsonNode *_parameters) {
  int ret = -1;
  JsonNode *_error = NULL;
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  int retry = 0;
  http_session_close_set(current_client(), false);
  while( arrow_send_event_ans(ev->gateway_hid, received, NULL) < 0 ) {
      RETRY_UP(retry, {return -2;});
      msleep(ARROW_RETRY_DELAY);
  }
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
  if ( ret < 0 ) {
      DBG("command_handler fail %d", ret);
  }
  // close session after next request
  http_session_close_set(current_client(), true);

  if ( _error ) {
    while ( arrow_send_event_ans(ev->gateway_hid, failed, json_encode(_error)) < 0 ) {
        RETRY_UP(retry, {return -2;});
        msleep(ARROW_RETRY_DELAY);
    }
    json_delete(_error);
  } else {
    while ( arrow_send_event_ans(ev->gateway_hid, succeeded, NULL) < 0 ) {
        RETRY_UP(retry, {return -2;});
        msleep(ARROW_RETRY_DELAY);
    }
  }

  return 0;
}
