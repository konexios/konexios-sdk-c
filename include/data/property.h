/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ACN_SDK_C_PROPERTY_H_
#define _ACN_SDK_C_PROPERTY_H_

#include <sys/mem.h>

enum prop_flags {
  is_stack    = 0x0,
  is_dynamic  = 0x1,
  is_const    = 0x2
};

typedef struct __attribute_packed__ _property {
  char *value;
  uint8_t flags;
#if defined(__cplusplus)
  _property(const char *val, uint8_t f) : value(const_cast<char*>(val)), flags(f) {}
#endif
} property_t;

#if defined(__cplusplus)
# define property(x, y) _property(x, y)
#else
# define property(x, y) (property_t){ .value=(char*)x, .flags=y }
#endif

#define p_const(x)  property(x, is_const)
#define p_stack(x)  property(x, is_stack)
#define p_heap(x)   property(x, is_dynamic)
#define p_null()    property(NULL, 0)

void property_copy(property_t *dst, const property_t src);
void property_n_copy(property_t *dst, const char *src, int n);
void property_free(property_t *dst);
int property_cmp(property_t *src, property_t *dst);

#define P_COPY(dst, src) property_copy(&dst, src)
#define P_NCOPY(dst, str, n)  property_n_copy(&dst, str, n)
#define P_FREE(prop) property_free(&(prop))

#define IS_EMPTY(field) ( (field).value ? 0 : 1 )
#define P_VALUE(field) ( (field).value )
#define P_SIZE(field) ( (field).value ? strlen((field).value) : 0 )
#define P_CLEAR(field) memset(&(field), 0x0, sizeof(property_t))

#endif  // _ACN_SDK_C_PROPERTY_H_
