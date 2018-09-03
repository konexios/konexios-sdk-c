/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <json/decode.h>
#include <json/property_json.h>
#include <json/aob.h>
#include <debug.h>

#define is_array_context(mach) ((mach)->p->root->tag == JSON_ARRAY)

#define BUFFER_INIT(jpm)    alloc_only_init((jpm)->buffer)
#define BUFFER_FIN(jpm)     alloc_only_finish_property((jpm)->buffer)
#define BUFFER_PUTC(jpm, c) alloc_only_put((jpm)->buffer, (c))
#define BUFFER_CLEAR(jpm)   alloc_only_clear((jpm)->buffer)
#define BUFFER_SIZE(jpm)    alloc_only_size((jpm)->buffer)

static int jpm_key_init(json_parse_machine_t *jpm, char byte);
static int jpm_key_body(json_parse_machine_t *jpm, char byte);
static int jpm_value_init(json_parse_machine_t *jpm, char byte);
static int jpm_value_end(json_parse_machine_t *jpm, char byte);
static int jpm_string_body(json_parse_machine_t *jpm, char byte);

static int json_parse_machine_clear(json_parse_machine_t *jpm);

#define jpm_bool_state_char(b, c, sym, n) \
static int jpm_bool_##b##_##c(json_parse_machine_t *jpm, char byte) { \
    if ( byte == sym ) { \
        jpm->process_byte = (_json_parse_fn) jpm_bool_##b##_##n; \
        return 0; \
    } \
    return -1; \
}

static int jpm_bool_false_e(json_parse_machine_t *jpm, char byte) {
    if ( byte == 'e' ) {
        jpm->root = json_mkbool(false);
        if ( !jpm->root ) return -1;
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
        return 0;
    }
    return -1;
}

jpm_bool_state_char(false, s, 's', e)
jpm_bool_state_char(false, l, 'l', s)
jpm_bool_state_char(false, a, 'a', l)

static int jpm_bool_true_e(json_parse_machine_t *jpm, char byte) {
    if ( byte == 'e' ) {
        jpm->root = json_mkbool(true);
        if ( !jpm->root ) {
            DBG("OOM mkbool");
            return -1;
        }
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
        return 0;
    }
    return -1;
}

jpm_bool_state_char(true, u, 'u', e)
jpm_bool_state_char(true, r, 'r', u)


static int jpm_string_escape(json_parse_machine_t *jpm, char byte) {
    int r = 0;
    switch ( byte ) {
    case 'n': {
        r = BUFFER_PUTC(jpm, '\n');
    } break;
    case 't': {
        r = BUFFER_PUTC(jpm, '\t');
    } break;
    default:
        r = BUFFER_PUTC(jpm, byte);
    }
    if ( r < 0 ) return r;
    jpm->process_byte = (_json_parse_fn) jpm_string_body;
    return 0;
}

static int jpm_string_body(json_parse_machine_t *jpm, char byte) {
    switch ( byte ) {
    case '\\': {
        jpm->process_byte = (_json_parse_fn) jpm_string_escape;
    } break;
    case '"': {
        if ( BUFFER_SIZE(jpm) <= 0 ) return -1;
        property_t value = BUFFER_FIN(jpm);
        if ( IS_EMPTY(value) ) return -1;
        jpm->root = json_mkproperty(&value);
        if ( !jpm->root ) return -1;
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
        BUFFER_CLEAR(jpm);
    } break;
    default:
        if ( BUFFER_PUTC(jpm, byte) < 0 ) return -1;
        break;
    }
    return 0;
}

static int jpm_number_body(json_parse_machine_t *jpm, char byte) {
    if ( is_digit(byte) || byte == '.' || byte == '-' ) {
        if ( BUFFER_PUTC(jpm, byte) < 0 ) return -1;
    } else {
        if ( BUFFER_SIZE(jpm) <= 0 ) return -1;
        char *str = (char *)jpm->buffer->start;
        if ( !str ) return -1;
        str[jpm->buffer->len] = 0;
        double d = strtod(str, NULL);
        jpm->root = json_mknumber(d);
        if ( !jpm->root ) return -1;
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
        BUFFER_CLEAR(jpm);
        return jpm->process_byte(jpm, byte);
    }
    return 0;
}

