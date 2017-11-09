/* Copyright (c) 2017 Arrow Electronics, Inc.
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Apache License 2.0
* which accompanies this distribution, and is available at
* http://apache.org/licenses/LICENSE-2.0
* Contributors: Arrow Electronics, Inc.
*/

#include "arrow/queue.h"
#include <debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

inline uint16_t queue_capacity(queue_buffer_t *buf) {
    return (uint16_t) ( sizeof(buf->buffer) - 1 - buf->size );
}

inline uint16_t queue_size(queue_buffer_t *buf) {
    return buf->size;
}

int queue_strcat(queue_buffer_t *buf, const char *str) {
    int len = strlen(str);
    return queue_push(buf, (uint8_t*)str, len);
}

int queue_clear( queue_buffer_t *buf ) {
    buf->buffer[0] = 0x0;
    buf->size = 0;
    buf->shift = 0;
    return 0;
}

int queue_push(queue_buffer_t *buf, uint8_t *s, int len) {
    if ( !len ) len = strlen((char*)s);
  if ( queue_capacity(buf) < len ) return -1;
  int wr = ( buf->shift + buf->size ) % sizeof(buf->buffer);
  int till_border = sizeof(buf->buffer) - wr;
  if ( till_border < len ) {
      memcpy(buf->buffer + wr, s, till_border);
      memcpy(buf->buffer, s + till_border, len - till_border);
  } else {
      memcpy(buf->buffer + wr, s, len);
  }
  buf->size += len;
  return 0;
}

int queue_pop(queue_buffer_t *buf, uint8_t *s, int len) {
    if ( queue_size(buf) < len ) return -1;
    int till_border = sizeof(buf->buffer) - buf->shift;
    if ( till_border < len ) {
        memcpy(s, buf->buffer + buf->shift, till_border);
        memcpy(s + till_border, buf->buffer, len - till_border);
    } else {
        memcpy(s, buf->buffer + buf->shift, len);
    }
    buf->shift = ( buf->shift + len ) % sizeof(buf->buffer);
    buf->size -= len;
    return 0;
}
