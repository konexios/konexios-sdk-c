/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/events.h"

#if !defined(NO_EVENTS)

#include <arrow/device_command.h>
#include <arrow/state.h>

#if !defined(NO_RELEASE_UPDATE)
#include <arrow/software_release.h>
#endif

#if !defined(NO_SOFTWARE_UPDATE)
# include <arrow/software_update.h>
#endif

#if defined(DEVICE_STARTSTOP)
#include <arrow/device_startstop.h>
#endif

#include <ctype.h>
#include <debug.h>
#include <http/client.h>
#include <json/json.h>
#include <sys/mem.h>
#include <arrow/gateway_payload_sign.h>

#if defined(STATIC_MQTT_ENV)
#include <data/static_alloc.h>
#endif

#if defined(ARROW_THREAD)
#include <sys/mutex.h>
#define MQTT_EVENTS_QUEUE_LOCK      arrow_mutex_lock(_event_mutex)
#define MQTT_EVENTS_QUEUE_UNLOCK    arrow_mutex_unlock(_event_mutex)
#else
#define MQTT_EVENTS_QUEUE_LOCK
#define MQTT_EVENTS_QUEUE_UNLOCK
#endif

static void free_mqtt_event(mqtt_event_t *mq) {
  if ( mq->gateway_hid ) free(mq->gateway_hid);
  if ( mq->device_hid ) free(mq->device_hid);
  if ( mq->cmd ) free(mq->cmd);
  if ( mq->name ) free(mq->name);
  if ( mq->parameters ) json_delete(mq->parameters);
}

static int fill_string_from_json(JsonNode *_node, const char *name, char **str) {
    // FIXME property
  JsonNode *tmp = json_find_member(_node, p_stack(name));
  if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
  *str = strdup(tmp->string_);
  return 0;
}

typedef int (*submodule)(void *, JsonNode *);
typedef void (*module_init)();
typedef void (*module_deinit)();
typedef struct {
  char *name;
  submodule proc;
  module_init init;
  module_deinit deinit;
} sub_t;

sub_t sub_list[] = {
  { "ServerToGateway_DeviceCommand", ev_DeviceCommand, NULL, arrow_command_handler_free },
  { "SendCommand", ev_DeviceCommand, NULL, NULL },
  { "ServerToGateway_DeviceStateRequest", ev_DeviceStateRequest, NULL, NULL },
#if !defined(NO_SOFTWARE_UPDATE)
  { "ServerToGateway_GatewaySoftwareUpdate", ev_GatewaySoftwareUpdate, NULL, NULL },
#endif
#if !defined(NO_RELEASE_UPDATE)
  { "ServerToGateway_DeviceSoftwareRelease", ev_DeviceSoftwareRelease, NULL, NULL },
  { "ServerToGateway_GatewaySoftwareRelease", ev_DeviceSoftwareRelease, NULL, NULL },
#endif
#if defined(DEVICE_STARTSTOP)
    { "ServerToGateway_DeviceStart", ev_DeviceSoftwareRelease, NULL, NULL },
    { "ServerToGateway_DeviceStop", ev_DeviceSoftwareRelease, NULL, NULL },
#endif
    { NULL, NULL, NULL, NULL }
};

// checker

typedef int(*sign_checker)(const char *, mqtt_event_t *, const char *);
struct check_signature_t {
  const char *version;
  sign_checker check;
};

static int check_sign_1(const char *sign, mqtt_event_t *ev, const char *can) {
  char signature[65] = {0};
  int err = gateway_payload_sign(signature,
                                 ev->gateway_hid,
                                 ev->name,
                                 ev->encrypted,
                                 can,
                                 "1");
  if ( err ) return -1;
  DBG("cmp { %s, %s }", sign, signature);
  return ( strcmp(sign, signature) == 0 ? 0 : -1 );
}

static struct check_signature_t checker_collection[] = {
  {"1", check_sign_1},
};