static int jpm_value_init(json_parse_machine_t *jpm, char byte) {
    if ( is_space(byte) ) return 0;
    switch( byte ) {
    case '{' : {
        json_parse_machine_t *nvalue = alloc_type(json_parse_machine_t);
        if ( !nvalue ) return -1;
        json_parse_machine_init(nvalue, jpm->buffer);
        arrow_linked_list_add_node_last(jpm, json_parse_machine_t, nvalue);
        nvalue->process_byte = (_json_parse_fn) jpm_key_init;
        jpm->root = json_mkobject();
        if ( !jpm->root ) return -1;
        nvalue->p = jpm;
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
    } break;
    case '"': {
        if ( BUFFER_INIT(jpm) < 0 )
            return -1;
        jpm->process_byte = (_json_parse_fn) jpm_string_body;
    } break;
    case 'f': {
        jpm->process_byte = (_json_parse_fn) jpm_bool_false_a;
    } break;
    case 't': {
        jpm->process_byte = (_json_parse_fn) jpm_bool_true_r;
    } break;
    case '[': {
        json_parse_machine_t *nvalue = alloc_type(json_parse_machine_t);
        if ( !nvalue ) return -1;
        json_parse_machine_init(nvalue, jpm->buffer);
        arrow_linked_list_add_node_last(jpm, json_parse_machine_t, nvalue);
        nvalue->process_byte = (_json_parse_fn) jpm_value_init;
        jpm->root = json_mkarray();
        if ( !jpm->root ) return -1;
        nvalue->p = jpm;
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
    } break;
    default:
        if ( BUFFER_INIT(jpm) < 0 )
            return -1;
        jpm->process_byte = (_json_parse_fn) jpm_number_body;
        return jpm->process_byte(jpm, byte);
    }
    return 0;
}

static int jpm_append_to_parent(json_parse_machine_t *jpm) {
    if ( jpm->p ) {
        if ( is_array_context(jpm) ) {
            json_append_element(jpm->p->root, jpm->root);
        } else {
            json_append_member(jpm->p->root, jpm->key, jpm->root);
        }
        return 0;
    }
    return -1;
}

static int jpm_value_end(json_parse_machine_t *jpm, char byte) {
    if ( is_space(byte) ) return 0;
    switch ( byte ) {
    case '}': {
        jpm->complete = 1;
        if ( !IS_EMPTY(jpm->key) ) {
            if ( jpm_append_to_parent(jpm) < 0 ) return -1;
            jpm->key = p_null;
        }
        BUFFER_CLEAR(jpm);
        jpm->root = NULL;
    } break;
    case ']': {
        jpm->complete = 1;
        if ( jpm_append_to_parent(jpm) < 0 )
            return -1;
        else
            jpm->root = NULL;
        jpm->key = p_null;
        BUFFER_CLEAR(jpm);
        jpm->process_byte = NULL;
    } break;
    case ',': {
        if ( jpm_append_to_parent(jpm) < 0 ) return -1;
        jpm->key = p_null;
        json_parse_machine_t *prev = jpm->p;
        json_parse_machine_clear(jpm);
        BUFFER_CLEAR(jpm);
        jpm->p = prev;
        if ( is_array_context(jpm) )
            jpm->process_byte = (_json_parse_fn) jpm_value_init;
        else
            jpm->process_byte = (_json_parse_fn) jpm_key_init;

    } break;
    default:
        return -1;
    }
    return 0;
}

static int jpm_key_init(json_parse_machine_t *jpm, char byte) {
    if ( is_space(byte) ) return 0;
    switch ( byte ) {
    case '"': {
        if ( BUFFER_INIT(jpm) < 0 )
            return -1;
        jpm->process_byte = (_json_parse_fn) jpm_key_body;
    } break;
    case '}':
        // empty object
        jpm->process_byte = (_json_parse_fn) jpm_value_end;
        return jpm->process_byte(jpm, byte);
        break;
    default:
        return -1;
    }
    return 0;
}

static int jpm_key_end(json_parse_machine_t *jpm, char byte) {
    if ( is_space(byte) ) return 0;
    switch ( byte ) {
    case ':': {
        jpm->process_byte = (_json_parse_fn) jpm_value_init;
    } break;
    default:
        return -1;
    }
    return 0;
}

static int jpm_key_body(json_parse_machine_t *jpm, char byte) {
    switch ( byte ) {
    case '"': {
        if ( BUFFER_SIZE(jpm) <= 0 ) return -1;
        property_t key = BUFFER_FIN(jpm);
        property_move(&jpm->key, &key);
        property_free(&key);
        BUFFER_CLEAR(jpm);
        jpm->process_byte = (_json_parse_fn) jpm_key_end;
    } break;
    default:
        if ( BUFFER_PUTC(jpm, byte) < 0 ) return -1;
        break;
    }
    return 0;
}


int json_parse_machine_init(json_parse_machine_t *jpm, alloc_only_t *allocator) {
    jpm->complete = 0;
    jpm->process_byte = (_json_parse_fn) jpm_value_init;
    property_init(&jpm->key);
    jpm->p = NULL;
    jpm->root = NULL;
    jpm->buffer = allocator;
    BUFFER_INIT(jpm);
    arrow_linked_list_init(jpm);
    return 0;
}

