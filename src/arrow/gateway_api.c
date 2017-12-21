#include "arrow/gateway_api.h"
#include <http/routine.h>
#include <arrow/sign.h>
#include <debug.h>
#include <data/chunk.h>

void who_when_init(who_when_t *ww) {
    memset(&ww->date, 0x0, sizeof(struct tm));
    property_init(&ww->by);
}

void who_when_free(who_when_t *ww) {
    property_free(&ww->by);
}

void gateway_info_init(gateway_info_t *gi) {
    who_when_init(&gi->created);
    who_when_init(&gi->lastModified);
    property_init(&gi->deviceType);
    property_init(&gi->hid);
    property_init(&gi->name);
    property_init(&gi->osName);
    property_init(&gi->softwareName);
    property_init(&gi->softwareVersion);
    property_init(&gi->type);
    property_init(&gi->uid);
}

void gateway_info_free(gateway_info_t *gi) {
    who_when_free(&gi->created);
    who_when_free(&gi->lastModified);
    property_free(&gi->deviceType);
    property_free(&gi->hid);
    property_free(&gi->name);
    property_free(&gi->osName);
    property_free(&gi->softwareName);
    property_free(&gi->softwareVersion);
    property_free(&gi->type);
    property_free(&gi->uid);
}

void gateway_log_init(gateway_log_t *gi) {
    who_when_init(&gi->created);
    property_init(&gi->objectHid);
    property_init(&gi->productName);
    property_init(&gi->type);
    gi->parameters = NULL;
}

void gateway_log_free(gateway_log_t *gi) {
    who_when_free(&gi->created);
    property_free(&gi->objectHid);
    property_free(&gi->productName);
    property_free(&gi->type);
    if (gi->parameters) json_delete(gi->parameters);
}

#define URI_LEN sizeof(ARROW_API_GATEWAY_ENDPOINT) + 50
#define GATEWAY_MSG "Gateway %d"

static void _gateway_config_init(http_request_t *request, void *arg) {
	arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
	CREATE_CHUNK(uri, URI_LEN);
    int ret = snprintf(uri, URI_LEN,
                       "%s/%s/config",
                       ARROW_API_GATEWAY_ENDPOINT,
                       P_VALUE(gateway->hid) );
    if ( ret > 0 ) uri[ret] = 0x0;
	http_request_init(request, GET, uri);
	FREE_CHUNK(uri);
}

static int _gateway_config_proc(http_response_t *response, void *arg) {
	arrow_gateway_config_t *config = (arrow_gateway_config_t *)arg;
	if ( response->m_httpResponseCode != 200 ) {
		return -1;
	}
	DBG("pay: {%s}\r\n", P_VALUE(response->payload.buf));

	JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
	if ( !_main ) {
		DBG("Parse error");
		return -1;
	}
	JsonNode *_main_key = json_find_member(_main, "key");
	if ( _main_key ) {
        JsonNode *tmp;
		tmp = json_find_member(_main_key, "apiKey");
		if (tmp) {
			set_api_key(tmp->string_);
		}
		tmp = json_find_member(_main_key, "secretKey");
		if (tmp) {
			set_secret_key(tmp->string_);
		}
	} else {
		DBG("There is no keys!");
		return -1;
	}
	arrow_gateway_config_init(config);
#if defined(__IBM__)
	JsonNode *_main_ibm = json_find_member(_main, "ibm");
	if ( _main_ibm ) {
		config->type = 1;
		tmp = json_find_member(_main_ibm, "organizationId");
		if ( tmp ) arrow_gateway_config_add_organizationId(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "gatewayType");
		if ( tmp ) arrow_gateway_config_add_gatewayType(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "gatewayId");
		if ( tmp ) arrow_gateway_config_add_gatewayId(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "authToken");
		if ( tmp ) arrow_gateway_config_add_authToken(config, tmp->string_);
		tmp = json_find_member(_main_ibm, "authMethod");
		if ( tmp ) arrow_gateway_config_add_authMethod(config, tmp->string_);
	}
#elif defined(__AZURE__)
	JsonNode *_main_azure = json_find_member(_main, "azure");
	if ( _main_azure ) {
		config->type = 3;
		tmp = json_find_member(_main_azure, "host");
		if ( tmp ) arrow_gateway_config_add_host(config, tmp->string_);
		tmp = json_find_member(_main_azure, "accessKey");
		if ( tmp ) arrow_gateway_config_add_accessKey(config, tmp->string_);
	}
#endif
	json_delete(_main);
	return 0;
}

int arrow_gateway_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config) {
    STD_ROUTINE(_gateway_config_init, gateway,
                _gateway_config_proc, config,
                GATEWAY_MSG, GATEWAY_CONFIG_ERROR);
}


