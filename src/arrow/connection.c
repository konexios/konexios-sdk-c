#include "arrow/connection.h"
#include <config.h>
#include <debug.h>
#include <http/client.h>
#include <json/json.h>
#include <json/telemetry.h>
#include <arrow/net.h>
#include <arrow/storage.h>
#include <arrow/mem.h>

int arrow_prepare_gateway(arrow_gateway_t *gateway) {
  arrow_gateway_init(gateway);
  arrow_gateway_add_name(gateway, GATEWAY_NAME);
  arrow_gateway_add_os(gateway, GATEWAY_OS);
  arrow_gateway_add_software_name(gateway, GATEWAY_SOFTWARE_NAME);
  arrow_gateway_add_software_version(gateway, GATEWAY_SOFTWARE_VERSION);
  arrow_gateway_add_type(gateway, GATEWAY_TYPE);
  char *uid = (char*)malloc(strlen(GATEWAY_UID_PREFIX) + 20);
  strcpy(uid, GATEWAY_UID_PREFIX);
  strcat(uid, "-");
  size_t uidlen = strlen(uid);
  char mac[7];
  get_mac_address(mac);
  int i;
  for(i=0; i<6; i++) sprintf(uid+uidlen+2*i, "%02x", (uint8_t)(mac[i]));
  uidlen += 12;
  uid[uidlen] = '\0';
  DBG("uid: [%s]", uid);
  arrow_gateway_add_uid(gateway, uid);
  free(uid);
  return 0;
}

int arrow_register_gateway(arrow_gateway_t *gateway) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;

    http_client_init(&cli);
    http_request_init(&request, POST, ARROW_API_GATEWAY_ENDPOINT);

    char *payload = arrow_gateway_serialize(gateway);
    DBG("payload %s", payload);

    http_request_set_payload(&request, payload);
    sign_request(&request);
    http_client_do(&cli, &request, &response);

    free(payload);

    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);

    http_client_free(&cli);

    if ( arrow_gateway_parse(gateway, (char*)response.payload.buf) < 0 ) {
        DBG("parse error");
        http_response_free(&response);
        return -1;
    } else {
        DBG("gateway hid: %s", gateway->hid);
    }
    http_response_free(&response);
    return 0;
}

int arrow_prepare_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_device_init(device);
  arrow_device_add_gateway_hid(device, gateway->hid);
  arrow_device_add_name(device, DEVICE_NAME);
  arrow_device_add_type(device, DEVICE_TYPE);
//    FIXME info property extra param?
//    arrow_device_add_info(device, "info1", "value1");
//    arrow_device_add_info(device, "info2", "value2");
//    arrow_device_add_property(device, "prop1", "value1");
//    arrow_device_add_property(device, "prop2", "value2");
  if ( !gateway->uid ) return -1;
  char *uid = (char*)malloc(strlen(gateway->uid)+strlen(DEVICE_UID_SUFFIX)+2);
  strcpy(uid, gateway->uid);
  strcat(uid, "-");
  strcat(uid, DEVICE_UID_SUFFIX);
  arrow_device_add_uid(device, uid);
  free(uid);
  return 0;
}

int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;

    http_client_init(&cli);
    http_request_init(&request, POST, ARROW_API_DEVICE_ENDPOINT);
    arrow_prepare_device(gateway, device);

    char *payload = arrow_device_serialize(device);
    http_request_set_payload(&request, payload);
    DBG("dev|%s|", payload);
    free(payload);
    sign_request(&request);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);

    http_client_free(&cli);
    if ( arrow_device_parse(device, (char*)response.payload.buf) < 0) {
        DBG("device parse error");
        http_response_free(&response);
        return -1;
    } else {
        DBG("device hid: %s", device->hid);
    }
    http_response_free(&response);
    return 0;
}

int arrow_heartbeat(arrow_gateway_t *gateway) {
  http_client_t cli;
  http_response_t response;
  http_request_t request;

  char *uri = (char *)malloc(strlen(ARROW_API_GATEWAY_ENDPOINT) + 50);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, gateway->hid);
  strcat(uri, "/heartbeat");

  http_client_init(&cli);
  http_request_init(&request, PUT, uri);

  sign_request(&request);
  http_client_do(&cli, &request, &response);
  http_request_close(&request);
  DBG("response %d", response.m_httpResponseCode);
  http_client_free(&cli);
  if ( response.m_httpResponseCode != 200 ) {
    http_response_free(&response);
    return -1;
  }
  http_response_free(&response);
  return 0;
}

int arrow_checkin(arrow_gateway_t *gateway) {
  http_client_t cli;
  http_response_t response;
  http_request_t request;

  char *uri = (char *)malloc(strlen(ARROW_API_GATEWAY_ENDPOINT) + 50);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, gateway->hid);
  strcat(uri, "/checkin");

  http_client_init(&cli);
  http_request_init(&request, PUT, uri);

  sign_request(&request);
  http_client_do(&cli, &request, &response);
  http_request_close(&request);
  DBG("response %d", response.m_httpResponseCode);
  http_client_free(&cli);
  if ( response.m_httpResponseCode != 200 ) {
    http_response_free(&response);
    return -1;
  }
  http_response_free(&response);
  return 0;
}

int arrow_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config) {
  http_client_t cli;
  http_response_t response;
  http_request_t request;

  char *uri = (char *)malloc(strlen(ARROW_API_GATEWAY_ENDPOINT) + strlen(gateway->hid) + 20);
  strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
  strcat(uri, "/");
  strcat(uri, gateway->hid);
  strcat(uri, "/config");

  http_client_init(&cli);
  http_request_init(&request, GET, uri);

  sign_request(&request);

  http_client_do(&cli, &request, &response);
  http_request_close(&request);
  DBG("response %d", response.m_httpResponseCode);
  http_client_free(&cli);
  if ( response.m_httpResponseCode != 200 ) {
    http_response_free(&response);
    return -1;
  }
  DBG("pay: {%s}\r\n", response.payload.buf);

  JsonNode *_main = json_decode((char*)response.payload.buf);
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
  http_response_free(&response);
  return 0;
}


int arrow_send_telemetry(arrow_device_t *device, void *d) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    sensor_data_t *data = (sensor_data_t*)d;

    http_client_init(&cli);
    http_request_init(&request, POST, ARROW_API_TELEMETRY_ENDPOINT);
    request.is_chunked = 1;
    char *tmp = telemetry_serialize(device, data);
    DBG("set payload %s", tmp);
    http_request_set_payload(&request, tmp);
    free(tmp);
    sign_request(&request);
    http_client_do(&cli, &request, &response);
    DBG("response %d", response.m_httpResponseCode);
    http_request_close(&request);
    http_client_free(&cli);
    http_response_free(&response);
    return 0;
}

int arrow_connect_gateway(arrow_gateway_t *gateway){
  arrow_prepare_gateway(gateway);
  int ret = restore_gateway_info(gateway);
  if ( ret < 0 ) {
    // new registration
    if ( arrow_register_gateway(gateway) < 0 ) {
      return -1;
    }
    save_gateway_info(gateway);
  } else {
    // hid already set
    DBG("gateway checkin hid %s", gateway->hid);
    return arrow_checkin(gateway);
  }
  return 0;
}

int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_device_init(device);
  arrow_prepare_device(gateway, device);
  if ( restore_device_info(device) < 0 ) {
    if ( arrow_register_device(gateway, device) < 0 ) return -1;
    save_device_info(device);
  }
  return 0;
}
