/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ARROW_KRONOS_C_SDK_MEM_H_
#define _ARROW_KRONOS_C_SDK_MEM_H_

#include <config.h>
#include <unint.h>
#if defined(__USE_STD__)
# include <stddef.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <strings.h>
#endif
#if defined(__XCC__)
#include <qcom_common.h>
#include <malloc_api.h>
//#include <qcom_mem.h>
#include <qcom_utils.h>
void bzero(void *s, size_t n);
void bcopy(const void *src, void *dest, size_t n);
char *strcat( char *dest, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strncat(char *dest, const char *src, size_t n);
#if !defined(malloc_module_init)
# define malloc qcom_mem_alloc
# define free qcom_mem_free
void *realloc(void *ptrmem, size_t size);
#endif
#endif

#define X_STR_COPY(dst, src) \
{ (dst) = malloc(strlen(src) + 1); \
  strcpy((dst), (src)); }

#define X_STR_FREE(str) if ( str ) free(str);

enum prop_flags {
  is_stack = 0x0,
  is_dynamic = 0x1,
  is_const = 0x2
};

typedef struct _property {
  char *value;
  uint8_t flags;
#if defined(__cplusplus)
  _property(const char *val, uint8_t f) : value(const_cast<char*>(val)), flags(f) {}
#endif
} property_t;

#if defined(__cplusplus)
#define property(x, y) property_t(x, y)
#else
#define property(x, y) (property_t){ .value=(char*)x, .flags=y }
#endif

#define p_const(x) property(x, is_const)
#define p_stack(x) property(x, is_stack)

#define P_FREE(prop) \
{ if ((prop).flags == is_dynamic && (prop).value) free((prop).value); \
  (prop).value = NULL; \
}

#define P_ADD_PROTO(type, name) \
void type##_set_##name##_dup(type##_t *gate, const char *name); \
void type##_set_##field##_copy(type##_t *date, const char *field, int n); \
void type##_set_##name##_own(type##_t *gate, const char *name); \
void type##_set_##name(type##_t *gate, const char *name);

#define P_ADD(type, field) \
void type##_set_##field##_dup(type##_t *date, const char *field) { \
  P_FREE(date->field); \
  date->field.value = strdup(field); \
  date->field.flags = is_dynamic;\
} \
void type##_set_##field##_copy(type##_t *date, const char *field, int n) { \
  P_FREE(date->field); \
  date->field.value = (char*)malloc(n+1); \
  strncpy(date->field.value, field, n); \
  date->field.value[n] = 0x0; \
  date->field.flags = is_dynamic;\
} \
void type##_set_##field##_own(type##_t *date, const char *field) { \
  P_FREE(date->field); \
  date->field.value = (char *)field; \
  date->field.flags = is_dynamic;\
} \
void type##_set_##field(type##_t *date, const char *field) { \
  date->field.value = (char *)field; \
  date->field.flags = is_const;\
}

#define P_COPY(dst, src) \
{ \
  switch((src).flags) { \
    case is_stack:    (dst) = property(strdup((src).value), is_dynamic); break; \
    case is_dynamic:  (dst) = property((src).value, is_dynamic); break; \
    case is_const:    (dst) = property((src).value, is_const); break; \
  } \
}

#define IS_EMPTY(field) ( (field).value ? 1 : 0 )
#define P_VALUE(field) ( (field).value )

#endif  // _ARROW_KRONOS_C_SDK_MEM_H_
