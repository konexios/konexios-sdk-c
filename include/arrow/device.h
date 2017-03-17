/*
 * device.h
 *
 *  Created on: 17 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef ARROW_DEVICE_H_
#define ARROW_DEVICE_H_

#include "json/json.h"

typedef struct {
#if defined(__XCC__)
  struct json_t  *main;
  struct json_t *info;
  struct json_t *prop;
#else
    JsonNode *main;
    JsonNode *info;
    JsonNode *prop;
#endif
    char *hid;
    char *eid;
} arrow_device_t;


void arrow_device_init(arrow_device_t *dev);
void arrow_device_free(arrow_device_t *dev);

char *arrow_device_get_type(arrow_device_t *dev);
char *arrow_device_get_uid(arrow_device_t *dev);
char *arrow_device_get_name(arrow_device_t *dev);

void arrow_device_add_name(arrow_device_t *dev, const char *name);
void arrow_device_add_gateway_hid(arrow_device_t *dev, const char *name);
void arrow_device_add_type(arrow_device_t *dev, const char *name);
void arrow_device_add_uid(arrow_device_t *dev, const char *name);

void arrow_device_set_hid(arrow_device_t *dev, const char *hid);
void arrow_device_set_eid(arrow_device_t *dev, const char *eid);
void arrow_device_add_info(arrow_device_t *dev, const char *key, const char *value);
void arrow_device_add_property(arrow_device_t *dev, const char *key, const char *value);

char *arrow_device_serialize(arrow_device_t *dev);
int arrow_device_parse(arrow_device_t *dev, const char *str);

#endif /* ARROW_DEVICE_H_ */
