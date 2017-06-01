#include "arrow/gateway_api.h"
#include <debug.h>
#include <stdarg.h>

#define URI_LEN sizeof(ARROW_API_GATEWAY_ENDPOINT) + 50

//#define USE_HEAP

static void _gateway_config_init(http_request_t *request, void *arg) {
	arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
#if defined(USE_HEAD)
	char *uri = (char *)malloc(URI_LEN);
#else
	char uri[URI_LEN];
#endif
	strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
	strcat(uri, "/");
	strcat(uri, gateway->hid);
	strcat(uri, "/config");

	http_request_init(request, GET, uri);
#if defined(USE_HEAD)
	free(uri);
#endif
}

static int _gateway_config_proc(http_response_t *response, void *arg) {
	arrow_gateway_config_t *config = (arrow_gateway_config_t *)arg;
	DBG("response %d", response->m_httpResponseCode);
	if ( response->m_httpResponseCode != 200 ) {
		return -1;
	}
	DBG("pay: {%s}\r\n", response->payload.buf);

	JsonNode *_main = json_decode((char*)response->payload.buf);
	JsonNode *tmp;
	JsonNode *_main_key = json_find_member(_main, "key");
	if ( _main_key ) {
		tmp = json_find_member(_main_key, "apiKey");
		if (tmp) {
			DBG("(%d) api key: %s", strlen(tmp->string_), tmp->string_);
			set_api_key(tmp->string_);
		}
		tmp = json_find_member(_main_key, "secretKey");
		if (tmp) {
			DBG("(%d) secret key: %s", strlen(tmp->string_), tmp->string_);
			set_secret_key(tmp->string_);
		}
	} else return -1;
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
	int ret = __http_routine(_gateway_config_init, gateway, _gateway_config_proc, config);
	if ( ret < 0 ) {
		DBG("Arrow Gateway config failed...");
	}
	return ret;
}


static void _gateway_register_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  http_request_init(request, POST, ARROW_API_GATEWAY_ENDPOINT);
  char *payload = arrow_gateway_serialize(gateway);
  DBG("payload %s", payload);
  http_request_set_payload_ptr(request, payload);
}

static int _gateway_register_proc(http_response_t *response, void *arg) {
  arrow_gateway_t *gateway = (arrow_gateway_t *)arg;
  DBG("response gate reg %d", response->m_httpResponseCode);
  if ( arrow_gateway_parse(gateway, (char*)response->payload.buf) < 0 ) {
      DBG("parse error");
      return -1;
  } else {
      DBG("gateway hid: %s", gateway->hid);
  }
  return 0;
}

int arrow_register_gateway(arrow_gateway_t *gateway) {
	static int count = 0;
  int ret = __http_routine(_gateway_register_init, gateway, _gateway_register_proc, gateway);
  if ( ret < 0 ) {
    DBG("Arrow Gateway register failed... %d", count++);
  }
  return ret;
}

#define URI_LEN sizeof(ARROW_API_GATEWAY_ENDPOINT) + 50

static void _gateway_find_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s", ARROW_API_GATEWAY_ENDPOINT, hid);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _gateway_find_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("find [%s]", response->payload.buf);
  } else return -1;
  return 0;
}

int arrow_gateway_find(const char *hid) {
  int ret = __http_routine(_gateway_find_init, (void*)hid, _gateway_find_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway register failed...");
  }
  return ret;
}

static void _gateway_find_by_init(http_request_t *request, void *arg) {
  find_by_t *params = (find_by_t *)arg;
  http_request_init(request, GET, ARROW_API_GATEWAY_ENDPOINT);
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _gateway_find_by_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("gw find [%s]", response->payload.buf);
  } else return -1;
  return 0;
}


int arrow_gateway_find_by(int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  int ret = __http_routine(_gateway_find_by_init, params, _gateway_find_by_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway find by failed...");
  }
  return ret;
}

typedef struct _gate_param_ {
  arrow_gateway_t *gate;
  find_by_t *params;
} gate_param_t;

