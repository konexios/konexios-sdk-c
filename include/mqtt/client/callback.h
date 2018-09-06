/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#if !defined(ACN_SDK_C_MQTT_CLIENT_CALLBACK_H_)
#define ACN_SDK_C_MQTT_CLIENT_CALLBACK_H_

#include <MQTTClient.h>
#include <mqtt/client/delivery.h>

int arrow_mqtt_client_delivery_message_reg(MQTTClient *c, arrow_mqtt_delivery_callback_t *dc);
int arrow_mqtt_client_delivery_message_init(MQTTClient *c, MQTTString *topicName, MQTTMessage *message);
int arrow_mqtt_client_delivery_message_process(MQTTClient *c, MQTTString *topicName, MQTTMessage *message);
int arrow_mqtt_client_delivery_message_done(MQTTClient *c, MQTTString *topicName, MQTTMessage *message);

#endif
