/*
 * gateway.h
 *
 *  Created on: 17 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef ARROW_GATEWAY_H_
#define ARROW_GATEWAY_H_

#include <arrow/mem.h>
#include <config.h>
#include <json/json.h>

typedef struct {
  char *name;
  char *uid;
  char *os;
  char *type;
  char *hid;
  char *software_name;
  char *software_version;
  char *sdkVersion;
} arrow_gateway_t;

enum account_type {
  unknown = 0,
  IoT,
  IBM,
  AWS,
  Azure
};

typedef struct {
  int type;    // enum account_type
#if defined(__IBM__)
  char *organizationId;
  char *authMethod;
  char *authToken;
  char *gatewayId;
  char *gatewayType;
#elif defined(__AZURE__)
  char *host;
  char *accessKey;
#else
#endif
} arrow_gateway_config_t;

void arrow_gateway_init(arrow_gateway_t *gate);
void arrow_gateway_add_name(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_uid(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_os(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_type(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_software_name(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_software_version(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_hid(arrow_gateway_t *gate, const char *name);
void arrow_gateway_add_sdkVersion(arrow_gateway_t *gate, const char *name);

char *arrow_gateway_serialize(arrow_gateway_t *gate);
int arrow_gateway_parse(arrow_gateway_t *gate, const char *str);
void arrow_gateway_free(arrow_gateway_t *gate);

void arrow_gateway_config_init(arrow_gateway_config_t *config);
void arrow_gateway_config_free(arrow_gateway_config_t *config);
#if defined(__IBM__)
void arrow_gateway_config_add_authMethod(arrow_gateway_config_t *conf, const char*authMeth);
void arrow_gateway_config_add_authToken(arrow_gateway_config_t *conf, const char*authToken);
void arrow_gateway_config_add_gatewayId(arrow_gateway_config_t *conf, const char*gID);
void arrow_gateway_config_add_gatewayType(arrow_gateway_config_t *conf, const char*gT);
void arrow_gateway_config_add_organizationId(arrow_gateway_config_t *conf, const char*oID);
#elif defined(__AZURE__)
void arrow_gateway_config_add_host(arrow_gateway_config_t *conf, const char *host);
void arrow_gateway_config_add_accessKey(arrow_gateway_config_t *conf, const char *accessKey);
#endif
#endif /* ARROW_GATEWAY_H_ */