static void _gateway_list_logs_init(http_request_t *request, void *arg) {
  gate_param_t *dp = (gate_param_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN,"%s/%s/logs", ARROW_API_GATEWAY_ENDPOINT, dp->gate->hid);
  http_request_init(request, GET, uri);
  free(uri);
  find_by_t *params = dp->params;
  ADD_FIND_BY_TO_REQ(params, request);
}

static int _gateway_list_logs_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  DBG("gateway list logs: %s", response->payload.buf);
  return 0;
}

int arrow_gateway_logs_list(arrow_gateway_t *gateway, int n, ...) {
  find_by_t *params = NULL;
  COLLECT_FIND_BY(params, n);
  gate_param_t dp = { gateway, params };
  int ret = __http_routine(_gateway_list_logs_init, &dp, _gateway_list_logs_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway logs failed...");
  }
  return ret;
}

static void _gateway_devices_list_init(http_request_t *request, void *arg) {
  char *hid = (char *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/devices", ARROW_API_GATEWAY_ENDPOINT, hid);
  http_request_init(request, GET, uri);
  free(uri);
}

static int _gateway_devices_list_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("devices [%s]", response->payload.buf);
  } else return -1;
  return 0;
}

int arrow_gateway_devices_list(const char *hid) {
  int ret = __http_routine(_gateway_devices_list_init, (void*)hid, _gateway_devices_list_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway devices list failed...");
  }
  return ret;
}

typedef struct _gate_dev_cmd_ {
  const char *g_hid;
  const char *d_hid;
  const char *cmd;
  const char *payload;
} gate_dev_cmd_t;

static void _gateway_device_cmd_init(http_request_t *request, void *arg) {
  gate_dev_cmd_t *gdc = (gate_dev_cmd_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/devices/%s/actions/command", ARROW_API_GATEWAY_ENDPOINT,
           gdc->g_hid, gdc->d_hid);
  http_request_init(request, GET, uri);
  free(uri);
  JsonNode *_main = json_mkobject();
  json_append_member(_main, "command", json_mkstring(gdc->cmd));
  json_append_member(_main, "deviceHid", json_mkstring(gdc->d_hid));
  json_append_member(_main, "payload", json_mkstring(gdc->payload));
  char *str = json_encode(_main);
  json_delete(_main);
  http_request_set_payload(request, str);
  free(str);
}

static int _gateway_device_cmd_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode == 200 ) {
    DBG("devices [%s]", response->payload.buf);
  } else return -1;
  return 0;
}

int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload) {
  gate_dev_cmd_t gdc = {gHid, dHid, cmd, payload};
  int ret = __http_routine(_gateway_device_cmd_init, &gdc, _gateway_device_cmd_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway devices list failed...");
  }
  return ret;
}

typedef struct _gateway_error {
  arrow_gateway_t *gateway;
  const char *error;
} gateway_error_t;

static void _gateway_errors_init(http_request_t *request, void *arg) {
  gateway_error_t *de = (gateway_error_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s/errors", ARROW_API_GATEWAY_ENDPOINT, de->gateway->hid);
  http_request_init(request, POST, uri);
  free(uri);
  JsonNode *error = json_mkobject();
  json_append_member(error, "error", json_mkstring(de->error));
  char *_error_str = json_encode(error);
  http_request_set_payload(request, _error_str);
  free(_error_str);
  json_delete(error);
}

int arrow_gateway_error(arrow_gateway_t *gateway, const char *error) {
  gateway_error_t de = { gateway, error };
  int ret = __http_routine(_gateway_errors_init, &de, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Gateway   error failed...");
  }
  return ret;
}

static void _gateway_update_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gate = (arrow_gateway_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/%s", ARROW_API_DEVICE_ENDPOINT, gate->hid);
  http_request_init(request, PUT, uri);
  free(uri);
  char *payload = arrow_gateway_serialize(gate);
  http_request_set_payload(request, payload);
  DBG("dev|%s|", payload);
  free(payload);
}

static int _gateway_update_proc(http_response_t *response, void *arg) {
  SSP_PARAMETER_NOT_USED(arg);
  if ( response->m_httpResponseCode != 200 ) return -1;
  return 0;
}

int arrow_gateway_update(arrow_gateway_t *gateway) {
  int ret = __http_routine(_gateway_update_init, gateway, _gateway_update_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Device update failed...");
  }
  return ret;
}