static int json_parse_machine_clear(json_parse_machine_t *jpm) {
    jpm->complete = 0;
    jpm->process_byte = (_json_parse_fn) jpm_value_init;
    property_init(&jpm->key);
    jpm->p = NULL;
    jpm->root = NULL;
    BUFFER_CLEAR(jpm);
    arrow_linked_list_init(jpm);
    return 0;
}

int json_parse_machine_process(json_parse_machine_t *jpm, char byte) {
    json_parse_machine_t *next = NULL;
    arrow_linked_list_next_node(next, jpm, json_parse_machine_t);
    if ( next ) {
        // pass into last machine
        if ( next->complete ) {
            arrow_linked_list_del_node(jpm, json_parse_machine_t, next);
            json_parse_machine_fin(next);
            free(next);
            return json_parse_machine_process(jpm, byte);
        } else {
            return json_parse_machine_process(next, byte);
        }
    } else {
        if ( jpm->process_byte ) {
            int r = jpm->process_byte(jpm, byte);
            if (r < 0) {
                if ( jpm->root ) json_delete(jpm->root);
            }
            return r;
        }
    }
    return -1;
}

int json_parse_machine_fin(json_parse_machine_t *jpm) {
//    if ( BUFFER_SIZE(jpm) ) BUFFER_FREE(jpm);
    if ( !IS_EMPTY(jpm->key) ) property_free(&jpm->key);
    jpm->process_byte = NULL;
    jpm->p = NULL;
    jpm->root = NULL;
    return 0;
}

int json_decode_init_at_property(json_parse_machine_t *sm, property_t *buffer) {
    alloc_only_t *ao = NULL;
    ao = alloc_type(alloc_only_t);
    if ( !ao ||
         alloc_only_memory_set(ao, buffer) < 0 ) return -1;
    return json_parse_machine_init(sm, ao);
}

int json_decode_init(json_parse_machine_t *sm, int len) {
    SB buf;
    int r = sb_size_init(&buf, len);
    if ( r < 0 ) return r;
    property_t tmp = { buf.start, PROPERTY_JSON_TAG | is_owner, len };
    r = json_decode_init_at_property(sm, &tmp);
    if ( r < 0 ) {
        sb_free(&buf);
        return -1;
    }
    return r;
}

int json_decode_part(json_parse_machine_t *sm, const char *json, size_t size) {
    const char *s = json;
    size_t i = 0;
    for ( i = 0; i < size; i++ ) {
        if ( json_parse_machine_process(sm, s[i]) < 0 ) {
            return -1;
        }
    }
    return size;
}

JsonNode *json_decode_finish(json_parse_machine_t *sm) {
    JsonNode *root = sm->root;
    json_parse_machine_t *next = NULL;
    arrow_linked_list_next_node(next, sm, json_parse_machine_t);
    if ( next ) {
        json_parse_machine_t *tmp = NULL;
        arrow_linked_list_for_each_safe(tmp, next, json_parse_machine_t) {
            json_parse_machine_fin(tmp);
            free(tmp);
        }
    }
    if ( sm->buffer ) {
        if ( !sm->buffer->offset && !sm->buffer->len ) {
            DBG("there are no allocations for this property %p", sm->buffer->source.value);
            DBG("start %p", sm->buffer->start);
            DBG("size %d", sm->buffer->size);
            DBG("offset %d", sm->buffer->offset);
            DBG("len %d", sm->buffer->len);
            property_free(&sm->buffer->source);
        }
        free(sm->buffer);
        sm->buffer = NULL;
    }
    json_parse_machine_fin(sm);
    return root;
}

JsonNode *json_decode_property(property_t prop) {
    int size = property_size(&prop);
    int ret = -1;
    json_parse_machine_t sm;
    ret = json_decode_init(&sm, size);
    if ( ret < 0 ) return NULL;
    ret = json_decode_part(&sm, P_VALUE(prop), size);
    JsonNode *_main = json_decode_finish(&sm);
    if ( ret < 0 ){
        return NULL;
    }
    return _main;
}

// FIXME code dublicate
JsonNode *json_decode_property_at(property_t prop, property_t *buffer) {
    int size = property_size(&prop);
    int ret = -1;
    json_parse_machine_t sm;
    ret = json_decode_init_at_property(&sm, buffer);
    if ( ret < 0 ) return NULL;
    ret = json_decode_part(&sm, P_VALUE(prop), size);
    JsonNode *_main = json_decode_finish(&sm);
    if ( ret < 0 ){
        return NULL;
    }
    return _main;
}
