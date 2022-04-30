#include "konexios/api/device/action.h"
#include <http/routine.h>
#include <sys/mem.h>
#include <debug.h>
#include <data/chunk.h>

#define URI_LEN sizeof(KONEXIOS_API_DEVICE_ENDPOINT) + 50

struct _dev_model {
  konexios_device_t *device;
  dev_action_model_t *model;
};

static void _device_action_create_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  strcpy(uri, KONEXIOS_API_DEVICE_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, P_VALUE(dm->device->hid));
  strcat(uri, "/actions");
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, p_const("criteria"), json_mkstring(dm->model->criteria));
  json_append_member(_main, p_const("description"), json_mkstring(dm->model->description));
  json_append_member(_main, p_const("enabled"), json_mkbool(dm->model->enabled));
  json_append_member(_main, p_const("expiration"), json_mknumber(dm->model->expiration));
  json_append_member(_main, p_const("index"), json_mknumber(dm->model->index));
  json_append_member(_main, p_const("systemName"), json_mkstring(dm->model->systemName));
  http_request_set_payload(request, json_encode_property(_main));
  json_delete(_main);
}

int konexios_create_device_action(konexios_device_t *dev, dev_action_model_t *model) {
  struct _dev_model dm = {dev, model};
  STD_ROUTINE(_device_action_create_init, &dm,
              NULL, NULL,
              "Arrow Device Action create failed...");
}

static void _device_action_delete_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/actions/%d", KONEXIOS_API_DEVICE_ENDPOINT,
           P_VALUE(dm->device->hid), dm->model->index);
  http_request_init(request, DELETE, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_delete_device_action(konexios_device_t *dev, dev_action_model_t *model) {
  struct _dev_model dm = {dev, model};
  STD_ROUTINE(_device_action_delete_init, &dm,
              NULL, NULL,
              "Arrow Device Action delete failed...");
}

static void _device_action_list_init(http_request_t *request, void *arg) {
  konexios_device_t *dev = (konexios_device_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/actions", KONEXIOS_API_DEVICE_ENDPOINT,
           P_VALUE(dev->hid));
  http_request_init(request, GET, &p_stack(uri));
  FREE_CHUNK(uri);
}

static int _device_action_list_process(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("dev list %hu", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  DBG("[%s]", P_VALUE(response->payload));
  return 0;
}

int konexios_list_device_action(konexios_device_t *dev) {
  STD_ROUTINE(_device_action_list_init, dev,
              _device_action_list_process, NULL,
              "Arrow Device Action list failed...");
}

static void _action_type_list_init(http_request_t *request, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/actions/types", KONEXIOS_API_DEVICE_ENDPOINT);
  http_request_init(request, GET, &p_stack(uri));
  FREE_CHUNK(uri);
}

static int _action_type_list_process(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("act list %hu", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  DBG("[%s]", P_VALUE(response->payload));
  return 0;
}


int konexios_list_action_type(void) {
  STD_ROUTINE(_action_type_list_init, NULL,
              _action_type_list_process, NULL,
              "Arrow Action types list failed...");
}

static void _device_action_update_init(http_request_t *request, void *arg) {
  struct _dev_model *dm = (struct _dev_model *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN,
           "%s/%s/actions/%d", KONEXIOS_API_DEVICE_ENDPOINT,
           P_VALUE(dm->device->hid), dm->model->index);
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, p_const("criteria"), json_mkstring(dm->model->criteria));
  json_append_member(_main, p_const("description"), json_mkstring(dm->model->description));
  json_append_member(_main, p_const("enabled"), json_mkbool(dm->model->enabled));
  json_append_member(_main, p_const("expiration"), json_mknumber(dm->model->expiration));
  json_append_member(_main, p_const("index"), json_mknumber(dm->model->index));
  json_append_member(_main, p_const("systemName"), json_mkstring(dm->model->systemName));
  http_request_set_payload(request, json_encode_property(_main));
  json_delete(_main);
}


int konexios_update_device_action(konexios_device_t *dev, dev_action_model_t *model) {
  struct _dev_model dm = {dev, model};
  STD_ROUTINE(_device_action_update_init, &dm,
              NULL, NULL,
              "Arrow Device Action update failed...");
}