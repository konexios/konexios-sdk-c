/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#if !defined(ACN_SDK_C_MQTT_CLIENT_DELIVERY_H_)
#define ACN_SDK_C_MQTT_CLIENT_DELIVERY_H_

#include <data/property.h>
#include <data/linkedlist.h>

typedef int(*konexios_mqtt_delivery_init)(int);
typedef int(*konexios_mqtt_delivery_process)(const char *,int);
typedef int(*konexios_mqtt_delivery_close)(void);

typedef struct konexios_mqtt_delivery_callback_ {
    property_t topic;
    konexios_mqtt_delivery_init init;
    konexios_mqtt_delivery_process process;
    konexios_mqtt_delivery_close done;
    konexios_linked_list_head_node;
} konexios_mqtt_delivery_callback_t;

#endif