static void _gateway_register_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  http_request_init(request, POST, ARROW_API_GATEWAY_ENDPOINT);
  char *payload = arrow_gateway_serialize(gateway);
  http_request_set_payload(request, p_heap(payload));
}

static int _gateway_register_proc(http_response_t *response, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  if ( response->m_httpResponseCode == 200 ) {
      if ( arrow_gateway_parse(gateway, P_VALUE(response->payload.buf)) < 0 ) {
          DBG("parse error");
          return -1;
      } else {
          DBG("gateway hid: %s", P_VALUE(gateway->hid) );
      }
  } else return -1;
  return 0;
}

int arrow_register_gateway(arrow_gateway_t *gateway) {
  STD_ROUTINE(_gateway_register_init, gateway,
              _gateway_register_proc, gateway,
              GATEWAY_MSG, GATEWAY_REGISTER_ERROR);
}

static void _gateway_heartbeat_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s/heartbeat",
                     ARROW_API_GATEWAY_ENDPOINT,
                     P_VALUE(gateway->hid) );
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_gateway_heartbeat(arrow_gateway_t *gateway) {
  STD_ROUTINE(_gateway_heartbeat_init, gateway,
              NULL, NULL,
              GATEWAY_MSG, GATEWAY_HEARTBEAT_ERROR);
}

static void _gateway_checkin_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s/checkin",
                     ARROW_API_GATEWAY_ENDPOINT,
                     P_VALUE(gateway->hid));
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
}

int arrow_gateway_checkin(arrow_gateway_t *gateway) {
  STD_ROUTINE(_gateway_checkin_init, gateway,
              NULL, NULL,
              GATEWAY_MSG, GATEWAY_CHECKIN_ERROR);
}

static void _gateway_find_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s",
                     ARROW_API_GATEWAY_ENDPOINT,
                     hid);
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

static int _gateway_find_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find [%s]", P_VALUE(response->payload.buf));
  } else return -1;
  return 0;
}

int arrow_gateway_find(const char *hid) {
  STD_ROUTINE(_gateway_find_init, (void*)hid,
              _gateway_find_proc, NULL,
              GATEWAY_MSG, GATEWAY_FIND_ERROR);
}

static void _gateway_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_GATEWAY_ENDPOINT);
  http_request_set_findby(request, params);
}

static int _gateway_find_by_proc(http_response_t *response, void *arg) {
  gateway_info_t **info = (gateway_info_t **)arg;
  *info = NULL;
  JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
  if ( !_main ) return -1;
  JsonNode *_data = parse_size_data(_main, NULL);
  if ( _data ) {
      JsonNode *tmp = NULL;
      json_foreach(tmp, _data) {
          gateway_info_t *gi = (gateway_info_t *)malloc(sizeof(gateway_info_t));
          gateway_info_init(gi);
          JsonNode *t = json_find_member(tmp, "hid");
          if ( t && t->tag == JSON_STRING )
              property_copy( &gi->hid, p_stack(t->string_));
          parse_who_when(tmp, &gi->created, "createdDate", "createdBy");
          parse_who_when(tmp, &gi->lastModified, "lastModifiedDate", "lastModifiedBy");
          json_fill_property(tmp, gi, uid);
          json_fill_property(tmp, gi, name);
          json_fill_property(tmp, gi, type);
          json_fill_property(tmp, gi, deviceType);
          json_fill_property(tmp, gi, osName);
          json_fill_property(tmp, gi, softwareName);
          json_fill_property(tmp, gi, softwareVersion);
          linked_list_add_node_last(*info, gateway_info_t, gi);
      }
  }
  json_delete(_main);
  return 0;
}


int arrow_gateway_find_by(gateway_info_t **info, int n, ...) {
  find_by_t *params = NULL;
  find_by_collect(params, n);
  STD_ROUTINE(_gateway_find_by_init, params,
              _gateway_find_by_proc, (void*)info,
              GATEWAY_MSG, GATEWAY_FINDBY_ERROR);
}

typedef struct _gate_param_ {
  arrow_gateway_t *gate;
  find_by_t *params;
} gate_param_t;

static void _gateway_list_logs_init(http_request_t *request, void *arg) {
  gate_param_t *dp = (gate_param_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s/logs",
                     ARROW_API_GATEWAY_ENDPOINT,
                     P_VALUE(dp->gate->hid) );
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  http_request_set_findby(request, dp->params);
}

