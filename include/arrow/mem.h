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
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if GCC_VERSION <= 50201
# include <sys/cdefs.h>
#endif
#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 17 && __GLIBC_MINOR__ < 23
# include <features.h>
#else
# include <sys/features.h>
#endif
# include <stddef.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <strings.h>
#endif
#if defined(__silex__)
#include <qcom_common.h>
#include <malloc_api.h>
//#include <qcom_mem.h>
#include <qcom_utils.h>
typedef int (*__compar_fn_t) (const void *, const void *);
extern void qsort(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
#define bzero(s, n) A_MEMSET(s, 0, n)
#define bcopy(src, dest, n) A_MEMCPY(dest, src, n)
char *strcat( char *dest, const char *src);
//char *strncpy(char *dst, const char *src, size_t n);
char *strncat(char *dest, const char *src, size_t n);
double strtod (const char* str, char** endptr);
# if !defined(malloc_module_init)
#  define malloc qcom_mem_alloc
#  define free qcom_mem_free
void *realloc(void *ptrmem, size_t size);
# endif
#endif

enum prop_flags {
  is_stack    = 0x0,
  is_dynamic  = 0x1,
  is_const    = 0x2
};
#if defined(__xtensa__) || defined(__XCC__)
# define __attribute_packed__
#else
# define __attribute_packed__ __attribute__((__packed__))
#endif

typedef struct __attribute_packed__ _property {
  char *value;
  uint8_t flags;
#if defined(__cplusplus)
  _property(const char *val, uint8_t f) : value(const_cast<char*>(val)), flags(f) {}
#endif
} property_t;

#if defined(__cplusplus)
#define property(x, y) _property(x, y)
#else
#define property(x, y) (property_t){ .value=(char*)x, .flags=y }
#endif

#define p_const(x) property(x, is_const)
#define p_stack(x) property(x, is_stack)
#define p_heap(x)  property(x, is_dynamic)
#define p_null()  property(NULL, 0)

void property_copy(property_t *dst, const property_t src);
void property_n_copy(property_t *dst, const char *src, int n);
void property_free(property_t *dst);

#define P_COPY(dst, src) property_copy(&dst, src)
#define P_NCOPY(dst, str, n)  property_n_copy(&dst, str, n)
#define P_FREE(prop) property_free(&(prop))

#define IS_EMPTY(field) ( (field).value ? 0 : 1 )
#define P_VALUE(field) ( (field).value )
#define P_SIZE(field) ( (field).value ? strlen((field).value) : 0 )
#define P_CLEAR(field) memset(&(field), 0x0, sizeof(property_t))

#endif  // _ARROW_KRONOS_C_SDK_MEM_H_
