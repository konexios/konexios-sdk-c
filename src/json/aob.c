/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "json/aob.h"
#include <data/linkedlist.h>

typedef struct ao_traits_ {
    void *key;
    property_t source;
    konexios_linked_list_head_node;
} ao_traits_t;

ao_traits_t *__traits = NULL;

static int traiteq( ao_traits_t *t, void *data ) {
    if ( t->key == data ) return 0;
    return -1;
}

int alloc_only_memory_set(alloc_only_t *p, property_t *b)  {
    p->start = b->value;
    p->size = property_size(b);
    p->len = 0;
    p->offset = 0;
    property_move(&p->source, b);
    return 0;
}

int alloc_only_init(alloc_only_t *p) {
    p->len = 0;
    return 0;
}

int alloc_only_size(alloc_only_t *p) {
    return p->size;
}

int alloc_only_put(alloc_only_t *p, char c) {
    if ( p->size - p->len - 1 > 0 ) {
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

void alloc_only_clear(alloc_only_t *p) {
    p->len = 0;
}

void *alloc_only_finish(alloc_only_t *p) {
    char *ptr = p->start;
    ptr[p->len++] = 0;
    p->start = (void*)((char*)p->start + p->len);
    p->size -= p->len;
    p->offset += p->len;
    p->len = 0;
    return (void*)ptr;
}

property_t alloc_only_finish_property(alloc_only_t *p) {
    property_t tmp;
    tmp.size = p->len;
    tmp.flags = PROPERTY_AOB_TAG;
    if ( !p->offset ) {
        // origin head of memory
        tmp.flags |= is_owner;
        ao_traits_t *aot = alloc_type(ao_traits_t);
        konexios_linked_list_init(aot);
        aot->key = P_VALUE(p->source);
        property_move(&aot->source, &p->source);
        konexios_linked_list_add_node_last(__traits, ao_traits_t, aot);
    }
    tmp.value = alloc_only_finish(p);
    return tmp;
}

#include <json/json.h>

void aob_weak(property_t *dst, property_t *src) {
    dst->value = src->value;
    dst->size = src->size;
    dst->flags = PROPERTY_AOB_TAG;
}

void aob_copy(property_t *dst, property_t *src) {
    // shouldn't be only weak copy
    aob_weak(dst, src);
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
    // only for origin memory head
    ao_traits_t *aot = NULL;
    linked_list_find_node(aot, __traits, ao_traits_t, traiteq, P_VALUE(*dst));
    if ( aot ) {
        konexios_linked_list_del_node(__traits, ao_traits_t, aot);
        property_free(&aot->source);
        memset(dst, 0x0, sizeof(property_t));
        free(aot);
    }
}

static property_dispetcher_t aob_property_type = {
    PROPERTY_AOB_TAG,   { aob_copy, aob_weak, aob_move, aob_destroy }, {NULL}
};

property_dispetcher_t *property_type_get_aob() {
    return &aob_property_type;
}