static int check_signature(const char *vers, const char *sing, mqtt_event_t *ev, const char *canParamStr) {
  unsigned int i = 0;
  for ( i = 0; i< sizeof(checker_collection) / sizeof(struct check_signature_t); i++ ) {
    if ( strcmp(vers, checker_collection[i].version ) == 0 ) {
      DBG("check version %s", checker_collection[i].version);
      return checker_collection[i].check(sing, ev, canParamStr);
    }
  }
  return -1;
}

#if defined(STATIC_MQTT_ENV)
static char static_canonical_prm[MQTT_RECVBUF_LEN];
static_object_pool_type(mqtt_event_t, ARROW_MAX_MQTT_COMMANDS)
#else
static int cmpstringp(const void *p1, const void *p2) {
  return strcmp(* (char * const *) p1, * (char * const *) p2);
}
#endif

typedef struct _str_t {
  char *start;
  int len;
} str_t;

int less(str_t *s1, str_t *s2) {
    int _min = (s1->len < s2->len ? s1->len : s2->len);
    int i = 0;
    for ( i = 0; i<_min; i++) {
        if ( s1->start[i] == s2->start[i] ) continue;
        if ( s1->start[i] < s2->start[i] ) {
            return 1;
        } else return -1;
    }
    return 0;
}

void swap(str_t *s1, str_t *s2) {
    char saved;
    int start_pos = s1->len-1;
    int current_pos = start_pos;
    int i = 0;
    start_pos = 0;
    while (i < s1->len + s2->len ) {
        current_pos = start_pos;
        saved = s1->start[current_pos];
        do {
            int next_pos = 0;
            if ( current_pos < s1->len ) next_pos = s2->len + current_pos;
            else next_pos = current_pos - s1->len;
            char t = s1->start[next_pos];
            //printf("move %d [%c] -> %d [%c]\r\n", current_pos, saved, next_pos, buffer[next_pos]);
            s1->start[next_pos] = saved;
            saved = t;
            i++;
            current_pos = next_pos;
        } while ( start_pos != current_pos );
        start_pos++;
        //  printf("\t %d [%s]\r\n", i+1, buffer);
    }
    int size1 = s1->len;
    s1->len = s2->len;
    s2->len = size1;
    s2->start = s1->start + s1->len;
}

int booble(str_t *s, int len) {
    int do_sort = 1;
    while(do_sort) {
        do_sort = 0;
        int i = 0;
        for ( i = 0; i<len-1; i++ ) {
            if ( less (&s[i], &s[i+1]) == -1 ) {
                swap(&s[i], &s[i+1]);
                do_sort = 1;
            }
        }
    }
    return 0;
}

static char *form_canonical_prm(JsonNode *param) {
  JsonNode *child;
  char *canParam = static_canonical_prm;
  str_t can_list[MAX_PARAM_LINE];
  int total = 0;
  int count = 0;
  json_foreach(child, param) {
      can_list[count].start = canParam + total;
      int i;
      int key_len = strlen(json_key(child));
      for ( i=0; i < key_len; i++ )
          *(can_list[count].start+i) = tolower((int)json_key(child)[i]);
      *(can_list[count].start+i) = '=';
      can_list[count].len = key_len + 1;

      int r = 0;
      switch(child->tag) {
      case JSON_STRING:
          r = snprintf(can_list[count].start+can_list[count].len,
                       sizeof(static_canonical_prm) - total,
                       "%s",
                       child->string_);
          break;
      case JSON_BOOL:
          r = snprintf(can_list[count].start+can_list[count].len,
                       sizeof(static_canonical_prm) - total,
                       "%s",
                       (child->bool_?"true":"false"));
          break;
      default:
          r = snprintf(can_list[count].start+can_list[count].len,
                       16,
                       "%f",
                       child->number_);
      }

      can_list[count].len += r;
      can_list[count].start[can_list[count].len] = '\n';
      can_list[count].len++;
      total += can_list[count].len;
      count++;
  }
  can_list[count-1].start[can_list[count-1].len-1] = '\0';
  can_list[count-1].len--;
  total--;
  booble(can_list, count);
  return canParam;
}

