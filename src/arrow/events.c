/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/events.h"
#include <arrow/devicecommand.h>
#include <arrow/state.h>

#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>

void free_mqtt_event(mqtt_event_t *mq) {
  if ( mq->gateway_hid ) free(mq->gateway_hid);
  if ( mq->device_hid ) free(mq->device_hid);
  if ( mq->cmd ) free(mq->cmd);
  if ( mq->payload ) free(mq->payload);
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
  JsonNode *tmp = json_find_member(_node, name);
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = malloc(strlen(tmp->string_)+1);
  strcpy(*str, tmp->string_);
  return 0;
}

typedef int (*submodule)(mqtt_event_t *, JsonNode *);
typedef struct {
  char *name;
  submodule proc;
} sub_t;

sub_t sub_list[] = {
  { "ServerToGateway_DeviceCommand", ev_DeviceCommand},
  { "ServerToGateway_DeviceStateRequest", ev_DeviceStateRequest }
};


int process_event(const char *str) {
  DBG("ev: %s", str);
  mqtt_event_t mqtt_e;
  memset(&mqtt_e, 0x0, sizeof(mqtt_event_t));
  JsonNode *_main = json_decode(str);
  if ( !_main ) {
      DBG("event payload decode failed %d", strlen(str));
      return -1;
  }

  if ( fill_string_from_json(_main, "hid", &mqtt_e.gateway_hid) < 0 ) {
    DBG("cannot find HID");
    goto error;
  }
  DBG("ev ghid: %s", mqtt_e.gateway_hid);

  JsonNode *event_name = json_find_member(_main, "name");
  if ( !event_name || event_name->tag != JSON_STRING ) {
    DBG("cannot find name");
    goto error;
  }
  DBG("ev name: %s", event_name->string_);

  JsonNode *_parameters = json_find_member(_main, "parameters");
  if ( !_parameters ) goto error;

  int ret = -1;
  submodule current_processor = NULL;
  for (int i=0; i< sizeof(sub_list)/sizeof(sub_t); i++) {
    if ( strcmp(sub_list[i].name, event_name->string_) == 0 ) {
      current_processor = sub_list[i].proc;
    }
  }

  if ( current_processor ) {
    ret = current_processor(&mqtt_e, _parameters);
  } else {
    DBG("No event processor for %s", event_name->string_);
    goto error;
  }

error:
  free_mqtt_event(&mqtt_e);
  if ( _main ) json_delete(_main);
  return ret;
}
