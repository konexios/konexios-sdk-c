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
#if defined(__USE_STD__)
# include <stddef.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
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

#endif  // _ARROW_KRONOS_C_SDK_MEM_H_
