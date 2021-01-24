/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ACN_SDK_C_PROPERTY_BASE_H_
#define _ACN_SDK_C_PROPERTY_BASE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/mem.h>
#include <data/linkedlist.h>

enum prop_flags {
  is_raw    =  0x1 << 6,
  is_owner  = 0x1 << 7
};

#define PROPERTY_BASE_MASK 0x3f

typedef struct __attribute_packed__ _property {
  char *value;
  uint8_t flags;
  uint16_t size;
} property_t;

#if defined(__cplusplus)
property_t property(char *v, uint8_t f, uint16_t s);
#else
# define property_nt(x, y) (property_t){ .value=(char*)x, .flags=y, .size=strlen((char*)x) }
# define property(x, y, z) (property_t){ .value=(char*)x, .flags=(y), .size=(z) }
#endif

#define p_null property(NULL, 0, 0)
#define p_static_null { NULL, 0, 0 }

typedef struct _property_handler_ {
    void (*copy)(property_t *dst, property_t *src);
    void (*weak)(property_t *dst, property_t *src);
    void (*move)(property_t *dst, property_t *src);
    void (*destroy)(property_t *dst);
} property_handler_t;

typedef struct _property_dispetcher_ {
    uint8_t index;
    property_handler_t handler;
    konexios_linked_list_head_node;
} property_dispetcher_t;

#if defined(__cplusplus)
}
#endif

#endif  // _ACN_SDK_C_PROPERTY_H_
