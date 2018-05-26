/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "data/property_dynamic.h"
#include <debug.h>

#if defined(STATIC_DYNAMIC_PROPERTY)
#include <data/static_buf.h>
CREATE_BUFFER(dynamicbuf, ARROW_DYNAMIC_STATIC_BUFFER_SIZE>>5)

static void *static_strndup(char *ptr, int size) {
    void *p = static_buf_alloc(dynamicbuf, size + 1);
    if ( !p ) {
        DBG("Out of Memory: static dynamic");
        return NULL;
    }
    memcpy(p, ptr, size);
    ((char *)p)[size + 1] = 0x0;
    return p;
}

static void *static_strdup(char *ptr) {
    int size =  strlen(ptr);
    void *p = static_buf_alloc(dynamicbuf, size + 1);
    if ( !p ) {
        DBG("Out of Memory: static dynamic");
        return NULL;
    }
    memcpy(p, ptr, size);
    ((char *)p)[size] = 0x0;
    return p;
}

void static_free(void *p) {
    static_buf_free(dynamicbuf, p);
}

#define STRNDUP static_strndup
#define STRDUP  static_strdup
#define FREE    static_free
#else
#define STRNDUP strndup
#define STRDUP  strdup
#define FREE    free
#endif

void dynmc_copy(property_t *dst, property_t *src) {
    dst->size = src->size;
    if ( ! (src->flags & is_owner) ) {
        dst->value = src->value;
        dst->flags = src->flags;
        return;
    }
    if ( src->flags & is_raw ) {
        dst->value = (char *)STRNDUP(src->value, src->size);
    } else {
        dst->value = (char *)STRDUP(src->value);
    }
    dst->flags = is_owner | PROPERTY_DYNAMIC_TAG;
}

void dynmc_weak(property_t *dst, property_t *src) {
    dst->value = src->value;
    dst->size = src->size;
    dst->flags = PROPERTY_DYNAMIC_TAG;
}

void dynmc_move(property_t *dst, property_t *src) {
    if ( ! ( src->flags & is_owner ) ) return;
    dst->value = src->value;
    dst->size = src->size;
    dst->flags = is_owner | PROPERTY_DYNAMIC_TAG;
    src->flags &= ~is_owner; // make weak
}

void dynmc_destroy(property_t *dst) {
    if ( dst->flags & is_owner ) {
        FREE(dst->value);
    }
}

static property_dispetcher_t dynamic_property_type = {
    PROPERTY_DYNAMIC_TAG, { dynmc_copy, dynmc_weak, dynmc_move, dynmc_destroy }, {NULL}
};

property_dispetcher_t *property_type_get_dynamic() {
    return &dynamic_property_type;
}
