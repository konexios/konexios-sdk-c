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
extern mqtt_driver_t ibm_driver;

static int _mqtt_init_common(mqtt_env_t *env) {
    env->buf.size = MQTT_BUF_LEN;
    env->buf.buf = (unsigned char*)malloc(env->buf.size);
    env->readbuf.size = MQTT_BUF_LEN;
    env->readbuf.buf = (unsigned char*)malloc(env->readbuf.size);
    env->timeout = 3000;
    env->port = MQTT_PORT;
    return 0;
}

static int _mqtt_deinit_common(mqtt_env_t *env) {
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

static int _mqtt_env_connect(mqtt_env_t *env, MQTTPacket_connectData *data) {
    int ret = -1;
    if ( env->init & MQTT_CLIENT_INIT ) return 0;
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
    ret = MQTTConnect(&env->client, data);
    if ( ret != MQTT_SUCCESS ) {
        DBG("MQTT Connect fail %d", ret);
        NetworkDisconnect(&env->net);
        return -1;
    }
    env->init |= MQTT_CLIENT_INIT;
    return 0;
}

static int _mqtt_env_close(mqtt_env_t *env) {
    MQTTDisconnect(&env->client);
    NetworkDisconnect(&env->net);
    env->init &= ~MQTT_CLIENT_INIT;
    env->init &= ~MQTT_COMMANDS_INIT;
    return 0;
}

static int _mqtt_env_free(mqtt_env_t *env) {
    if ( env->init & MQTT_CLIENT_INIT ) {
        _mqtt_env_close(env);
    }
    _mqtt_deinit_common(env);
    env->init = 0;
    return 0;
}

#if !defined(NO_EVENTS)
static void messageArrived(MessageData* md) {
    MQTTMessage* message = md->message;
    DBG("mqtt msg arrived %u", message->payloadlen);
    char *nt_str = (char*)malloc(message->payloadlen+1);
    if ( !nt_str ) {
        DBG("Can't allocate more memory [%d]", message->payloadlen+1);
        return;
    }
    memcpy(nt_str, message->payload, message->payloadlen);
    nt_str[message->payloadlen] = 0x0;
#if defined(SINGLE_SOCKET)
    mqtt_disconnect();
#endif
    process_event(nt_str);
    free(nt_str);
}
#endif

#if defined(__AZURE__)
#include <time/time.h>
#include <arrow/utf8.h>
#include <crypt/crypt.h>
#include "wolfssl/wolfcrypt/coding.h"
#ifndef SHA256_DIGEST_SIZE
#define SHA256_DIGEST_SIZE 32
#endif


static int sas_token_gen(char *sas, char *devname, char *key, char *time_exp) {
  char *dev = MQTT_ADDR "/devices/";
  char common[256];
  strcpy(common, dev);
  strcat(common, devname);
  strcat(common, "\n");
  strcat(common, time_exp);
  DBG("common string <%s>", common);
  char hmacdig[SHA256_DIGEST_SIZE];

  char decoded_key[100];
  int decoded_key_len = 100;
  DBG("decode key %d {%s}\r\n", strlen(key), key);
  int ret = Base64_Decode((byte*)key, (word32)strlen(key), (byte*)decoded_key, (word32*)&decoded_key_len);
  if ( ret ) return -1;

  hmac256(hmacdig, decoded_key, decoded_key_len, common, strlen(common));

  decoded_key_len = 100;
  ret = Base64_Encode((byte*)hmacdig, SHA256_DIGEST_SIZE, (byte*)decoded_key, (word32*)&decoded_key_len);
  if ( ret ) return -1;
  decoded_key[decoded_key_len] = 0x0;

  urlencode(sas, decoded_key, decoded_key_len-1);

  DBG("enc hmac [%s]\r\n", decoded_key);
  return 0;
}

static int mqtt_connect_azure(arrow_gateway_t *gateway,
                       arrow_device_t *device,
                       arrow_gateway_config_t *config) {
  SSP_PARAMETER_NOT_USED(device);
  char username[100];
  strcpy(username, config->host);
  strcat(username, "/");
  strcat(username, gateway->uid);
  strcat(username, "/");
  strcat(username, "api-version=2016-11-14&DeviceClientType=iothubclient%2F1.1.7");
  DBG("qmtt.username %s", username);
  char time_exp[32];
  int time_len = sprintf(time_exp, "%ld", time(NULL) + 3600);
  time_exp[time_len] = '\0';

  strcpy(s_topic, "devices/");
  strcat(s_topic, gateway->uid);
  strcat(s_topic, "/messages/events/");

  strcpy(p_topic, "devices/");
  strcat(p_topic, gateway->uid);
  strcat(p_topic, "/messages/events/");

  char pass[256];
  char sas[128];
  if ( sas_token_gen(sas, gateway->uid, config->accessKey, time_exp) < 0) {
    DBG("Fail SAS");
    return -2;
  }
  DBG("SAS: {%s}\r\n", sas);
  strcpy(pass, "SharedAccessSignature sr=");
  strcat(pass, config->host);
  strcat(pass, "/devices/");
  strcat(pass, gateway->uid);
  strcat(pass, "&sig=");
  strcat(pass, sas);
  strcat(pass, "&se=");
  strcat(pass, time_exp);
  strcat(pass, "&skn=");

  DBG("pass: %s\r\n", pass);

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.will.qos = 1;
  data.MQTTVersion = 4;
  data.clientID.cstring = gateway->uid;
  data.username.cstring = username;
  data.password.cstring = pass;
  data.keepAliveInterval = 240;
  data.cleansession = 0;

  int rc;
  NetworkInit(&mqtt_net);
  char mqtt_addr[100];
  strcpy(mqtt_addr, MQTT_ADDR);
  DBG("azure addr: (%d)%s", strlen(mqtt_addr), mqtt_addr);
  rc = NetworkConnect(&mqtt_net, mqtt_addr, MQTT_PORT);
  DBG("Connecting to %s %d", mqtt_addr, MQTT_PORT);
  if ( rc < 0 ) return rc;
  MQTTClientInit(&mqtt_client, &mqtt_net, 3000, buf, MQTT_BUF_LEN, readbuf, MQTT_BUF_LEN);
  rc = MQTTConnect(&mqtt_client, &data);
  DBG("Connected %d", rc);
  if ( rc != MQTT_SUCCESS ) return FAILURE;
  return rc;
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

static mqtt_driver_t *get_telemetry_driver(uint32_t mqttmask) {
    mqtt_driver_t *drv = NULL;
    switch(mqttmask) {
    case ACN_num: drv = &iot_driver; break;
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

static void data_prep(MQTTPacket_connectData *data) {
    data->willFlag = 0;
    data->MQTTVersion = 3;
    data->keepAliveInterval = 10;
    data->cleansession = MQTT_CLEAN_SESSION;
}

int mqtt_telemetry_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    int mqttmask = get_telemetry_mask();
    mqtt_driver_t *drv = get_telemetry_driver(mqttmask);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data_prep(&data);
    i_args args = { &data, device, gateway, config };
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          mqttmask );
    if ( !tmp ) {
        tmp = (mqtt_env_t *)malloc(sizeof(mqtt_env_t));
        _mqtt_init_common(tmp);
        tmp->mask = mqttmask;
        linked_list_add_node_last(__mqtt_channels,
                                  mqtt_env_t,
                                  tmp);
        if ( ! ( tmp->init & MQTT_COMMON_INIT ) ) {
            drv->common_init(tmp, &args);
            tmp->init |= MQTT_COMMON_INIT;
        }
        if ( ! ( tmp->init & MQTT_TELEMETRY_INIT ) ) {
            int ret = drv->telemetry_init(tmp, &args);
            if ( ret < 0 ) {
                DBG("MQTT telemetry setting fail");
                return ret;
            }
            tmp->init |= MQTT_TELEMETRY_INIT;
        }
    }
    if ( tmp->init & MQTT_CLIENT_INIT ) return 0;
    return _mqtt_env_connect(tmp, &data);
}

#if !defined(NO_EVENTS)
int mqtt_subscribe_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    mqtt_driver_t *drv = &iot_driver;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data_prep(&data);
    i_args args = { &data, device, gateway, config };
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          ACN_num );
    if ( !tmp ) {
        tmp = (mqtt_env_t *)malloc(sizeof(mqtt_env_t));
        _mqtt_init_common(tmp);
        tmp->mask = ACN_num;
        linked_list_add_node_last(__mqtt_channels,
                                  mqtt_env_t,
                                  tmp);
        if ( ! ( tmp->init & MQTT_COMMON_INIT ) ) {
            drv->common_init(tmp, &args);
            tmp->init |= MQTT_COMMON_INIT;
        }
        if ( ! ( tmp->init & MQTT_SUBSCRIBE_INIT ) ) {
            int ret = drv->commands_init(tmp, &args);
            if ( ret < 0 ) {
                DBG("MQTT subscribe setting fail");
                return ret;
            }
            tmp->init |= MQTT_SUBSCRIBE_INIT;
        }
    }
    if ( tmp->init & MQTT_CLIENT_INIT ) return 0;
    return _mqtt_env_connect(tmp, &data);
}

