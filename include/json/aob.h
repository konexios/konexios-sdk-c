/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_ACN_SDK_C_JSON_AOB_H_
#define ARROW_ACN_SDK_C_JSON_AOB_H_

#include <sys/mem.h>
#include <json/sb.h>
#include <data/property.h>

typedef struct alloc_only {
    void *start;
    uint16_t size;
    uint16_t len;
    uint16_t offset;
} alloc_only_t;

void alloc_only_memory_set(alloc_only_t *p, void *start, int len);
int alloc_only_init(alloc_only_t *p);
int alloc_only_size(alloc_only_t *p);
int alloc_only_put(alloc_only_t *p, char c);
int alloc_only_puts(alloc_only_t *p, char *s, int len);
void alloc_only_clear(alloc_only_t *p);
void *alloc_only_finish(alloc_only_t *p);
property_t alloc_only_finish_property(alloc_only_t *p);



#include <data/property_base.h>

#define PROPERTY_AOB_TAG 5

#define p_aob(x)  property((x), PROPERTY_AOB_TAG | is_owner, strlen((char *)(x)))
#define p_aob_raw(x, len)  property((x), PROPERTY_AOB_TAG | is_owner | is_raw, (len))

property_dispetcher_t *property_type_get_aob();

#endif
