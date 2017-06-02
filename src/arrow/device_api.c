#include "arrow/device_api.h"
#include <debug.h>

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
  DBG("dev|%s|", payload);
}

static int _device_register_proc(http_response_t *response, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  if ( arrow_device_parse(dev, P_VALUE(response->payload.buf)) < 0) {
      DBG("device parse error");
      return -1;
  } else {
      DBG("device hid: %s", dev->hid);
  }
  return 0;
}

int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device) {
    gate_dev_t gd = {gateway, device};
    int ret = __http_routine(_device_register_init, &gd, _device_register_proc, device);
    if ( ret < 0 ) {
      DBG("Arrow Device register failed...");
    }
    return ret;
}

static void _device_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_DEVICE_ENDPOINT);
  ADD_FIND_BY_TO_REQ(params, request);
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
  COLLECT_FIND_BY(params, n);

  int ret = __http_routine(_device_find_by_init, params, _device_find_by_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device find by failed...");
  }

  return ret;
}

static void _device_find_by_hid_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s", ARROW_API_DEVICE_ENDPOINT, hid);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _device_find_by_hid_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find[%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}


int arrow_device_find_by_hid(const char *hid) {
  int ret = __http_routine(_device_find_by_hid_init, (void *)hid, _device_find_by_hid_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device find by failed...");
  }
  return ret;
}

static void _device_update_init(http_request_t *request, void *arg) {
  gate_dev_t *gd = (gate_dev_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s", ARROW_API_DEVICE_ENDPOINT, gd->device->hid);
  http_request_init(request, PUT, uri);
  free(uri);
  char *payload = arrow_device_serialize(gd->device);
  http_request_set_payload(request, p_heap(payload));
  DBG("dev|%s|", payload);
}

static int _device_update_proc(http_response_t *response, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  if ( arrow_device_parse(dev, P_VALUE(response->payload.buf)) < 0) {
      DBG("device parse error");
      return -1;
  } else {
      DBG("device hid: %s", dev->hid);
  }
  return 0;
}

int arrow_update_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  gate_dev_t gd = {gateway, device};
  int ret = __http_routine(_device_update_init, &gd, _device_update_proc, device);
  if ( ret < 0 ) {
    DBG("Arrow Device update failed...");
  }
  return ret;
}

typedef struct _dev_params_ {
  arrow_device_t *device;
  find_by_t *params;
} dev_param_t;

static void _device_list_events_init(http_request_t *request, void *arg) {
  dev_param_t *dp = (dev_param_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/events", ARROW_API_DEVICE_ENDPOINT, dp->device->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = dp->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _device_list_events_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list events: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_list_device_events(arrow_device_t *device, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  dev_param_t dp = { device, params };
  int ret = __http_routine(_device_list_events_init, &dp, _device_list_events_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow list device events failed...");
  }
  return ret;
}

static void _device_list_logs_init(http_request_t *request, void *arg) {
  dev_param_t *dp = (dev_param_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/logs", ARROW_API_DEVICE_ENDPOINT, dp->device->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = dp->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _device_list_logs_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list events: %s", P_VALUE(response->payload.buf));
  return 0;
}

int arrow_list_device_logs(arrow_device_t *device, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  dev_param_t dp = { device, params };
  int ret = __http_routine(_device_list_logs_init, &dp, _device_list_logs_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow list device events failed...");
  }
  return ret;
}

typedef struct _device_error {
  arrow_device_t *device;
  const char *error;
} device_error_t;

static void _device_errors_init(http_request_t *request, void *arg) {
  device_error_t *de = (device_error_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/errors", ARROW_API_DEVICE_ENDPOINT, de->device->hid);
  http_request_init(request, POST, uri);
  free(uri);
  JsonNode *error = json_mkobject();
  json_append_member(error, "error", json_mkstring(de->error));
  http_request_set_payload(request, p_heap(json_encode(error)));
  json_delete(error);
}

//static int _device_errors_proc(http_response_t *response, void *arg) {
//  SSP_PARAMETER_NOT_USED(arg);
//  DBG("dev list events: %s", response->payload.buf);
//  return 0;
//}

int arrow_error_device(arrow_device_t *device, const char *error) {
  device_error_t de = { device, error };
  int ret = __http_routine(_device_errors_init, &de, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device error failed...");
  }
  return ret;
}
