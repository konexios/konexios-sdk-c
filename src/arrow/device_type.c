#include "arrow/device_type.h"

#include <debug.h>
#include <stdarg.h>

#define X_STR_COPY(dst, src) \
{ (dst) = malloc(strlen(src) + 1); \
  strcpy((dst), (src)); }

#define X_STR_FREE(str) if ( str ) free(str);

void device_type_init(device_type_t *dev, int enable, const char *name, const char *dec) {
  dev->enabled = enable;
  X_STR_COPY(dev->description, dec);
  X_STR_COPY(dev->name, name);
  dev->telemetries = NULL;
}

void device_type_add_telemetry(device_type_t *dev, int contr, const char *name, const char *type, const char *desc) {
  device_type_telemetry_t *telemetry;
  telemetry = malloc(sizeof(device_type_telemetry_t));
  telemetry->controllable = contr;
  X_STR_COPY(telemetry->description, desc);
  X_STR_COPY(telemetry->name, name);
  X_STR_COPY(telemetry->type, type);
  telemetry->next = NULL;
  device_type_telemetry_t *last = dev->telemetries;
  while( last && last->next )
    last = last->next;
  if ( !last ) {
    dev->telemetries = telemetry;
  } else {
    last->next = telemetry;
  }
}

void device_type_free(device_type_t *dev) {
  X_STR_FREE(dev->description);
  X_STR_FREE(dev->name);
  device_type_telemetry_t *telemetry = dev->telemetries;
  while ( telemetry ) {
    X_STR_FREE(telemetry->description);
    X_STR_FREE(telemetry->name);
    X_STR_FREE(telemetry->type);
    device_type_telemetry_t *t_free = telemetry;
    telemetry = telemetry->next;
    free(t_free);
  }
}

static void _device_type_list_init(http_request_t *request, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/types", ARROW_API_DEVICE_ENDPOINT);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _device_type_list_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("types[%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}


int arrow_device_type_list(void) {
  int ret = __http_routine(_device_type_list_init, NULL, _device_type_list_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Type list failed...");
  }
  return ret;
}

static char  *device_type_serialize(device_type_t *dev) {
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "description", json_mkstring(dev->description));
  json_append_member(_main, "enabled", json_mkbool(dev->enabled));
  json_append_member(_main, "name", json_mkstring(dev->name));
  JsonNode *tls = json_mkarray();
  device_type_telemetry_t *t = dev->telemetries;
  while( t ) {
    JsonNode *tl_element = json_mkobject();
    json_append_member(tl_element, "controllable", json_mkbool(t->controllable));
    json_append_member(tl_element, "description", json_mkstring(t->description));
    json_append_member(tl_element, "name", json_mkstring(t->name));
    json_append_member(tl_element, "type", json_mkstring(t->type));
    json_append_element(tls, tl_element);
    t = t->next;
  }
  json_append_member(_main, "telemetries", tls);
  char *payload = json_encode(_main);
  DBG("type pay: [%s]", payload);
  json_delete(_main);
  return payload;
}

static void _device_type_create_init(http_request_t *request, void *arg) {
  device_type_t *dev_type = (device_type_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/types", ARROW_API_DEVICE_ENDPOINT);
  http_request_init(request, POST, uri);
  free(uri);
  http_request_set_payload(request, p_heap(device_type_serialize(dev_type)));
}

//static int _device_type_create_proc(http_response_t *response, void *arg) {
//  SSP_PARAMETER_NOT_USED(arg);
//  if ( response->m_httpResponseCode == 200 ) {
//    DBG("types[%s]", response->payload.buf);
//  } else return -1;
//  return 0;
//}

int arrow_device_type_create(device_type_t *dev_type) {
  int ret = __http_routine(_device_type_create_init, dev_type, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Type create failed...");
  }
  return ret;
}

typedef struct _device_device_type_ {
  arrow_device_t *dev;
  device_type_t *type;
} device_device_type_t;


static void _device_type_update_init(http_request_t *request, void *arg) {
  device_device_type_t *ddt = (device_device_type_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/types/%s", ARROW_API_DEVICE_ENDPOINT,
           P_VALUE(ddt->dev->hid));
  http_request_init(request, PUT, uri);
  free(uri);
  http_request_set_payload(request, p_heap(device_type_serialize(ddt->type)));
}

int arrow_device_type_update(arrow_device_t *dev, device_type_t *dev_type) {
  device_device_type_t ddt = { dev, dev_type };
  int ret = __http_routine(_device_type_update_init, &ddt, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Type update failed...");
  }
  return ret;
}
