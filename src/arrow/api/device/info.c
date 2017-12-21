/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <arrow/api/device/info.h>

void device_info_init(device_info_t *gd) {
    who_when_init(&gd->created);
    who_when_init(&gd->lastModified);
    property_init(&gd->hid);
    property_init(&gd->uid);
    property_init(&gd->name);
    property_init(&gd->type);
    property_init(&gd->gatewayHid);
    gd->enabled = 0;
    gd->info = NULL;
    gd->properties = NULL;
}

void device_info_free(device_info_t *gd) {
    who_when_free(&gd->created);
    who_when_free(&gd->lastModified);
    property_free(&gd->hid);
    property_free(&gd->uid);
    property_free(&gd->name);
    property_free(&gd->type);
    property_free(&gd->gatewayHid);
    gd->enabled = 0;
    json_delete(gd->info);
    json_delete(gd->properties);
}

int device_info_parse(device_info_t **list, const char *s) {
    JsonNode *_main = json_decode(s);
    if ( !_main ) return -1;
    JsonNode *_data = parse_size_data(_main, NULL);
    if ( _data ) {
        JsonNode *tmp = NULL;
        json_foreach(tmp, _data) {
            device_info_t *gd = (device_info_t *)malloc(sizeof(device_info_t));
            device_info_init(gd);
            parse_who_when(tmp, &gd->created, "createdDate", "createdBy");
            parse_who_when(tmp, &gd->lastModified, "lastModifiedDate", "lastModifiedBy");
            json_fill_property(tmp, gd, hid);
            json_fill_property(tmp, gd, uid);
            json_fill_property(tmp, gd, name);
            json_fill_property(tmp, gd, type);
            json_fill_property(tmp, gd, gatewayHid);
            JsonNode *t = json_find_member(tmp, "enabled");
            if ( t && t->tag == JSON_BOOL )
                gd->enabled = t->bool_;
            t = json_find_member(tmp, "info");
            json_remove_from_parent(t);
            gd->info = t;
            t = json_find_member(tmp, "properties");
            json_remove_from_parent(t);
            gd->properties = t;
            linked_list_add_node_last(*list, device_info_t, gd);
        }
    }
    json_delete(_main);
    return 0;
}
