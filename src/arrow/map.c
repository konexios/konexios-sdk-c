/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/map.h"

int property_map_add(property_map_t **m, property_t key, property_t value) {
    property_map_t *el = (property_map_t *)malloc(sizeof(property_map_t));
    property_copy(&el->key, key);
    property_copy(&el->value, value);
    el->next = NULL;
    property_map_t *root = *m;
    if ( root ) {
        while ( root->next ) {
            if ( property_cmp(&root->key, &key) == 0 ) {
                free(el);
                return -1;
            }
            root = root->next;
        }
        root->next = el;
    } else {
        *m = el;
    }
    return 0;
}

int property_map_delete(property_map_t *m, property_t key) {
    property_map_t *pre = NULL;
    property_map_t *root = m;
    while ( root && property_cmp(&root->key, &key) < 0 ) {
        pre = root;
        root = root->next;
    }
    if ( root ) {
        pre->next = root;
        property_free(&root->key);
        property_free(&root->value);
        free(root);
        return 0;
    }
    return -1;
}

property_map_t *property_map_find(property_map_t *m, property_t key) {
    property_map_t *root = m;
    while ( root ) {
        if ( property_cmp(&root->key, &key) == 0 ) {
            return root;
        }
        root = root->next;
    }
    return NULL;
}

int property_map_assign(property_map_t *m, property_t key, property_t value) {
    property_map_t *el = property_map_find(m, key);
    if ( el ) el->value = value;
    else return -1;
    return 0;
}

int property_map_clear(property_map_t *m) {
    property_map_t *root = m;
    while ( root ) {
        property_map_t *next = root->next;
        property_free(&root->key);
        property_free(&root->value);
        free(root);
        root = next;
    }
    return 0;
}
