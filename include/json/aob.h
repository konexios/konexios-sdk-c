/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_ACN_SDK_C_JSON_AOB_H_
#define ARROW_ACN_SDK_C_JSON_AOB_H_

#include <sys/mem.h>

typedef struct alloc_only {
    void *start;
    uint16_t size;
    uint16_t len;
} alloc_only_t;

void alloc_only_memory_set(alloc_only_t *p, void *start, int len);
void alloc_only_init(alloc_only_t *p);
int alloc_only_put(alloc_only_t *p, char c);
int alloc_only_puts(alloc_only_t *p, char *s, int len);
void *alloc_only_finish(alloc_only_t *p);


#endif
