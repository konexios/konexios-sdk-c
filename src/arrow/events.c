/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/events.h"
#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>

static cmd_handler *__handlers = NULL;

static void __create_cmd_handler(cmd_handler *hd, const char *name, fp callback) {
  hd->name = malloc(strlen(name)+1);
  strcpy(hd->name, name);
  hd->callback = callback;
  hd->next = NULL;
}

// handlers
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
  // FIXME impl if it is needed
}


// events
static char *form_evetns_url(const char *hid, event_t ev) {
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

int arrow_send_event_ans(const char *hid, event_t ev, const char *payload) {
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

typedef struct {
  char *gateway_hid;
  char *device_hid;
  char *cmd;
  char *payload;
} mqtt_event_t;

void free_mqtt_event(mqtt_event_t *mq) {
  if ( mq->gateway_hid ) free(mq->gateway_hid);
  if ( mq->device_hid ) free(mq->device_hid);
  if ( mq->cmd ) free(mq->cmd);
  if ( mq->payload ) free(mq->payload);
}

int arrow_event_parse(const char *str, mqtt_event_t *mq_ev) {
  if (!str) return -1;
  JsonNode *_main = json_decode(str);
  if ( !_main ) {
      DBG("event payload decode failed %d", strlen(str));
      return -1;
  }
  JsonNode *hid = json_find_member(_main, "hid");
  if ( !hid ) {
      DBG("cannot find HID");
      return -1;
  }
  if ( hid->tag != JSON_STRING ) return -1;
  DBG("ev ghid: %s", hid->string_);
  mq_ev->gateway_hid = malloc(strlen(hid->string_)+1);
  strcpy(mq_ev->gateway_hid, hid->string_);
  JsonNode *_parameters = json_find_member(_main, "parameters");
  if ( !_parameters ) return -1;
  hid = json_find_member(_parameters, "deviceHid");
  if ( !hid || hid->tag != JSON_STRING ) return -1;
  mq_ev->device_hid = malloc(strlen(hid->string_)+1);
  strcpy(mq_ev->device_hid, hid->string_);
  
  hid = json_find_member(_parameters, "command");
  if ( !hid || hid->tag != JSON_STRING ) return -1;
  DBG("ev cmd: %s", hid->string_);
  mq_ev->cmd = malloc(strlen(hid->string_)+1);
  strcpy(mq_ev->cmd, hid->string_);
  
  hid = json_find_member(_parameters, "payload");
  if ( !hid || hid->tag != JSON_STRING ) return -1;
  DBG("ev msg: %s", hid->string_);
  mq_ev->payload = malloc(strlen(hid->string_)+1);
  strcpy(mq_ev->payload, hid->string_);
  
  json_delete(_main);
  return 0;
}

fp find_cmd_handler(char *cmd) {
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

int process_event(const char *str) {
  DBG("ev: %s", str);
  mqtt_event_t mqtt_e;
  JsonNode *_error = NULL;
  
  int ret = -1;
  if ( arrow_event_parse(str, &mqtt_e) >= 0 ) {
    DBG("gateway hid %s", mqtt_e.gateway_hid);
    DBG("device  hid %s", mqtt_e.device_hid);
    arrow_send_event_ans(mqtt_e.gateway_hid, received, NULL);
    fp callback = find_cmd_handler(mqtt_e.cmd);
    if ( callback ) {
      ret = callback(mqtt_e.payload);
    } else {
      _error = json_mkobject();
      json_append_member(_error, "error", json_mkstring("there is no a command handler"));
    }
  } else {
      // json is broken
      return -1;
  }
  if ( ret < 0 ) {
    if ( !_error ) {
      // default
      _error = json_mkobject();
      json_append_member(_error, "error", json_mkstring("unknown"));
    }
    arrow_send_event_ans(mqtt_e.gateway_hid, failed, json_encode(_error));
  }
  else arrow_send_event_ans(mqtt_e.gateway_hid, succeeded, NULL);
  free_mqtt_event(&mqtt_e);
  if ( _error ) json_delete(_error);
  return 0;
}
