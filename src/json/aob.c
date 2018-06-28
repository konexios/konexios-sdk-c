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
