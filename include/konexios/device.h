/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_DEVICE_H_
#define ARROW_DEVICE_H_

#include "json/json.h"
#include <sys/mem.h>
#include <konexios/gateway.h>

typedef struct __attribute_packed__ {
  property_t name;
  property_t type;
  property_t uid;
  property_t gateway_hid;
  property_t softwareName;
  property_t softwareVersion;
  JsonNode *info;
  JsonNode *prop;
  property_t hid;
#if defined(ARROW_HAS_USERHID)
  property_t app;
  property_t user;
#endif
#if defined(__IBM__)
    property_t eid;
#endif
} konexios_device_t;


void konexios_device_init(konexios_device_t *dev);
void konexios_device_free(konexios_device_t *dev);

char *konexios_device_get_uid(konexios_device_t *dev);
char *konexios_device_get_gateway_hid(konexios_device_t *dev);

void konexios_device_add_gateway_hid(konexios_device_t *dev, const char *name);
void konexios_device_add_uid(konexios_device_t *dev, const char *name);

void konexios_device_add_info(konexios_device_t *dev, property_t key, const char *value);
void konexios_device_add_property(konexios_device_t *dev, property_t key, const char *value);

property_t konexios_device_serialize(konexios_device_t *dev);
int konexios_device_parse(konexios_device_t *dev, const char *str);

int konexios_prepare_device(konexios_gateway_t *gateway, konexios_device_t *device);

#endif /* ARROW_DEVICE_H_ */
