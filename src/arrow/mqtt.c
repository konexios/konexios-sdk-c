/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/mqtt.h"
#include <config.h>
#include <json/telemetry.h>
#include <data/property.h>
#include <arrow/events.h>
#include <debug.h>

#define USE_STATIC
#include <data/chunk.h>

#if defined(NO_EVENTS)
  #define MQTT_CLEAN_SESSION 1
#else
  #define MQTT_CLEAN_SESSION 0
#endif

static mqtt_env_t *__mqtt_channels = NULL;

extern mqtt_driver_t iot_driver;
#if defined(__IBM__)
extern mqtt_driver_t ibm_driver;
#endif
#if defined(__AZURE__)
extern mqtt_driver_t azure_driver;
#endif

static void data_prep(MQTTPacket_connectData *data) {
    MQTTPacket_connectData d = MQTTPacket_connectData_initializer;
    *data = d;
    data->cleansession = MQTT_CLEAN_SESSION;
}

static int _mqtt_init_common(mqtt_env_t *env) {
    property_init(&env->p_topic);
    property_init(&env->s_topic);
    property_init(&env->username);
    property_init(&env->addr);
    data_prep(&env->data);
    env->buf.size = MQTT_BUF_LEN;
    env->buf.buf = (unsigned char*)malloc(env->buf.size);
    env->readbuf.size = MQTT_BUF_LEN;
    env->readbuf.buf = (unsigned char*)malloc(env->readbuf.size);
    env->timeout = 3000;
    env->port = MQTT_PORT;
    env->init = 0;
    return 0;
}

static int _mqtt_deinit_common(mqtt_env_t *env) {
#if defined(MQTT_TASK)
    arrow_mutex_deinit(env->client.mutex);
#endif
    property_free(&env->p_topic);
    property_free(&env->s_topic);
    property_free(&env->username);
    property_free(&env->addr);
    free(env->buf.buf);
    free(env->readbuf.buf);
    env->buf.size = 0;
    env->readbuf.size = 0;
    return 0;
}

static int _mqtt_env_is_init(mqtt_env_t *env, uint32_t init_mask) {
    int ret = 0;
    if ( env->init & init_mask ) ret = 1;
    return ret;
}

static void _mqtt_env_set_init(mqtt_env_t *env, uint32_t init_mask) {
    env->init |= init_mask;
}

static void _mqtt_env_unset_init(mqtt_env_t *env, uint32_t init_mask) {
    env->init &= ~init_mask;
}

static int _mqtt_env_connect(mqtt_env_t *env) {
    int ret = -1;
    NetworkInit(&env->net);
    ret = NetworkConnect(&env->net,
                         P_VALUE(env->addr),
                         env->port);
    DBG("MQTT Connecting %s %d", P_VALUE(env->addr), env->port);
    if ( ret < 0 ) {
        DBG("MQTT Connecting fail %s %d [%d]",
            P_VALUE(env->addr),
            env->port,
            ret);
        return -1;
    }
    MQTTClientInit(&env->client,
                   &env->net,
                   env->timeout,
                   env->buf.buf, env->buf.size,
                   env->readbuf.buf, env->readbuf.size);
    ret = MQTTConnect(&env->client, &env->data);
    if ( ret != SUCCESS ) {
        DBG("MQTT Connect fail %d", ret);
        NetworkDisconnect(&env->net);
        return -1;
    }
    _mqtt_env_set_init(env, MQTT_CLIENT_INIT);
    return 0;
}

static int _mqtt_env_close(mqtt_env_t *env) {
    MQTTDisconnect(&env->client);
    NetworkDisconnect(&env->net);
    _mqtt_env_unset_init(env, MQTT_CLIENT_INIT);
    _mqtt_env_unset_init(env, MQTT_COMMANDS_INIT);
    return 0;
}

static int _mqtt_env_free(mqtt_env_t *env) {
    if ( _mqtt_env_is_init(env, MQTT_CLIENT_INIT) )
        _mqtt_env_close(env);
    _mqtt_deinit_common(env);
    _mqtt_env_unset_init(env, 0xffffffff);
    return 0;
}

