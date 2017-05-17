#include "arrow/state.h"
#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>
#include <time/time.h>
#include <arrow/events.h>

static JsonNode *state_tree = NULL;
static char *_device_hid = NULL;

int state_handler(char *str) __attribute__((weak));

void add_state(const char *name, const char *value) {
  if ( !state_tree ) state_tree = json_mkobject();
  json_append_member(state_tree, name, json_mkstring(value));
}

int arrow_state_mqtt_is_running() {
  if ( !state_tree ) return -1;
  return 0;
}

int arrow_state_mqtt_run(arrow_device_t *device) {
  if ( !_device_hid ) {
    _device_hid = malloc(strlen(device->hid)+1);
    strcpy(_device_hid, device->hid);
  }
  return arrow_state_mqtt_is_running();
}

int arrow_get_state(arrow_device_t *device) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;

    char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + strlen(device->hid) + 10);
    strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, device->hid);
    strcat(uri, "/state");
    http_client_init( &cli );
    http_request_init(&request, GET, uri);
    sign_request(&request);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);
    DBG("[%s]", response.payload.buf);
    http_client_free(&cli);
    free(uri);
    if ( response.m_httpResponseCode != 200 ) {
        http_response_free(&response);
        return -1;
    }
    http_response_free(&response);
    return 0;
}

typedef enum {
  st_request,
  st_update
} _st_post_api;

static int _arrow_post_state(arrow_device_t *device, _st_post_api post_type) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    JsonNode *_state = NULL;

    char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + strlen(device->hid) + 20);
    strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, device->hid);
    strcat(uri, "/state/");
    switch( post_type ) {
      case st_request:
        strcat(uri, "request");
      break;
      case st_update:
        strcat(uri, "update");
      break;
      default:
        return -1;
    }
    http_client_init( &cli );
    http_request_init(&request, POST, uri);
    {
      _state = json_mkobject();
      json_append_member(_state, "states", state_tree);
      char ts[30];
      get_time(ts);
      json_append_member(_state, "timestamp", json_mkstring(ts));
    }
    http_request_set_payload(&request, json_encode(_state));
    sign_request(&request);
    DBG("send: %s", request.payload.buf);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);
    DBG("[%s]", response.payload.buf);
    http_client_free(&cli);
    if (_state) json_delete(_state);
    free(uri);
    if ( response.m_httpResponseCode != 200 ) {
        http_response_free(&response);
        return -1;
    }
    http_response_free(&response);
    return 0;
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

static int _arrow_put_state(const char *device_hid, _st_put_api put_type, const char *trans_hid) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    JsonNode *_error = NULL;

    http_client_init( &cli );
    char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + strlen(device_hid) + strlen(trans_hid) + 50);
    strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, device_hid);
    strcat(uri, "/state/trans/");
    strcat(uri, trans_hid);
    switch( put_type ) {
      case st_received:
        strcat(uri, "/received");
      break;
      case st_complete:
        strcat(uri, "/succeeded");
      break;
      case st_error: {
        strcat(uri, "/error");
        _error = json_mkobject();
        json_append_member(_error, "error", json_mkstring("unknown"));
      }
      break;
      default:
        return -1;
    }
    http_request_init(&request, PUT, uri);

    if ( _error ) {
      http_request_set_payload(&request, json_encode(_error));
      json_delete(_error);
    }
    sign_request(&request);
    DBG("send: %s", request.payload.buf);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);
    DBG("[%s]", response.payload.buf);
    http_client_free(&cli);
    free(uri);
    if ( response.m_httpResponseCode != 200 ) {
        http_response_free(&response);
        return -1;
    }
    http_response_free(&response);
    return 0;
}

int state_handler(char *str) {
  SSP_PARAMETER_NOT_USED(str);
  DBG("weak state handler [%s]", str);
  return 0;
} // __attribute__((weak))

int ev_DeviceStateRequest(void *_ev, JsonNode *_parameters) {
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  SSP_PARAMETER_NOT_USED(ev);
  if ( !_device_hid ) return -1;

  JsonNode *device_hid = json_find_member(_parameters, "deviceHid");
  if ( !device_hid ) {
      DBG("cannot find device HID");
      return -1;
  }

  JsonNode *trans_hid = json_find_member(_parameters, "transHid");
  if ( !trans_hid ) {
      DBG("cannot find trans HID");
      return -1;
  }

  JsonNode *payload = json_find_member(_parameters, "payload");
  if ( !payload ) {
      DBG("cannot find payload");
      return -1;
  }

  _arrow_put_state(_device_hid, st_received, trans_hid->string_);

  int ret = state_handler(json_encode(payload));

  if ( ret < 0 ) {
    _arrow_put_state(_device_hid, st_error, trans_hid->string_);
  } else {
    _arrow_put_state(_device_hid, st_complete, trans_hid->string_);
  }
  return 0;
}
