/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <arrow/api/json/parse.h>

JsonNode *parse_size_data(JsonNode *_main, page_size_t *ps) {
    if ( ps ) memset(ps, 0x0, sizeof(page_size_t));
    JsonNode *_size = json_find_member(_main, "size");
    if ( !_size && _size->tag != JSON_NUMBER ) return NULL;
    int __size = (int)_size->number_;
    if ( __size ) {
        if ( ps ) {
            ps->size = __size;
            JsonNode *_page = json_find_member(_main, "page");
            if ( !_page && _page->tag != JSON_NUMBER ) return NULL;
            ps->page = _page->number_;
            JsonNode *_totalSize = json_find_member(_main, "totalSize");
            if ( !_totalSize && _totalSize->tag != JSON_NUMBER ) return NULL;
            ps->totalSize = _totalSize->number_;
            JsonNode *_totalPages = json_find_member(_main, "totalPages");
            if ( !_totalPages && _totalPages->tag != JSON_NUMBER ) return NULL;
            ps->totalPages = _totalPages->number_;
        }
        JsonNode *_data = json_find_member(_main, "data");
        if ( !_data ) return NULL;
        return _data;
    }
    return NULL;
}


int parse_who_when(JsonNode *tmp, who_when_t *ww, const char *date, const char *person) {
    // FIXME parse timestamp
    JsonNode *t = json_find_member(tmp, date);
    if ( t && t->tag == JSON_STRING )
        strptime(t->string_, "%Y-%m-%dT%H:%M:%S", &ww->date);
    t = json_find_member(tmp, person);
    if ( t && t->tag == JSON_STRING )
        property_copy( &ww->by, p_stack(t->string_));
    return 0;
}
