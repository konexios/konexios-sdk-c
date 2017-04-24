#include "arrow/state.h"
#include <debug.h>
#include <http/client.h>
#include <arrow/request.h>
#include <json/json.h>
#include <arrow/mem.h>
#include <time/time.h>

static JsonNode *state_tree = NULL;

void add_state(const char *name, const char *value) {
  if ( !state_tree ) state_tree = json_mkobject();
  json_append_member(state_tree, name, json_mkstring(value));
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

static int _arrow_put_state(arrow_device_t *device, _st_put_api put_type, const char *trans_hid) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    JsonNode *_error = NULL;

    char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + strlen(device->hid) + strlen(trans_hid) + 20);
    strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, device->hid);
    strcat(uri, "/state/trans/");
    switch( put_type ) {
      case st_received:
        strcat(uri, "received");
      break;
      case st_complete:
        strcat(uri, "complete");
      break;
      case st_error: {
        strcat(uri, "error");
        _error = json_mkobject();
        json_append_member(_error, "states", json_mkstring("unknown"));
      }
      break;
      default:
        return -1;
    }
    http_client_init( &cli );
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
