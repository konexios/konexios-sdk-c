/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_GATEWAY_H_
#define ARROW_GATEWAY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <data/property.h>
#include <json/json.h>

#define GATEWAY_UID_SIZE sizeof(GATEWAY_UID_PREFIX) + 14

typedef struct __attribute_packed__ _konexios_gateway_ {
  property_t name;
  property_t uid;
  property_t os;
  property_t type;
  property_t hid;
  property_t software_name;
  property_t software_version;
  property_t sdkVersion;
#if defined(ARROW_HAS_USERHID)
  property_t app;
  property_t user;
#endif
#if defined(__cplusplus)
  _konexios_gateway_() {}
#endif
} konexios_gateway_t;

enum account_type {
  unknown = 0,
  IoT,
  IBM,
  AWS,
  Azure
};

typedef struct __attribute_packed__ {
  int type;    // enum account_type
#if defined(__IBM__)
  property_t organizationId;
  property_t authMethod;
  property_t authToken;
  property_t gatewayId;
  property_t gatewayType;
#elif defined(__AZURE__)
  property_t host;
  property_t accessKey;
#else
#endif
} konexios_gateway_config_t;

void konexios_gateway_init(konexios_gateway_t *gate);
property_t konexios_gateway_serialize(konexios_gateway_t *gate);
int konexios_gateway_parse(konexios_gateway_t *gate, const char *str);
void konexios_gateway_free(konexios_gateway_t *gate);

void konexios_gateway_config_init(konexios_gateway_config_t *config);
void konexios_gateway_config_free(konexios_gateway_config_t *config);
#if defined(__IBM__)
void konexios_gateway_config_add_authMethod(konexios_gateway_config_t *conf, const char*authMeth);
void konexios_gateway_config_add_authToken(konexios_gateway_config_t *conf, const char*authToken);
void konexios_gateway_config_add_gatewayId(konexios_gateway_config_t *conf, const char*gID);
void konexios_gateway_config_add_gatewayType(konexios_gateway_config_t *conf, const char*gT);
void konexios_gateway_config_add_organizationId(konexios_gateway_config_t *conf, const char*oID);
#elif defined(__AZURE__)
void konexios_gateway_config_add_host(konexios_gateway_config_t *conf, const char *host);
void konexios_gateway_config_add_accessKey(konexios_gateway_config_t *conf, const char *accessKey);
#endif

int konexios_prepare_gateway(konexios_gateway_t *gateway);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_GATEWAY_H_ */
