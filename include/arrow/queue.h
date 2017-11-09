/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef SDK_QUEUE_H_
#define SDK_QUEUE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <unint.h>

#if !defined(QUEUE_SIZE)
#define QUEUE_SIZE 256
#endif

typedef struct _queue_buffer_ {
    uint8_t buffer[QUEUE_SIZE];
    uint16_t size;
    uint16_t shift;
} queue_buffer_t;

uint16_t queue_capacity(queue_buffer_t *buf);
uint16_t queue_size(queue_buffer_t *buf);
int queue_strcat(queue_buffer_t *buf, const char *str);
int queue_clear( queue_buffer_t *buf );
int queue_push(queue_buffer_t *buf, uint8_t *s, int len);
int queue_pop(queue_buffer_t *buf, uint8_t *s, int len);


#if defined(__cplusplus)
}
#endif

#endif  // SDK_QUEUE_H_
