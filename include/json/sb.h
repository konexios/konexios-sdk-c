/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef KONEXIOS_ACN_SDK_C_JSON_SB_H_
#define KONEXIOS_ACN_SDK_C_JSON_SB_H_

#include <json/property_json.h>

/* String buffer */
typedef struct {
    char *cur;
    char *end;
    char *start;
} SB;

int sb_init(SB *sb);
int sb_size_init(SB *sb, int need);
int sb_grow(SB *sb, int need);
int sb_need(SB *sb, int need);
int sb_put(SB *sb, const char *bytes, int count);
int sb_putc(SB *sb, char byte);
int sb_is_valid(SB *sb);
int sb_puts(SB *sb, const char *str);
char *sb_finish(SB *sb);
property_t sb_finish_property(SB *sb);
void sb_clear(SB *sb);
void sb_free(SB *sb);
int sb_size(SB *sb);

#endif
