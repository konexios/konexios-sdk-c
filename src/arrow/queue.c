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
    return (uint16_t) ( sizeof(buf->buffer) - buf->size );
}

inline uint16_t queue_size(queue_buffer_t *buf) {
    return buf->size;
}

uint8_t *queue_wr_addr(queue_buffer_t *buf) {
    return (buf->buffer + buf->shift + buf->size);
}

uint8_t *queue_rd_addr(queue_buffer_t *buf) {
    return (buf->buffer + buf->shift);
}

int queue_size_add(queue_buffer_t *buf, uint16_t sz) {
    buf->size += sz;
    if ( buf->shift + buf->size >= sizeof ( buf->buffer ) ) {
        return -1;
    }
    buf->buffer[buf->shift + buf->size] = 0x0;
    return 0;
}

int queue_null_terminate(queue_buffer_t *buf, uint8_t *end) {
    if ( (intptr_t)end >= (intptr_t)buf->buffer &&
         (intptr_t)end < (intptr_t)buf->buffer + (intptr_t)sizeof(buf->buffer) ) {
        *end = 0x0;
        return *end;
    }
    return -1;
}

#if 0
static int queue_strcpy(queue_buffer_t *buf, const char *str) {
    int len = strlen(str);
    strcpy((char *)buf->http_buffer, str);
    buf->last = len;
    return 0;
}
#endif

int queue_strcat(queue_buffer_t *buf, const char *str) {
    int len = strlen((char*)str);
    memcpy(buf->buffer + buf->shift + buf->size, str, len);
    buf->size += len;
    return 0;
}

#if 0
static uint8_t *queue_index(queue_buffer_t *buf, uint16_t index) {
    if ( buf->shift + index < sizeof(buf->http_buffer) ) {
        return buf->http_buffer + buf->shift + index;
    }
    return NULL;
}
#endif

int queue_clear( queue_buffer_t *buf ) {
    buf->buffer[0] = 0x0;
    buf->size = 0;
    buf->shift = 0;
    return 0;
}

int queue_shift_clear( queue_buffer_t *buf ) {
    if ( buf->shift ) {
        memmove(buf->buffer,
                buf->buffer + buf->shift,
                buf->size);
        buf->shift = 0;
        buf->buffer[buf->size] = 0x0;
        return buf->shift;
    }
    return -1;
}

int queue_shift_immediately( queue_buffer_t *buf, uint8_t *pos ) {
    if ( (intptr_t)pos >= (intptr_t)buf->buffer &&
         (intptr_t)pos < (intptr_t)buf->buffer + (intptr_t)sizeof(buf->buffer) ) {
        uint16_t sz = (uint16_t)(pos - buf->buffer);
        buf->shift += sz;
        buf->size -= sz;
        return queue_shift_clear(buf);
    }
    return -1;
}

int queue_shift_immediately_by_ind( queue_buffer_t *buf, uint16_t pos ) {
    if ( buf->shift + buf->size + pos < (uint16_t)sizeof(buf->buffer) ) {
        buf->shift += pos;
        buf->size -= pos;
        return queue_shift_clear(buf);
    }
    return -1;
}

int queue_shift(queue_buffer_t *buf, uint16_t sz) {
    buf->shift += sz;
    buf->size -= sz;
    if ( buf->shift > ( sizeof(buf->buffer) >> 2 ) ) {
        DBG(" === forced shift :queue:");
        queue_shift_clear(buf);
    }
    return 0;
}

int queue_printf(queue_buffer_t *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf((char*)queue_wr_addr(buf),
                        queue_capacity(buf),
                        fmt, args);
    if ( ret < 0 ) goto queue_printf_end;
    queue_size_add(buf, ret);
queue_printf_end:
    va_end(args);
    return ret;
}
