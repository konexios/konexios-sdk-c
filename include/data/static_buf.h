/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ARROW_C_SDK_STATIC_BUF_H_
#define _ARROW_C_SDK_STATIC_BUF_H_

#include <sys/mem.h>

#define ALLOC_BUF_CHUNK 0x20

void *__static_alloc(uint8_t *__alloc_head, uint8_t *__alloc_space, uint8_t *buffer, uint32_t _buf_size, int size);
void  __static_free(uint8_t *__alloc_head, uint8_t *__alloc_space, uint8_t *buffer, void *ptr);
void *__static_realloc(uint8_t *__alloc_head, uint8_t *__alloc_space, uint8_t *buffer, uint32_t _buf_size, void *ptr, int size);

#define CREATE_BUFFER(name, size) \
static uint32_t __##name##_size = size; \
static uint8_t name[ALLOC_BUF_CHUNK * size] = {0}; \
static uint8_t __alloc_space_##name[size>>3] = {0}; \
static uint8_t __alloc_head_##name[size>>3] = {0};

#define static_buf_alloc(name, size) __static_alloc(__alloc_head_##name, __alloc_space_##name, name, __##name##_size, size)
#define static_buf_realloc(name, ptr, size) __static_realloc(__alloc_head_##name, __alloc_space_##name, name, __##name##_size, ptr, size)
#define static_buf_free(name, ptr)   __static_free(__alloc_head_##name, __alloc_space_##name, name, ptr)

#endif  // _ARROW_C_SDK_STATIC_ALLOC_H_