int mqtt_subscribe_disconnect(void) {
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          ACN_num );
    if ( !tmp ) return -1;
    _mqtt_env_close(tmp);
    return 0;
}
#endif

int mqtt_telemetry_disconnect(void) {
    int mqttmask = get_telemetry_mask();
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          mqttmask );
    if ( !tmp ) return -1;
    _mqtt_env_close(tmp);
    return 0;
}

void mqtt_close(void) {
    mqtt_env_t *curr = NULL;
    for_each_node_hard ( curr, __mqtt_channels, mqtt_env_t ) {
        if ( curr->init & MQTT_CLIENT_INIT )
            _mqtt_env_free(curr);
        free(curr);
    }
}

#if !defined(NO_EVENTS)
int mqtt_subscribe(void) {
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          ACN_num );
    if ( tmp && !(tmp->init & MQTT_COMMANDS_INIT) ) {
        DBG("Subscribing to %s", P_VALUE(tmp->s_topic));
        int rc = MQTTSubscribe(&tmp->client,
                               P_VALUE(tmp->s_topic),
                               QOS2,
                               messageArrived);
        if ( rc < 0 ) {
            DBG("Subscribe failed %d\n", rc);
            return rc;
        }
        tmp->init |= MQTT_COMMANDS_INIT;
    }
    return 0;
}

int mqtt_yield(int timeout_ms) {
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          ACN_num );
    if ( tmp && tmp->init & MQTT_SUBSCRIBE_INIT ) {
            return MQTTYield(&tmp->client, timeout_ms);
    }
    return -1;
}
#endif

int mqtt_publish(arrow_device_t *device, void *d) {
    MQTTMessage msg = {
        MQTT_QOS,
        MQTT_RETAINED,
        MQTT_DUP,
        0,
        NULL,
        0
    };
    int mqttmask = ACN_num;
#if defined(__IBM__)
    mqttmask = IBM_num;
#endif
    mqtt_env_t *tmp = NULL;
    linked_list_find_node(tmp,
                          __mqtt_channels,
                          mqtt_env_t,
                          mqttchannelseq,
                          mqttmask );
    int ret = -1;
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

int mqtt_is_connect() {
    return 1;//__mqtt_telemetry.client.isconnected;
}
