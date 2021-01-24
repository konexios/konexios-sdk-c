/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <mqtt/client/callback.h>

static int deliveryeq( konexios_mqtt_delivery_callback_t *cb, MQTTString *topic ) {
    if ( topic->cstring ) {
        if ( strcmp(P_VALUE(cb->topic), topic->cstring) == 0 ) return 0;
    } else {
        if ( strncmp(P_VALUE(cb->topic), topic->lenstring.data, topic->lenstring.len) == 0 ) return 0;
    }
    return -1;
}

int konexios_mqtt_client_delivery_message_reg(MQTTClient *c, konexios_mqtt_delivery_callback_t *dc) {
    konexios_mqtt_delivery_callback_t *tmp;
    MQTTString topicname;
    topicname.cstring = P_VALUE(dc->topic);
    linked_list_find_node(tmp, c->delivery_cb, konexios_mqtt_delivery_callback_t, deliveryeq, &topicname);
    if ( !tmp ) {
        konexios_linked_list_add_node_last(c->delivery_cb, konexios_mqtt_delivery_callback_t, dc);
        return 0;
    }
    return 0;
}

int konexios_mqtt_client_delivery_message_init(MQTTClient *c, MQTTString *topicName, MQTTMessage *message) {
    SSP_PARAMETER_NOT_USED(c);
    konexios_mqtt_delivery_callback_t *tmp;
    linked_list_find_node(tmp, c->delivery_cb, konexios_mqtt_delivery_callback_t, deliveryeq, topicName);
    if ( tmp && tmp->init ) return tmp->init(message->payloadlen);
    return -1;
}

int konexios_mqtt_client_delivery_message_process(MQTTClient *c, MQTTString *topicName, MQTTMessage *message) {
    SSP_PARAMETER_NOT_USED(c);
    konexios_mqtt_delivery_callback_t *tmp;
    linked_list_find_node(tmp, c->delivery_cb, konexios_mqtt_delivery_callback_t, deliveryeq, topicName);
    if ( tmp && tmp->process ) return tmp->process((char *)message->payload, message->payloadlen);
    return -1;
}

int konexios_mqtt_client_delivery_message_done(MQTTClient *c, MQTTString *topicName, MQTTMessage *message) {
    SSP_PARAMETER_NOT_USED(c);
    SSP_PARAMETER_NOT_USED(message);
    konexios_mqtt_delivery_callback_t *tmp;
    linked_list_find_node(tmp, c->delivery_cb, konexios_mqtt_delivery_callback_t, deliveryeq, topicName);
    if ( tmp && tmp->done ) return tmp->done();
    return -1;
}
