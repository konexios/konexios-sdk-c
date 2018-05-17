#include "arrow/state.h"
#include <time/time.h>
#include <http/client.h>
#include <json/json.h>
#include <sys/mem.h>
#include <http/routine.h>
#include <arrow/events.h>
#include <debug.h>
#include <data/chunk.h>

static JsonNode *state_tree = NULL;
static property_t _device_hid = {0};

int state_handler(char *str) __attribute__((weak));

void add_state(const char *name, const char *value) {
  if ( !state_tree ) state_tree = json_mkobject();
  // FIXME should be propery
  json_append_member(state_tree, p_stack(name), json_mkstring(value));
}

int arrow_state_mqtt_is_running(void) {
  if ( !state_tree ) return -1;
  return 0;
}

int arrow_state_mqtt_stop(void) {
  if (state_tree) json_delete(state_tree);
  property_free(&_device_hid);
  return 0;
}

int arrow_state_mqtt_run(arrow_device_t *device) {
  if ( !IS_EMPTY(_device_hid) ) {
    property_weak_copy(&_device_hid, device->hid);
  }
  return arrow_state_mqtt_is_running();
}

static void _state_get_init(http_request_t *request, void *arg) {
  arrow_device_t *device = (arrow_device_t *)arg;
  CREATE_CHUNK(uri, sizeof(ARROW_API_DEVICE_ENDPOINT) + P_SIZE(device->hid) + 10);
  strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(device->hid));
  strcat(uri, "/state");
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

int arrow_get_state(arrow_device_t *device) {
  STD_ROUTINE(_state_get_init, device, NULL, NULL, "State get failed...");
}

typedef enum {
  st_request,
  st_update
} _st_post_api;

typedef struct _post_dev_ {
  arrow_device_t *device;
  int post;
} post_dev_t;

static void _state_post_init(http_request_t *request, void *arg) {
  post_dev_t *pd = (post_dev_t *)arg;
  JsonNode *_state = NULL;
  CREATE_CHUNK(uri, sizeof(ARROW_API_DEVICE_ENDPOINT) + P_SIZE(pd->device->hid) + 20);
  strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(pd->device->hid));
  strcat(uri, "/state/");
  switch( pd->post ) {
    case st_request:
      strcat(uri, "request");
    break;
    case st_update:
      strcat(uri, "update");
    break;
    default:
      FREE_CHUNK(uri);
      return;
  }
  FREE_CHUNK(uri);
  http_request_init(request, POST, uri);
  {
    _state = json_mkobject();
    json_append_member(_state, p_const("states"), state_tree);
    char ts[30];
    get_time(ts);
    json_append_member(_state, p_const("timestamp"), json_mkstring(ts));
  }
  http_request_set_payload(request, json_encode_property(_state));
  if (_state) {
    json_remove_from(_state, state_tree);
    json_delete(_state);
  }
}

static int _arrow_post_state(arrow_device_t *device, _st_post_api post_type) {
  post_dev_t pd = {device, post_type};
  STD_ROUTINE(_state_post_init, &pd,
              NULL, NULL,
              "State post failed...");
}

int arrow_post_state_request(arrow_device_t *device) {
  return _arrow_post_state(device, st_request);
}

int arrow_post_state_update(arrow_device_t *device) {
  return _arrow_post_state(device, st_update);
}



typedef enum {
  st_received,
  st_complete,
  st_error
} _st_put_api;

typedef struct _put_dev_ {
  property_t device_hid;
  const char *trans_hid;
  int put_type;
} put_dev_t;

int state_handler(char *str) {
  SSP_PARAMETER_NOT_USED(str);
  DBG("weak state handler [%s]", str);
  return 0;
} // __attribute__((weak))

#if !defined(NO_EVENTS)
static void _state_put_init(http_request_t *request, void *arg) {
  put_dev_t *pd = (put_dev_t *)arg;
  JsonNode *_error = NULL;
  CREATE_CHUNK(uri, sizeof(ARROW_API_DEVICE_ENDPOINT) +
               property_size(&pd->device_hid) + strlen(pd->trans_hid) + 50);
  strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(pd->device_hid));
  strcat(uri, "/state/trans/");
  strcat(uri, pd->trans_hid);
  switch( pd->put_type ) {
    case st_received:
      strcat(uri, "/received");
    break;
    case st_complete:
      strcat(uri, "/succeeded");
    break;
    case st_error: {
      strcat(uri, "/error");
      _error = json_mkobject();
      json_append_member(_error, p_const("error"), json_mkstring("unknown"));
    }
    break;
    default:
      FREE_CHUNK(uri);
      return;
  }
  FREE_CHUNK(uri);
  http_request_init(request, PUT, uri);
  if ( _error ) {
    http_request_set_payload(request, json_encode_property(_error));
    json_delete(_error);
  }
}

static int _arrow_put_state(property_t device_hid, _st_put_api put_type, const char *trans_hid) {
    put_dev_t pd = {device_hid, trans_hid, put_type};
    STD_ROUTINE(_state_put_init, &pd, NULL, NULL, "State put failed...");
}

int ev_DeviceStateRequest(void *_ev, JsonNode *_parameters) {
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  SSP_PARAMETER_NOT_USED(ev);
  if ( !IS_EMPTY(_device_hid) ) return -1;

  JsonNode *device_hid = json_find_member(_parameters, p_const("deviceHid"));
  if ( !device_hid ) {
      DBG("cannot find device HID");
      return -1;
  }

  JsonNode *trans_hid = json_find_member(_parameters, p_const("transHid"));
  if ( !trans_hid ) {
      DBG("cannot find trans HID");
      return -1;
  }

  JsonNode *payload = json_find_member(_parameters, p_const("payload"));
  if ( !payload ) {
      DBG("cannot find payload");
      return -1;
  }
  int retry = 0;

  while ( _arrow_put_state(_device_hid, st_received, trans_hid->string_) < 0 ) {
      RETRY_UP(retry, {return -2;});
      msleep(ARROW_RETRY_DELAY);
  }

  int ret = state_handler(payload->string_);

  if ( ret < 0 ) {
    while ( _arrow_put_state(_device_hid, st_error, trans_hid->string_) < 0 ) {
        RETRY_UP(retry, {return -2;});
        msleep(ARROW_RETRY_DELAY);
    }
  } else {
    while ( _arrow_put_state(_device_hid, st_complete, trans_hid->string_) < 0 ) {
        RETRY_UP(retry, {return -2;});
        msleep(ARROW_RETRY_DELAY);
    }
  }
  return 0;
}
#endif