#if !defined(NO_EVENTS)
static void messageArrived(MessageData* md) {
    MQTTMessage* message = md->message;
    char *nt_str = (char*)malloc(message->payloadlen+1);
    if ( !nt_str ) {
        DBG("Can't allocate more memory [%d]", message->payloadlen+1);
        return;
    }
    memcpy(nt_str, message->payload, message->payloadlen);
    nt_str[message->payloadlen] = 0x0;
    process_event(nt_str);
    free(nt_str);
}
#endif

static int mqttchannelseq( mqtt_env_t *ch, uint32_t num ) {
    if ( ch->mask == num ) return 0;
    return -1;
}

static uint32_t get_telemetry_mask() {
    int mqttmask = ACN_num;
#if defined(__IBM__)
    mqttmask = IBM_num;
#elif defined(__AZURE__)
    mqttmask = Azure_num;
#endif
    return mqttmask;
}

static mqtt_env_t *get_telemetry_env() {
    int mqttmask = get_telemetry_mask();
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          mqttmask );
    return tmp;
}

static mqtt_driver_t *get_telemetry_driver(uint32_t mqttmask) {
    mqtt_driver_t *drv = NULL;
    switch(mqttmask) {
    case ACN_num:
        drv = &iot_driver; break;
#if defined(__IBM__)
    case IBM_num: drv = &ibm_driver; break;
#endif
#if defined(__AZURE__)
    case Azure_num: drv = &azure_driver; break;
#endif
    default: drv = NULL;
    }
    return drv;
}

int mqtt_telemetry_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    int mqttmask = get_telemetry_mask();
    mqtt_driver_t *drv = get_telemetry_driver(mqttmask);
    i_args args = { device, gateway, config };
    mqtt_env_t *tmp = get_telemetry_env();
    if ( !tmp ) {
        tmp = (mqtt_env_t *)malloc(sizeof(mqtt_env_t));
        memset(tmp, 0x0, sizeof(sizeof(mqtt_env_t)));
        _mqtt_init_common(tmp);
        tmp->mask = mqttmask;
        arrow_linked_list_add_node_last(__mqtt_channels,
                                  mqtt_env_t,
                                  tmp);
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_COMMON_INIT) ) {
        drv->common_init(tmp, &args);
        _mqtt_env_set_init(tmp, MQTT_COMMON_INIT);
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
        int ret = drv->telemetry_init(tmp, &args);
        if ( ret < 0 ) {
            DBG("MQTT telemetry setting fail");
            return ret;
        }
        _mqtt_env_set_init(tmp, MQTT_TELEMETRY_INIT);
    }
    if ( !_mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
        int ret = _mqtt_env_connect(tmp);
        if ( ret < 0 ) return ret;
    }
    return 0;
}

int mqtt_telemetry_disconnect(void) {
    mqtt_env_t *tmp = get_telemetry_env();
    if ( !tmp ) {
        return -1;
    }
    if ( ! _mqtt_env_is_init(tmp,  MQTT_SUBSCRIBE_INIT) )
        _mqtt_env_close(tmp);
    else
        _mqtt_env_unset_init(tmp, MQTT_TELEMETRY_INIT);
    return 0;
}

int mqtt_telemetry_terminate(void) {
    mqtt_env_t *tmp = get_telemetry_env();
    if ( !tmp ) {
        return -1;
    }
    if ( ! _mqtt_env_is_init(tmp,  MQTT_SUBSCRIBE_INIT) ) {
        arrow_linked_list_del_node(__mqtt_channels, mqtt_env_t, tmp);
        _mqtt_env_free(tmp);
        free(tmp);
    } else
        _mqtt_env_unset_init(tmp, MQTT_TELEMETRY_INIT);
    return 0;
}

int mqtt_publish(arrow_device_t *device, void *d) {
    MQTTMessage msg = {MQTT_QOS, MQTT_RETAINED, MQTT_DUP, 0, NULL, 0};
    int ret = -1;
    mqtt_env_t *tmp = get_telemetry_env();
    if ( tmp ) {
        char *payload = telemetry_serialize(device, d);
        msg.payload = payload;
        msg.payloadlen = strlen(payload);
        ret = MQTTPublish(&tmp->client,
                          P_VALUE(tmp->p_topic),
                          &msg);
        free(payload);
    }
    return ret;
}

int mqtt_is_telemetry_connect(void) {
    mqtt_env_t *tmp = get_telemetry_env();
    if ( tmp ) {
        if ( _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT ) )
            return 1;
    }
    return 0;
}

