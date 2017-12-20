#include "arrow/api/device/device.h"
#include <http/routine.h>
#include <debug.h>
#include <data/chunk.h>

#define URI_LEN sizeof(ARROW_API_DEVICE_ENDPOINT) + 50
#define DEVICE_MSG "Device %d"

typedef struct _gate_dev {
  arrow_gateway_t *gateway;
  arrow_device_t *device;
} gate_dev_t;

static void _device_register_init(http_request_t *request, void *arg) {
  gate_dev_t *gd = (gate_dev_t *)arg;
  http_request_init(request, POST, ARROW_API_DEVICE_ENDPOINT);
  arrow_prepare_device(gd->gateway, gd->device);
  char *payload = arrow_device_serialize(gd->device);
  http_request_set_payload(request, p_heap(payload));
}

static int _device_register_proc(http_response_t *response, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  if ( arrow_device_parse(dev, P_VALUE(response->payload.buf)) < 0) {
      DBG("Parse error");
      return -1;
  } else {
      DBG("device hid: %s", P_VALUE(dev->hid));
  }
  return 0;
}

int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device) {
    gate_dev_t gd = {gateway, device};
    STD_ROUTINE(_device_register_init, &gd,
                _device_register_proc, device,
                DEVICE_MSG, DEVICE_REGISTER_ERROR);
}

static void _device_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_DEVICE_ENDPOINT);
  http_request_set_findby(request, params);
}

static int _device_find_by_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find[%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}

int arrow_device_find_by(int n, ...) {
  find_by_t *params = NULL;
  find_by_collect(params, n);
  STD_ROUTINE(_device_find_by_init, (void*)params,
              _device_find_by_proc, NULL,
              DEVICE_MSG, DEVICE_FINDBY_ERROR);
}

static void _device_find_by_hid_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s",
           ARROW_API_DEVICE_ENDPOINT,
           hid);
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

static int _device_find_by_hid_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find[%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}


int arrow_device_find_by_hid(const char *hid) {
  STD_ROUTINE(_device_find_by_hid_init, (void *)hid,
              _device_find_by_hid_proc, NULL,
              DEVICE_MSG, DEVICE_FINDBY_ERROR);
}

static void _device_update_init(http_request_t *request, void *arg) {
  gate_dev_t *gd = (gate_dev_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
           "%s/%s",
           ARROW_API_DEVICE_ENDPOINT,
           P_VALUE(gd->device->hid));
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
  char *payload = arrow_device_serialize(gd->device);
  http_request_set_payload(request, p_heap(payload));
}

static int _device_update_proc(http_response_t *response, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  if ( arrow_device_parse(dev, P_VALUE(response->payload.buf)) < 0) {
      DBG("Parse error");
      return -1;
  } else {
      DBG("device hid: %s", P_VALUE(dev->hid));
  }
  return 0;
}

int arrow_update_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  gate_dev_t gd = {gateway, device};
  STD_ROUTINE(_device_update_init, &gd,
              _device_update_proc, device,
              DEVICE_MSG, DEVICE_UPDATE_ERROR);
}

typedef struct _dev_params_ {
  arrow_device_t *device;
  find_by_t *params;
} dev_param_t;

static void _device_list_events_init(http_request_t *request, void *arg) {
  dev_param_t *dp = (dev_param_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/events",
           ARROW_API_DEVICE_ENDPOINT,
           P_VALUE(dp->device->hid));
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  http_request_set_findby(request, dp->params);
}

static int _device_list_events_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list events: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_list_device_events(arrow_device_t *device, int n, ...) {
  find_by_t *params = NULL;
  find_by_collect(params, n);
  dev_param_t dp = { device, params };
  STD_ROUTINE(_device_list_events_init, &dp,
              _device_list_events_proc, NULL,
              DEVICE_MSG, DEVICE_EVLIST_ERROR);
}

static void _device_list_logs_init(http_request_t *request, void *arg) {
  dev_param_t *dp = (dev_param_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/logs",
           ARROW_API_DEVICE_ENDPOINT,
           P_VALUE(dp->device->hid));
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  http_request_set_findby(request, dp->params);
}

static int _device_list_logs_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list events: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_list_device_logs(arrow_device_t *device, int n, ...) {
  find_by_t *params = NULL;
  find_by_collect(params, n);
  dev_param_t dp = { device, params };
  STD_ROUTINE(_device_list_logs_init, &dp,
              _device_list_logs_proc, NULL,
              DEVICE_MSG, DEVICE_LOGS_ERROR);
}

typedef struct _device_error {
  arrow_device_t *device;
  const char *error;
} device_error_t;

static void _device_errors_init(http_request_t *request, void *arg) {
  device_error_t *de = (device_error_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/errors",
           ARROW_API_DEVICE_ENDPOINT,
           P_VALUE(de->device->hid));
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  JsonNode *error = json_mkobject();
  json_append_member(error, "error", json_mkstring(de->error));
  http_request_set_payload(request, p_heap(json_encode(error)));
  json_delete(error);
}

int arrow_error_device(arrow_device_t *device, const char *error) {
  device_error_t de = { device, error };
  STD_ROUTINE(_device_errors_init, &de,
              NULL, NULL,
              DEVICE_MSG, DEVICE_ERROR);
}
