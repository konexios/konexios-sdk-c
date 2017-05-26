#include "arrow/device_action.h"
#include <arrow/connection.h>
#include <debug.h>

struct _dev_model {
  arrow_device_t *device;
  dev_action_model_t *model;
};

static void _device_action_create_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  strcpy(uri, ARROW_API_DEVICE_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, dm->device->hid);
  strcat(uri, "/actions");
  http_request_init(request, POST, uri);
  free(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "criteria", json_mkstring(dm->model->criteria));
  json_append_member(_main, "description", json_mkstring(dm->model->description));
  json_append_member(_main, "enabled", json_mkbool(dm->model->enabled));
  json_append_member(_main, "expiration", json_mknumber(dm->model->expiration));
  json_append_member(_main, "index", json_mknumber(dm->model->index));
//  json_append_member(_main, "", json_mkstring(dm->model));
  json_append_member(_main, "systemName", json_mkstring(dm->model->systemName));
  char *payload = json_encode(_main);
  json_minify(payload);
  json_delete(_main);
  http_request_set_payload(request, payload);
  free(payload);
}

int arrow_create_device_action(arrow_device_t *dev, dev_action_model_t *model) {
  int ret = 0;
  struct _dev_model dm = {dev, model};
  ret = __http_routine(_device_action_create_init, &dm, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Action create failed...");
  }
  return ret;
}

static void _device_action_delete_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/actions/%d", ARROW_API_DEVICE_ENDPOINT, dm->device->hid, dm->model->index);
  http_request_init(request, DELETE, uri);
  free(uri);
}

int arrow_delete_device_action(arrow_device_t *dev, dev_action_model_t *model) {
  int ret = 0;
  struct _dev_model dm = {dev, model};
  ret = __http_routine(_device_action_delete_init, &dm, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Action delete failed...");
  }
  return ret;
}

static void _device_action_list_init(http_request_t *request, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/actions", ARROW_API_DEVICE_ENDPOINT, dev->hid);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _device_action_list_process(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  DBG("[%s]", response->payload.buf);
  return 0;
}

int arrow_list_device_action(arrow_device_t *dev) {
  int ret = __http_routine(_device_action_list_init, dev, _device_action_list_process, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Action list failed...");
  }
  return ret;
}

static void _action_type_list_init(http_request_t *request, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/actions/types", ARROW_API_DEVICE_ENDPOINT);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _action_type_list_process(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("act list", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  DBG("[%s]", response->payload.buf);
  return 0;
}


int arrow_list_action_type(void) {
  int ret = __http_routine(_action_type_list_init, NULL, _action_type_list_process, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Action types list failed...");
  }
  return ret;
}

static void _device_action_update_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  char *uri = (char *)malloc(strlen(ARROW_API_DEVICE_ENDPOINT) + 50);
  snprintf(uri, strlen(ARROW_API_DEVICE_ENDPOINT) + 50,
           "%s/%s/actions/%d", ARROW_API_DEVICE_ENDPOINT, dm->device->hid, dm->model->index);
  http_request_init(request, PUT, uri);
  free(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "criteria", json_mkstring(dm->model->criteria));
  json_append_member(_main, "description", json_mkstring(dm->model->description));
  json_append_member(_main, "enabled", json_mkbool(dm->model->enabled));
  json_append_member(_main, "expiration", json_mknumber(dm->model->expiration));
  json_append_member(_main, "index", json_mknumber(dm->model->index));
//  json_append_member(_main, "", json_mkstring(dm->model));
  json_append_member(_main, "systemName", json_mkstring(dm->model->systemName));
  char *payload = json_encode(_main);
  json_minify(payload);
  json_delete(_main);
  http_request_set_payload(request, payload);
  free(payload);
}


int arrow_update_device_action(arrow_device_t *dev, dev_action_model_t *model) {
  struct _dev_model dm = {dev, model};
  int ret = __http_routine(_device_action_update_init, &dm, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device Action update failed...");
  }
  return ret;
}
