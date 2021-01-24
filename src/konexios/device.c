/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "konexios/device.h"
#include <sys/mem.h>
#include <konexios_config.h>
#include <json/decode.h>

void konexios_device_init(konexios_device_t *dev) {
    dev->info = NULL;
    dev->prop = NULL;
    property_init(&dev->name);
    property_init(&dev->type);
    property_init(&dev->uid);
    property_init(&dev->gateway_hid);
    property_init(&dev->hid);
    property_init(&dev->softwareName);
    property_init(&dev->softwareVersion);
#if defined(KONEXIOS_HAS_USERHID)
    property_init(&dev->app);
    property_init(&dev->user);
#endif
#if defined(__IBM__)
    property_init(&dev->eid);
#endif
}

void konexios_device_free(konexios_device_t *dev) {
  property_free(&dev->name);
  property_free(&dev->type);
  property_free(&dev->uid);
  property_free(&dev->gateway_hid);
  property_free(&dev->hid);
  property_free(&dev->softwareName);
  property_free(&dev->softwareVersion);
#if defined(KONEXIOS_HAS_USERHID)
  property_free(&dev->app);
  property_free(&dev->user);
#endif
  if ( dev->info ) json_delete(dev->info);
  if ( dev->prop ) json_delete(dev->prop);
#if defined(__IBM__)
  P_FREE(dev->eid);
#endif
}

void konexios_device_add_info(konexios_device_t *dev, property_t key, const char *value) {
  if ( !dev->info) dev->info = json_mkobject();
  json_append_member(dev->info, key, json_mkstring(value));
}

void konexios_device_add_property(konexios_device_t *dev, property_t key, const char *value) {
  if ( !dev->prop ) dev->prop = json_mkobject();
  json_append_member(dev->prop, key, json_mkstring(value));
}

property_t konexios_device_serialize(konexios_device_t *dev) {
  JsonNode *_main = json_mkobject();
  if ( !IS_EMPTY(dev->name) )
      json_append_member(_main, p_const("name"), json_mk_weak_property(dev->name));
  if ( !IS_EMPTY(dev->type) )
      json_append_member(_main, p_const("type"), json_mk_weak_property(dev->type));
  if ( !IS_EMPTY(dev->uid) )
      json_append_member(_main, p_const("uid"), json_mk_weak_property(dev->uid));
  if ( !IS_EMPTY(dev->gateway_hid) )
      json_append_member(_main, p_const("gatewayHid"), json_mk_weak_property(dev->gateway_hid));
  if ( !IS_EMPTY(dev->softwareName) )
      json_append_member(_main, p_const("softwareName"), json_mk_weak_property(dev->softwareName));
  if ( !IS_EMPTY(dev->softwareVersion) )
      json_append_member(_main, p_const("softwareVersion"), json_mk_weak_property(dev->softwareVersion));

#if defined(KONEXIOS_HAS_USERHID)
  if ( !IS_EMPTY(dev->app) )
      json_append_member(_main, p_const("applicationHid"), json_mk_weak_property(dev->app));
  if ( !IS_EMPTY(dev->user) )
      json_append_member(_main, p_const("userHid"), json_mk_weak_property(dev->user));
#endif

  if ( dev->info ) json_append_member(_main, p_const("info"), dev->info);
  if ( dev->prop ) json_append_member(_main, p_const("properties"), dev->prop);
  property_t dev_property = json_encode_property(_main);
  if ( dev->info ) json_remove_from(_main, dev->info);
  if ( dev->prop ) json_remove_from(_main, dev->prop);
  json_delete(_main);
  return dev_property;
}

int konexios_device_parse(konexios_device_t *dev, const char *s) {
    JsonNode *_main = json_decode_property(p_stack(s));
    if ( !_main ) return -1;
    JsonNode *hid = json_find_member(_main, p_const("hid"));
    if ( !hid || hid->tag != JSON_STRING ) return -1;
    property_copy_as(PROPERTY_DYNAMIC_TAG, &dev->hid, hid->string_);
#if defined(__IBM__)
    JsonNode *eid = json_find_member(_main, p_const("externalId"));
    if ( !eid || eid->tag != JSON_STRING ) return -1;
    property_copy(&dev->eid, p_stack(eid->string_));
#endif
    json_delete(_main);
    return 0;
}

#if defined(STATIC_ACN)
static char static_device_uid[GATEWAY_UID_SIZE + sizeof(DEVICE_UID_SUFFIX)+2];
#endif

int konexios_prepare_device(konexios_gateway_t *gateway, konexios_device_t *device) {
  if ( IS_EMPTY(device->gateway_hid) )
      property_weak_copy(&device->gateway_hid, gateway->hid );

#if defined(DEBUG_NJOHNSON)
#else
  if ( IS_EMPTY(device->name) )
      property_copy(&device->name, p_const(DEVICE_NAME));
#endif

  if ( IS_EMPTY(device->type) )
      property_copy(&device->type, p_const(DEVICE_TYPE));
  if ( IS_EMPTY(device->softwareName) )
      property_copy(&device->softwareName, p_const(DEVICE_SOFTWARE_NAME));
  if ( IS_EMPTY(device->softwareVersion) )
      property_copy(&device->softwareVersion, p_const(DEVICE_SOFTWARE_VERSION));
  if ( IS_EMPTY(gateway->uid) ) return -1;

#if defined(KONEXIOS_HAS_USERHID)
  if ( IS_EMPTY(device->user) && !IS_EMPTY(gateway->user) ) {
      property_weak_copy(&device->user, gateway->user);
  }
  if ( IS_EMPTY(device->app) && !IS_EMPTY(gateway->app) ) {
      property_weak_copy(&device->app, gateway->app);
  }
#endif

  if ( IS_EMPTY(device->uid) ) {
#if defined(STATIC_ACN)
      char *uid = static_device_uid;
#else
      char *uid = (char*)malloc(P_SIZE(gateway->uid)+sizeof(DEVICE_UID_SUFFIX)+2);
#endif
      strcpy(uid, P_VALUE(gateway->uid) );
      strcat(uid, "-");
      strcat(uid, DEVICE_UID_SUFFIX);
#if defined(STATIC_ACN)
      property_t tmp = p_const(uid);
#else
      property_t tmp = p_heap(uid);
#endif
      property_move(&device->uid, &tmp);
  }
  return 0;
}
