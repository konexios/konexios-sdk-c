/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "json/aob.h"

void alloc_only_memory_set(alloc_only_t *p, void *start, int len)  {
    p->start = start;
    p->size = len;
    p->len = 0;
}

void alloc_only_init(alloc_only_t *p) {
    p->len = 0;
}

int alloc_only_put(alloc_only_t *p, char c) {
    if ( p->size - p->len ) {
        *((uint8_t*)p->start + p->len) = c;
        p->len++;
        return 0;
    }
    return -1;
}

int alloc_only_puts(alloc_only_t *p, char *s, int len) {
    if ( p->size - p->len > len ) {
        strncpy((char*)p->start + p->len, s, len);
        p->len += len;
        return 0;
    }
    return -1;
}

void *alloc_only_finish(alloc_only_t *p) {
    char *ptr = p->start;
    ptr[p->len++] = 0;
    p->start = (void*)((char*)p->start + p->len);
    p->size -= p->len;
    p->len = 0;
    return (void*)ptr;
}

#include <json/json.h>
#include <debug.h>

void aob_copy(property_t *dst, property_t *src) {
    // shouldn't be a copy in the same piece memory
    property_copy_as(PROPERTY_DYNAMIC_TAG, dst, *src);
}

void aob_weak(property_t *dst, property_t *src) {
    dst->value = src->value;
    dst->size = src->size;
    dst->flags = PROPERTY_AOB_TAG;
}

void aob_move(property_t *dst, property_t *src) {
    dst->value = src->value;
    dst->size = src->size;
    dst->flags = PROPERTY_AOB_TAG;
    if ( src->flags & is_owner ) {
        dst->flags |= is_owner;
        src->flags &= ~is_owner;
    }
}

void aob_destroy(property_t *dst) {
    memset(dst, 0x0, sizeof(property_t));
}

static property_dispetcher_t aob_property_type = {
    PROPERTY_AOB_TAG,   { aob_copy, aob_weak, aob_move, aob_destroy }, {NULL}
};

property_dispetcher_t *property_type_get_aob() {
    return &aob_property_type;
}

