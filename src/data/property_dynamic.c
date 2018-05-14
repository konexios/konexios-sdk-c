/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "data/property_dynamic.h"
#include <debug.h>

void dynmc_copy(property_t *dst, property_t *src) {
    dst->size = src->size;
    if ( ! (src->flags & is_owner) ) {
        dst->value = src->value;
        dst->flags = src->flags;
        return;
    }
    if ( src->flags & is_raw ) {
        dst->value = (char *)strndup(src->value, src->size);
    } else {
        dst->value = (char *)strdup(src->value);
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
        free(dst->value);
    }
}

static property_dispetcher_t dynamic_property_type = {
    PROPERTY_DYNAMIC_TAG, { dynmc_copy, dynmc_weak, dynmc_move, dynmc_destroy }, {NULL}
};

property_dispetcher_t *property_type_get_dynamic() {
    return &dynamic_property_type;
}
