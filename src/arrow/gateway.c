/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/gateway.h"
#include <debug.h>
#include <sys/mac.h>
#include <json/decode.h>

#if defined(__IBM__)
#elif defined(__AZURE__)
P_ADD(arrow_gateway_config, host);
P_ADD(arrow_gateway_config, accessKey);
#endif

void arrow_gateway_init(arrow_gateway_t *gate) {
  memset(gate, 0, sizeof(arrow_gateway_t));
}

#include <arrow/storage.h>

property_t arrow_gateway_serialize(arrow_gateway_t *gate) {
  JsonNode *_main = json_mkobject();
  if ( !IS_EMPTY( gate->name ) )
    json_append_member(_main, p_const("name"), json_mk_weak_property(gate->name));
  if ( !IS_EMPTY( gate->uid ) )
    json_append_member(_main, p_const("uid"), json_mk_weak_property(gate->uid));
  if ( !IS_EMPTY(gate->os) )
    json_append_member(_main, p_const("osName"), json_mk_weak_property(gate->os));
  if ( !IS_EMPTY(gate->type) )
    json_append_member(_main, p_const("type"), json_mk_weak_property(gate->type));
  if ( !IS_EMPTY(gate->software_name) )
    json_append_member(_main, p_const("softwareName"), json_mk_weak_property(gate->software_name));
  if ( !IS_EMPTY(gate->software_version) )
    json_append_member(_main, p_const("softwareVersion"), json_mk_weak_property(gate->software_version));
  if ( !IS_EMPTY(gate->sdkVersion) )
    json_append_member(_main, p_const("sdkVersion"), json_mk_weak_property(gate->sdkVersion));

#if defined(ARROW_HAS_USERHID)
  if ( !IS_EMPTY(gate->app) )
      json_append_member(_main, p_const("applicationHid"), json_mk_weak_property(gate->app));
  if ( !IS_EMPTY(gate->user) )
      json_append_member(_main, p_const("userHid"), json_mk_weak_property(gate->user));
#endif

  property_t tmp = json_encode_property(_main);

  json_delete(_main);
  return tmp;
}

int arrow_gateway_parse(arrow_gateway_t *gate, const char *s) {
  if (!s) return -1;
  DBG("parse this: %s", s);
  int ret = -1;
  JsonNode *_main = json_decode_property(p_stack(s));
  if ( !_main ) goto parse_error;
  JsonNode *hid = json_find_member(_main, p_const("hid"));
  if ( !hid || hid->tag != JSON_STRING ) goto parse_error;
  property_copy_as( PROPERTY_DYNAMIC_TAG, &gate->hid, hid->string_);
  ret = 0;
parse_error:
  json_delete(_main);
  return ret;
}

void arrow_gateway_free(arrow_gateway_t *gate) {
  property_free( &gate->name );
  property_free( &gate->uid );
  property_free( &gate->os );
  property_free( &gate->type );
  property_free( &gate->hid );
  property_free( &gate->software_name );
  property_free( &gate->software_version );
  property_free( &gate->sdkVersion );
#if defined(ARROW_HAS_USERHID)
  property_free( &gate->user );
  property_free( &gate->app );
#endif
}


void arrow_gateway_config_init(arrow_gateway_config_t *config) {
  memset(config, 0x00, sizeof(arrow_gateway_config_t));
  config->type = unknown;
}


void arrow_gateway_config_free(arrow_gateway_config_t *config) {
#if defined(__IBM__)
	property_free ( &config->authMethod );
	property_free ( &config->authToken );
	property_free ( &config->gatewayId );
	property_free ( &config->gatewayType );
	property_free ( &config->organizationId );
#else
  SSP_PARAMETER_NOT_USED(config);
#endif
}

#if defined(STATIC_ACN)
static char static_gateway_uid[GATEWAY_UID_SIZE];
#endif

int arrow_prepare_gateway(arrow_gateway_t *gateway) {
  if ( IS_EMPTY(gateway->name) )
      property_copy( &gateway->name, p_const(GATEWAY_NAME));
  if ( IS_EMPTY(gateway->os) )
      property_copy( &gateway->os, p_const(GATEWAY_OS));
  if ( IS_EMPTY(gateway->software_name) )
      property_copy( &gateway->software_name, p_const(GATEWAY_SOFTWARE_NAME));
  if ( IS_EMPTY(gateway->software_version) )
      property_copy( &gateway->software_version, p_const(GATEWAY_SOFTWARE_VERSION));
  if ( IS_EMPTY(gateway->type) )
      property_copy( &gateway->type, p_const(GATEWAY_TYPE));
  if ( IS_EMPTY(gateway->sdkVersion) )
      property_copy( &gateway->sdkVersion, p_const(xstr(SDK_VERSION)));
  #if defined(ARROW_HAS_USERHID)
  if ( IS_EMPTY(gateway->app) ) {
      char tmphid[46] = {0};
      if ( restore_apphid_address(tmphid) == 0 ) {
        property_copy( &gateway->app, p_stack(tmphid));
      } else {
          DBG("no application hid!");
      }
  }
  if ( IS_EMPTY(gateway->user) ) {
      char tmphid[46] = {0};
      if ( restore_userhid_address(tmphid) == 0 ) {
          property_copy( &gateway->user, p_stack(tmphid));
      } else {
          DBG("no user hid!");
      }
  }
#endif
  if ( IS_EMPTY(gateway->uid) ) {
#if defined(STATIC_ACN)
      char *uid = static_gateway_uid;
#else
      char *uid = (char*)malloc(GATEWAY_UID_SIZE); // 6*2 for mac + 2
#endif
      strcpy(uid, GATEWAY_UID_PREFIX);
      strcat(uid, "-");
      uint32_t uidlen = sizeof(GATEWAY_UID_PREFIX);
      char mac[7];
      get_mac_address(mac);
      int i = 0;
      for(i=0; i<6; i++) sprintf(uid+uidlen+2*i, "%02x", (uint8_t)(mac[i]));
      uidlen += 12;
      uid[uidlen] = '\0';
      DBG("uid: [%s]", uid);
#if defined(STATIC_ACN)
      property_t tmp = p_const(uid);
#else
      property_t tmp = p_heap(uid);
#endif
      property_move( &gateway->uid, &tmp);
  }
  return 0;
}