void mqtt_disconnect(void) {
    mqtt_env_t *curr = NULL;
    arrow_linked_list_for_each ( curr, __mqtt_channels, mqtt_env_t ) {
        if ( _mqtt_env_is_init(curr, MQTT_CLIENT_INIT) ) _mqtt_env_close(curr);
    }
}

void mqtt_terminate(void) {
    mqtt_env_t *curr = NULL;
    arrow_linked_list_for_each_safe ( curr, __mqtt_channels, mqtt_env_t ) {
        if ( curr->init )
            _mqtt_env_free(curr);
        free(curr);
    }
    __mqtt_channels = NULL;
}

#if !defined(NO_EVENTS)
static mqtt_env_t *get_event_env() {
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          ACN_num );
    return tmp;
}

int mqtt_subscribe_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    mqtt_driver_t *drv = &iot_driver;
    i_args args = { device, gateway, config };
    mqtt_env_t *tmp = get_event_env();
    if ( !tmp ) {
        tmp = (mqtt_env_t *)calloc(1, sizeof(mqtt_env_t));
        _mqtt_init_common(tmp);
        tmp->mask = ACN_num;
        arrow_linked_list_add_node_last(__mqtt_channels,
                                  mqtt_env_t,
                                  tmp);
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_COMMON_INIT) ) {
        drv->common_init(tmp, &args);
        _mqtt_env_set_init(tmp, MQTT_COMMON_INIT);
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) ) {
        int ret = drv->commands_init(tmp, &args);
        if ( ret < 0 ) {
            DBG("MQTT subscribe setting fail");
            return ret;
        }
        _mqtt_env_set_init(tmp, MQTT_SUBSCRIBE_INIT);
    }
    if ( MQTTSetMessageHandler(&tmp->client,
                               P_VALUE(tmp->s_topic),
                               messageArrived) < 0 ) {
        DBG("MQTT handler fail");
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
        int ret = _mqtt_env_connect(tmp);
        if ( ret < 0 ) {
            return -1;
        }
    }
    return 0;
}

int mqtt_subscribe_disconnect(void) {
    mqtt_env_t *tmp = get_event_env();
    if ( !tmp ) {
        DBG("There is no sub channel");
        return -1;
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) )
        _mqtt_env_close(tmp);
    else
        _mqtt_env_unset_init(tmp, MQTT_SUBSCRIBE_INIT);
    return 0;
}

int mqtt_subscribe_terminate(void) {
    mqtt_env_t *tmp = get_event_env();
    if ( !tmp ) {
        return -1;
    }
    if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
        arrow_linked_list_del_node(__mqtt_channels, mqtt_env_t, tmp);
        _mqtt_env_free(tmp);
        free(tmp);
    } else
        _mqtt_env_unset_init(tmp, MQTT_SUBSCRIBE_INIT);
    return 0;
}

int mqtt_subscribe(void) {
    mqtt_env_t *tmp = get_event_env();
    if ( tmp &&
         _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) &&
         !_mqtt_env_is_init(tmp, MQTT_COMMANDS_INIT) ) {
        DBG("Subscribing to %s", P_VALUE(tmp->s_topic));
        int rc = MQTTSubscribe(&tmp->client,
                               P_VALUE(tmp->s_topic),
                               QOS2,
                               messageArrived);
        if ( rc < 0 ) {
            DBG("Subscribe failed %d\n", rc);
            return rc;
        }
        _mqtt_env_set_init(tmp, MQTT_COMMANDS_INIT);
    }
    return 0;
}

int mqtt_is_subscribe_connect(void) {
    mqtt_env_t *tmp = get_event_env();
    if ( tmp ) {
        if ( _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) return 1;
    }
    return 0;
}
#endif

int mqtt_yield(int timeout_ms) {
//    TimerInterval timer;
//    TimerInit(&timer);
//    TimerCountdownMS(&timer, (unsigned int)timeout_ms);
#if !defined(NO_EVENTS)
    int ret = -1;
    mqtt_env_t *tmp = get_event_env();
    if ( tmp &&
         _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) &&
         _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
//        do {
        ret = MQTTYield(&tmp->client, timeout_ms);
//        } while (!TimerIsExpired(&timer));
        return ret;
    }
    return -1;
#else
    msleep(timeout_ms);
    return 0;
#endif
}
