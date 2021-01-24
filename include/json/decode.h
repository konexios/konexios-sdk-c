/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */
#ifndef KONEXIOS_ACN_SDK_C_JSON_DECODE_H_
#define KONEXIOS_ACN_SDK_C_JSON_DECODE_H_

#include <data/linkedlist.h>
#include <json/sb.h>
#include <json/aob.h>
#include <json/json.h>

typedef int(*buffer_init)(void *);
typedef int(*buffer_putc)(void *, char c);
typedef int(*buffer_puts)(void *, const char *s, int len);
typedef char*(*buffer_fin)(void *);
typedef void(*buffer_clear)(void *);
typedef void(*buffer_free)(void *);
typedef int(*buffer_size)(void *);

typedef struct buffer_drive_ {
    int i;
    buffer_init init;
    buffer_size size;
    buffer_putc putbyte;
    buffer_puts puts;
    buffer_clear clear;
    buffer_fin  fin;
    buffer_free free;
} buffer_drive_t;

typedef int(*_json_parse_fn)(void *, char);

typedef struct _json_parse_machine_ {
    struct _json_parse_machine_ *p;
    uint32_t complete;
    _json_parse_fn process_byte;
    alloc_only_t *buffer;
    property_t key;
    JsonNode *root;
    konexios_linked_list_head_node;
} json_parse_machine_t;


int json_parse_machine_init(json_parse_machine_t *jpm, alloc_only_t *allocator);
int json_parse_machine_process(json_parse_machine_t *jpm, char byte);
int json_parse_machine_fin(json_parse_machine_t *jpm);

int json_decode_init_at_property(json_parse_machine_t *sm, property_t *buffer);
int json_decode_init(json_parse_machine_t *sm, int len) __attribute_warn_unused_result__;
int json_decode_part(json_parse_machine_t *sm, const char *json, size_t size) __attribute_warn_unused_result__;
JsonNode *json_decode_finish(json_parse_machine_t *sm) __attribute_warn_unused_result__;

JsonNode *json_decode_property(property_t prop) __attribute_warn_unused_result__;
JsonNode *json_decode_property_at(property_t prop, property_t *buffer) __attribute_warn_unused_result__;

#endif
