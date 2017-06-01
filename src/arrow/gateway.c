/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/gateway.h"
#include <unint.h>
#if defined(__USE_STD__)
#include <stdlib.h>
#endif
#include <debug.h>

#define GATEWAY_ADD_PROPERTY(type, name) \
void type##_add_##name(type##_t *gate, const char *name) { \
  if ( gate->name ) free(gate->name); \
  gate->name = strdup(name); \
}

GATEWAY_ADD_PROPERTY(arrow_gateway, name)
GATEWAY_ADD_PROPERTY(arrow_gateway, uid)
GATEWAY_ADD_PROPERTY(arrow_gateway, os)
GATEWAY_ADD_PROPERTY(arrow_gateway, type)
GATEWAY_ADD_PROPERTY(arrow_gateway, software_name)
GATEWAY_ADD_PROPERTY(arrow_gateway, software_version)
GATEWAY_ADD_PROPERTY(arrow_gateway, sdkVersion)
GATEWAY_ADD_PROPERTY(arrow_gateway, hid)

#if defined(__IBM__)
GATEWAY_ADD_PROPERTY(arrow_gateway_config, organizationId);
GATEWAY_ADD_PROPERTY(arrow_gateway_config, authMethod);
GATEWAY_ADD_PROPERTY(arrow_gateway_config, authToken);
GATEWAY_ADD_PROPERTY(arrow_gateway_config, gatewayId);
GATEWAY_ADD_PROPERTY(arrow_gateway_config, gatewayType);
#elif defined(__AZURE__)
GATEWAY_ADD_PROPERTY(arrow_gateway_config, host);
GATEWAY_ADD_PROPERTY(arrow_gateway_config, accessKey);
#endif

void arrow_gateway_init(arrow_gateway_t *gate) {
  memset(gate, 0, sizeof(arrow_gateway_t));
}

char *arrow_gateway_serialize(arrow_gateway_t *gate) {
  JsonNode *_main = json_mkobject();
  if ( gate->name )
    json_append_member(_main, "name", json_mkstring(gate->name));
  if ( gate->uid )
    json_append_member(_main, "uid", json_mkstring(gate->uid));
  if ( gate->os )
    json_append_member(_main, "osName", json_mkstring(gate->os));
  if ( gate->type )
    json_append_member(_main, "type", json_mkstring(gate->type));
  if ( gate->software_name )
    json_append_member(_main, "softwareName", json_mkstring(gate->software_name));
  if ( gate->software_version )
    json_append_member(_main, "softwareVersion", json_mkstring(gate->software_version));
  if ( gate->sdkVersion )
    json_append_member(_main, "sdkVersion", json_mkstring(gate->sdkVersion));
  char *str = json_encode(_main);
  json_minify(str);
  json_delete(_main);
  return str;
}

int arrow_gateway_parse(arrow_gateway_t *gate, const char *str) {
  if (!str) return -1;
  JsonNode *_main = json_decode(str);
  if ( !_main ) return -1;
  JsonNode *hid = json_find_member(_main, "hid");
  if ( !hid ) return -1;
  if ( hid->tag != JSON_STRING ) return -1;
  arrow_gateway_add_hid(gate, hid->string_);
  json_delete(_main);
  return 0;
}

#define FREE_PROP(prop) if (prop) free(prop)

void arrow_gateway_free(arrow_gateway_t *gate) {
  FREE_PROP( gate->name );
  FREE_PROP( gate->uid );
  FREE_PROP( gate->os );
  FREE_PROP( gate->type );
  FREE_PROP( gate->hid );
  FREE_PROP( gate->software_name );
  FREE_PROP( gate->software_version );
  FREE_PROP( gate->sdkVersion );
}


void arrow_gateway_config_init(arrow_gateway_config_t *config) {
  memset(config, 0x00, sizeof(arrow_gateway_config_t));
  config->type = unknown;
}


void arrow_gateway_config_free(arrow_gateway_config_t *config) {
#if defined(__IBM__)
  FREE_PROP ( config->authMethod );
  FREE_PROP ( config->authToken );
  FREE_PROP ( config->gatewayId );
  FREE_PROP ( config->gatewayType );
  FREE_PROP ( config->organizationId );
#else
  SSP_PARAMETER_NOT_USED(config);
#endif
}
