/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/device.h"
#include <arrow/mem.h>
#include <config.h>
#if defined(__USE_STD__)
# include <stdlib.h>
#endif

void arrow_device_init(arrow_device_t *dev) {
    dev->main = json_mkobject();
    dev->info = json_mkobject();
    json_append_member(dev->main, "info", dev->info);
    dev->prop = json_mkobject();
    json_append_member(dev->main, "properties", dev->prop);
    P_CLEAR(dev->hid);
#if defined(__IBM__)
    P_CLEAR(dev->eid);
#endif
}

void arrow_device_free(arrow_device_t *dev) {
  json_delete(dev->main);
  P_FREE(dev->hid);
#if defined(__IBM__)
  P_FREE(dev->eid);
#endif
}

#define DEVICE_ADD_PROPERTY(key, name) \
    void arrow_device_add_##name(arrow_device_t *dev, const char *name) { \
	    json_append_member(dev->main, key, json_mkstring(name)); \
  } \
  char *arrow_device_get_##name(arrow_device_t *dev) { \
    JsonNode *_node = json_find_member(dev->main, key); \
    if ( _node ) { \
      return _node->string_; \
    } \
    return NULL; \
  }

#define DEVICE_EXT_PROPERTY(name) \
  void arrow_device_set_##name(arrow_device_t *dev, const char *name) { \
      if ( dev->name ) \
          dev->name = (char*)realloc(dev->name, strlen(name)+1); \
        else \
          dev->name = (char*)malloc(strlen(name)+1); \
        strcpy(dev->name, name); \
  }


DEVICE_ADD_PROPERTY("name", name)
DEVICE_ADD_PROPERTY("gatewayHid", gateway_hid)
DEVICE_ADD_PROPERTY("type", type)
DEVICE_ADD_PROPERTY("uid", uid)

P_ADD(arrow_device, hid)
#if defined(__IBM__)
P_ADD(arrow_device, eid)
#endif

void arrow_device_add_info(arrow_device_t *dev, const char *key, const char *value) {
    json_append_member(dev->info, key, json_mkstring(value));
}

void arrow_device_add_property(arrow_device_t *dev, const char *key, const char *value) {
    json_append_member(dev->prop, key, json_mkstring(value));
}

char *arrow_device_serialize(arrow_device_t *dev) {
    char *dev_str = json_encode(dev->main);
    json_minify(dev_str);
    return dev_str;
}

int arrow_device_parse(arrow_device_t *dev, const char *str) {
    JsonNode *_main = json_decode(str);
    if ( !_main ) return -1;
    JsonNode *hid = json_find_member(_main, "hid");
    if ( !hid || hid->tag != JSON_STRING ) return -1;
    arrow_device_set_hid_dup(dev, hid->string_);
#if defined(__IBM__)
    JsonNode *eid = json_find_member(_main, "externalId");
    if ( !eid || eid->tag != JSON_STRING ) return -1;
    arrow_device_set_eid_dup(dev, eid->string_);
#endif
    json_delete(_main);
    return 0;
}

int arrow_prepare_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_device_init(device);
  arrow_device_add_gateway_hid(device, P_VALUE(gateway->hid) );
  arrow_device_add_name(device, DEVICE_NAME);
  arrow_device_add_type(device, DEVICE_TYPE);
//    FIXME info property extra param?
//    arrow_device_add_info(device, "info1", "value1");
//    arrow_device_add_info(device, "info2", "value2");
//    arrow_device_add_property(device, "prop1", "value1");
//    arrow_device_add_property(device, "prop2", "value2");
  if ( IS_EMPTY(gateway->uid) ) return -1;
  char *uid = (char*)malloc(strlen(P_VALUE(gateway->uid))+sizeof(DEVICE_UID_SUFFIX)+2);
  strcpy(uid, P_VALUE(gateway->uid) );
  strcat(uid, "-");
  strcat(uid, DEVICE_UID_SUFFIX);
  arrow_device_add_uid(device, uid);
  free(uid);
  return 0;
}