#if 0
static __attribute_used__ char *form_canonical_prm2(JsonNode *param) {
  JsonNode *child;
  char *canParam = NULL;
  char *can_list[MAX_PARAM_LINE] = {0};
  int total_len = 0;
  int count = 0;
  json_foreach(child, param) {
    int alloc_len = child->tag==JSON_STRING?strlen(child->string_):50;
    alloc_len += strlen(json_key(child));
    alloc_len += 10;
    can_list[count] = (char*)malloc( alloc_len );
    if ( !can_list[count] ) {
        DBG("GATEWAY SIGN: not enough memory");
        goto can_list_error;
    }
    total_len += alloc_len;
    unsigned int i;
    for ( i=0; i<strlen(json_key(child)); i++ ) *(can_list[count]+i) = tolower((int)json_key(child)[i]);
    *(can_list[count]+i) = '=';
    switch(child->tag) {
      case JSON_STRING: strcpy(can_list[count]+i+1, child->string_);
        break;
      case JSON_BOOL: strcpy(can_list[count]+i+1, (child->bool_?"true\0":"false\0"));
        break;
      default: {
        int r = snprintf(can_list[count]+i+1, 50, "%f", child->number_);
        *(can_list[count]+i+1 + r ) = 0x0;
      }
    }
    count++;
  }
#if defined(STATIC_MQTT_ENV)
  if ( (size_t)total_len > sizeof(static_canonical_prm) ) goto can_list_error;
  canParam = static_canonical_prm;
  DBG("GATEWAY SIGN: static mem %d", total_len);
#else
  canParam = (char*)malloc(total_len);
  DBG("GATEWAY SIGN: alloc memory %d", total_len);
#endif
  if ( !canParam ) {
      DBG("GATEWAY SIGN: not enough memory %d", total_len);
      goto can_list_error;
  }
  *canParam = 0;
  qsort(can_list, count, sizeof(char *), cmpstringp);
  int i = 0;
  for (i=0; i<count; i++) {
    strcat(canParam, can_list[i]);
    if ( i < count-1 ) strcat(canParam, "\n");
    free(can_list[i]);
  }
  return canParam;
can_list_error:
  for (i=0; i < count; i++) {
    free(can_list[i]);
  }
  return NULL;
}
#endif



static mqtt_event_t *__event_queue = NULL;
#if defined(ARROW_THREAD)
static arrow_mutex *_event_mutex = NULL;
#endif

void arrow_mqtt_events_init(void) {
#if defined(ARROW_THREAD)
    arrow_mutex_init(&_event_mutex);
#endif
    int i = 0;
    for (i=0; i < (int)(sizeof(sub_list)/sizeof(sub_t)); i++) {
      if ( sub_list[i].init ) sub_list[i].init();
    }
}

int arrow_mqtt_has_events(void) {
    int ret = -1;
    MQTT_EVENTS_QUEUE_LOCK;
    if ( !__event_queue ) return 0;
    ret = 0;
    mqtt_event_t *tmp = NULL;
    arrow_linked_list_for_each(tmp, __event_queue, mqtt_event_t) {
        ret++;
    }
    MQTT_EVENTS_QUEUE_UNLOCK;
    return ret;
}

int arrow_mqtt_event_proc(void) {
    mqtt_event_t *tmp = NULL;
    MQTT_EVENTS_QUEUE_LOCK;
    tmp = __event_queue;
    if ( !tmp ) {
        MQTT_EVENTS_QUEUE_UNLOCK;
        return -1;
    }
    arrow_linked_list_del_node_first(__event_queue, mqtt_event_t);
    MQTT_EVENTS_QUEUE_UNLOCK;

    submodule current_processor = NULL;
    int i = 0;
    for (i=0; i < (int)(sizeof(sub_list)/sizeof(sub_t)); i++) {
      if ( sub_list[i].name && strcmp(sub_list[i].name, tmp->name) == 0 ) {
        current_processor = sub_list[i].proc;
      }
    }
    int ret = -1;
    if ( current_processor ) {
      ret = current_processor(tmp, tmp->parameters);
    } else {
      DBG("No event processor for %s", tmp->name);
      goto mqtt_event_proc_error;
    }
    if ( __event_queue ) ret = 1;
mqtt_event_proc_error:
    free_mqtt_event(tmp);
#if defined(STATIC_MQTT_ENV)
    static_free(mqtt_event_t, tmp);
#else
    free(tmp);
#endif
    return ret;
}