static int _gateway_list_logs_proc(http_response_t *response, void *arg) {
    gateway_log_t **logs = (gateway_log_t **)arg;
    *logs = NULL;
    JsonNode *_main = json_decode(P_VALUE(response->payload.buf));
    if ( !_main ) return -1;
    JsonNode *_data = parse_size_data(_main, NULL);
    if ( _data ) {
        JsonNode *tmp = NULL;
        json_foreach(tmp, _data) {
            gateway_log_t *gl = (gateway_log_t *)malloc(sizeof(gateway_log_t));
            gateway_log_init(gl);
            parse_who_when(tmp, &gl->created, "createdDate", "createdBy");
            json_fill_property(tmp, gl, productName);
            json_fill_property(tmp, gl, type);
            json_fill_property(tmp, gl, objectHid);
            JsonNode *t = json_find_member(tmp, "parameters");
            json_remove_from_parent(t);
            gl->parameters = t;
            linked_list_add_node_last(*logs, gateway_log_t, gl);
        }
    }
    json_delete(_main);
    return 0;
}

int arrow_gateway_logs_list(gateway_log_t **logs, arrow_gateway_t *gateway, int n, ...) {
  find_by_t *params = NULL;
  find_by_collect(params, n);
  gate_param_t dp = { gateway, params };
  STD_ROUTINE(_gateway_list_logs_init, &dp,
              _gateway_list_logs_proc, (void*)logs,
              GATEWAY_MSG, GATEWAY_LOGS_ERROR);
}

static void _gateway_devices_list_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/%s/devices", ARROW_API_GATEWAY_ENDPOINT, hid);
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
}

static int _gateway_devices_list_proc(http_response_t *response, void *arg) {
    device_info_t **devs = (device_info_t **)arg;
    *devs = NULL;
    return device_info_parse(devs, P_VALUE(response->payload.buf));
}

int arrow_gateway_devices_list(device_info_t **list, const char *hid) {
  STD_ROUTINE(_gateway_devices_list_init, (void*)hid,
              _gateway_devices_list_proc, list,
              GATEWAY_MSG, GATEWAY_DEVLIST_ERROR);
}

typedef struct _gate_dev_cmd_ {
  const char *g_hid;
  const char *d_hid;
  const char *cmd;
  const char *payload;
} gate_dev_cmd_t;

static void _gateway_device_cmd_init(http_request_t *request, void *arg) {
  gate_dev_cmd_t *gdc = (gate_dev_cmd_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
           "%s/%s/devices/%s/actions/command",
           ARROW_API_GATEWAY_ENDPOINT,
           gdc->g_hid,
           gdc->d_hid);
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, GET, uri);
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "command", json_mkstring(gdc->cmd));
  json_append_member(_main, "deviceHid", json_mkstring(gdc->d_hid));
  json_append_member(_main, "payload", json_mkstring(gdc->payload));
  http_request_set_payload(request, p_heap(json_encode(_main)));
  json_delete(_main);
}

int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload) {
  gate_dev_cmd_t gdc = {gHid, dHid, cmd, payload};
  STD_ROUTINE(_gateway_device_cmd_init, &gdc,
              NULL, NULL,
              GATEWAY_MSG, GATEWAY_DEVCOMS_ERROR);
}

typedef struct _gateway_error {
  arrow_gateway_t *gateway;
  const char *error;
} gateway_error_t;

static void _gateway_errors_init(http_request_t *request, void *arg) {
  gateway_error_t *de = (gateway_error_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s/errors",
                     ARROW_API_GATEWAY_ENDPOINT,
                     P_VALUE(de->gateway->hid) );
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, POST, uri);
  FREE_CHUNK(uri);
  JsonNode *error = json_mkobject();
  json_append_member(error, "error", json_mkstring(de->error));
  http_request_set_payload(request, p_heap(json_encode(error)));
  json_delete(error);
}

int arrow_gateway_error(arrow_gateway_t *gateway, const char *error) {
  gateway_error_t de = { gateway, error };
  STD_ROUTINE(_gateway_errors_init, &de,
              NULL, NULL,
              GATEWAY_MSG, GATEWAY_ERROR);
}

static void _gateway_update_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gate = (arrow_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN,
                     "%s/%s",
                     ARROW_API_GATEWAY_ENDPOINT,
                     P_VALUE(gate->hid) );
  if ( ret > 0 ) uri[ret] = 0x0;
  http_request_init(request, PUT, uri);
  FREE_CHUNK(uri);
  http_request_set_payload(request, p_heap(arrow_gateway_serialize(gate)));
}

static int _gateway_update_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  return 0;
}

int arrow_gateway_update(arrow_gateway_t *gateway) {
  STD_ROUTINE(_gateway_update_init, gateway,
              _gateway_update_proc, NULL,
              GATEWAY_MSG, GATEWAY_UPDATE_ERROR);
}
