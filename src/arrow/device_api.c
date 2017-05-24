#include "arrow/device_api.h"
#include <debug.h>
#include <stdarg.h>

typedef struct _gate_dev {
  arrow_gateway_t *gateway;
  arrow_device_t *device;
} gate_dev_t;

static void _device_register_init(http_request_t *request, void *arg) {
  gate_dev_t *gd = (gate_dev_t *)arg;
  http_request_init(request, POST, ARROW_API_DEVICE_ENDPOINT);
  arrow_prepare_device(gd->gateway, gd->device);
  char *payload = arrow_device_serialize(gd->device);
  http_request_set_payload(request, payload);
  DBG("dev|%s|", payload);
  free(payload);
}

static int _device_register_proc(http_response_t *response, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  if ( arrow_device_parse(dev, (char*)response->payload.buf) < 0) {
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
      DBG("Arrow Gateway register failed...");
    }
    return ret;
}

static void add_find_param(find_by_t **first, find_by_t *val) {
  find_by_t *tmp = malloc(sizeof(find_by_t));
  tmp->key = val->key;
  tmp->value = val->value;
  tmp->next = val->next;
  if ( ! *first ) *first = tmp;
  else {
    find_by_t *t_first = *first;
    while( t_first->next ) t_first = t_first->next;
    t_first->next = tmp;
  }
}

static void _device_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_DEVICE_ENDPOINT);
  if ( params ) {
    do {
      const char *key = NULL;
      switch(params->key) {
        case f_userHid: key = "userHid"; break;
        case f_uid: key = "uid"; break;
        case f_type: key = "type"; break;
        case f_gatewayHid: key = "gatewayHid"; break;
        case f_createdBefore: key = "createdBefore"; break;
        case f_createdAfter: key = "createdAfter"; break;
        case f_updatedBefore: key = "updatedBefore"; break;
        case f_updatedAfter: key = "updatedAfter"; break;
        case f_enabled: key = "enabled"; break;
        case f_page: key = "_page"; break;
        case f_size: key = "_size"; break;
        default: break;
      }
      if (key) http_request_add_query(request, key, params->value);
      params = params->next;
    } while( params );
  }
}

static int _device_find_by_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find[%s]", response->payload.buf);
  } else return -1;
  return 0;
}

int arrow_device_find_by(int n, ...) {
  find_by_t val;
  find_by_t *params = NULL;
  va_list args;
  va_start(args, n);
  int i = 0;
  for (i=0; i < n; i++) {
    val = va_arg(args, find_by_t);
    add_find_param(&params, &val);
  }
  va_end(args);

  int ret = __http_routine(_device_find_by_init, params, _device_find_by_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway find by failed...");
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
    DBG("find[%s]", response->payload.buf);
  } else return -1;
  return 0;
}


int arrow_device_find_by_hid(const char *hid) {
  int ret = __http_routine(_device_find_by_hid_init, (void *)hid, _device_find_by_hid_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway find by failed...");
  }
  return ret;
}