int process_event(const char *str) {

#if defined(ARROW_MAX_MQTT_COMMANDS)
    if (arrow_mqtt_has_events() >= ARROW_MAX_MQTT_COMMANDS)
        return -1;
#endif

#if defined(STATIC_MQTT_ENV)
    mqtt_event_t *mqtt_e = static_allocator(mqtt_event_t);
#else
  mqtt_event_t *mqtt_e = (mqtt_event_t *)calloc(1, sizeof(mqtt_event_t));
#endif
  if ( !mqtt_e ) {
      DBG("PROCESS EVENT: not enough memory");
      return -2;
  }
  int ret = -1;
  JsonNode *_main = json_decode(str);
  if ( !_main ) {
      DBG("event payload decode failed %d", strlen(str));
      return -1;
  }

  if ( fill_string_from_json(_main, "hid", &mqtt_e->gateway_hid) < 0 ) {
    DBG("cannot find HID");
    goto error;
  }
#if defined(DEBUG_MQTT_PROCESS_EVENT)
  DBG("ev ghid: %s", mqtt_e->gateway_hid);
#endif

  if ( fill_string_from_json(_main, "name", &mqtt_e->name) < 0 ) {
    DBG("cannot find name");
    goto error;
  }
#if defined(DEBUG_MQTT_PROCESS_EVENT)
  DBG("ev name: %s", mqtt_e->name);
#endif

  JsonNode *_encrypted = json_find_member(_main, p_const("encrypted"));
  if ( !_encrypted ) goto error;
  mqtt_e->encrypted = _encrypted->bool_;

  JsonNode *_parameters = json_find_member(_main, p_const("parameters"));
  if ( !_parameters ) goto error;
  JsonNode *sign_version = json_find_member(_main, p_const("signatureVersion"));
  if ( sign_version ) {
#if defined(DEBUG_MQTT_PROCESS_EVENT)
      DBG("signature vertsion: %s", sign_version->string_);
#endif
    JsonNode *sign = json_find_member(_main, p_const("signature"));
    if ( !sign ) goto error;
    char *can = form_canonical_prm(_parameters);
#if defined(DEBUG_MQTT_PROCESS_EVENT)
    DBG("[%s]", can);
#endif
    if ( !can ) goto error;
    if ( check_signature(sign_version->string_, sign->string_, mqtt_e, can) < 0 ) {
      DBG("Alarm! signature is failed...");
#if !defined(STATIC_MQTT_ENV)
      free(can);
#endif
      goto error;
    }
#if !defined(STATIC_MQTT_ENV)
    free(can);
#endif
  }
  json_remove_from_parent(_parameters);
  mqtt_e->parameters = _parameters;
  ret = 0;
  arrow_linked_list_add_node_last(__event_queue, mqtt_event_t, mqtt_e);

error:
  if ( ret < 0 ) {
#if defined(STATIC_MQTT_ENV)
      static_free(mqtt_event_t, mqtt_e);
#else
      free(mqtt_e);
#endif
  }
  if ( _main ) json_delete(_main);
  return ret;
}

void arrow_mqtt_events_done() {
#if defined(ARROW_THREAD)
    arrow_mutex_deinit(_event_mutex);
#endif
    int i = 0;
    for (i=0; i < (int)(sizeof(sub_list)/sizeof(sub_t)); i++) {
      if ( sub_list[i].deinit ) sub_list[i].deinit();
    }
}
#else
typedef void __dummy;
#endif
