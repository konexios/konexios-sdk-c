/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/mqtt.h"
#include <config.h>
#include <arrow/sign.h>
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

static mqtt_env_t __mqtt_telemetry;
#if !defined(NO_EVENTS)
static mqtt_env_t *__mqtt_commands;
#endif

static int _mqtt_init_common(mqtt_env_t *env) {
    env->buf.size = MQTT_BUF_LEN;
    env->buf.buf = (unsigned char*)malloc(__mqtt_telemetry.buf.size);
    env->readbuf.size = MQTT_BUF_LEN;
    env->readbuf.buf = (unsigned char*)malloc(__mqtt_telemetry.readbuf.size);
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
    env->init = 1;
    return 0;
}

static int _mqtt_env_disconnect(mqtt_env_t *env) {
    MQTTDisconnect(&env->client);
    NetworkDisconnect(&env->net);
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

typedef struct _mqtt_driver {
    int (*telemetry_init)(mqtt_env_t *env, i_args *arg);
    int (*commands_init)(mqtt_env_t *env, i_args *arg);
    int (*common_init)(mqtt_env_t *env, i_args *arg);
} mqtt_driver_t;

#if defined(__IBM__)

typedef struct _ibm_args {
    arrow_device_t *device;
    arrow_gateway_config_t *config;
} ibm_args;

int mqtt_common_init_ibm(mqtt_env_t *env, i_args *args) {
    ibm_args *ibm = (ibm_args *)args->args;
    char username[256];
    char addr[100];

    char *organizationId = P_VALUE(ibm->config->organizationId);
    char *gatewayType = P_VALUE(ibm->config->gatewayType);
    char *gatewayId = P_VALUE(ibm->config->gatewayId);
    char *authToken = P_VALUE(ibm->config->authToken);
    char *deviceType = P_VALUE(ibm->device->type);
    char *externalId = P_VALUE(ibm->device->eid);
    if ( !organizationId || !strlen(organizationId) ) {
        DBG("There is no organizationId");
        return -1;
    }

#if 1
    // init gateway
    SSP_PARAMETER_NOT_USED(deviceType);
    SSP_PARAMETER_NOT_USED(externalId);
    int ret = snprintf(username, sizeof(username),
                       "g:%s:%s:%s",
                       organizationId, gatewayType, gatewayId);
#else
    // init device
    SSP_PARAMETER_NOT_USED(gatewayType);
    SSP_PARAMETER_NOT_USED(gatewayId);
    int ret = snprintf(username, sizeof(username),
                       "d:%s:%s:%s",
                       organizationId, deviceType, externalId);
#endif
    username[ret] = '\0';
    property_copy(&env->username, p_stack(username));
    DBG("username: %s", username);

    args->data->clientID.cstring = P_VALUE(env->username);
    args->data->username.cstring = "use-token-auth";
    args->data->password.cstring = authToken;

    ret = snprintf(addr, sizeof(addr),
                   "%s%s",
                   organizationId, MQTT_ADDR);
    addr[ret] = '\0';
    property_copy(&env->addr, p_stack(addr));
    return 0;
}

int mqtt_telemetry_init_ibm(mqtt_env_t *env, i_args *args) {
    SSP_PARAMETER_NOT_USED(args);
    // should be "iot-2/type/%s/id/%s/evt/telemetry/fmt/json"
    //                 deviceType, externalId
#if 1
    // init gateway
    ibm_args *ibm = (ibm_args *)args->args;
    char *gatewayType = P_VALUE(ibm->config->gatewayType);
    char *gatewayId = P_VALUE(ibm->config->gatewayId);
    char p_topic[256];
    snprintf(p_topic, sizeof(p_topic),
             "iot-2/type/%s/id/%s/evt/telemetry/fmt/json",
             gatewayType, gatewayId);
    property_copy(&env->p_topic, p_stack(p_topic));
#else
    property_copy(&env->p_topic, p_const("iot-2/evt/telemetry/fmt/json"));
#endif
    return 0;
}

int mqtt_subscribe_init_ibm(mqtt_env_t *env, i_args *args) {
    SSP_PARAMETER_NOT_USED(args);
    property_copy(&env->s_topic, p_const("iot-2/cmd/test/fmt/json"));
    return 0;
}

static mqtt_driver_t ibm_driver = {
    mqtt_telemetry_init_ibm,
    mqtt_subscribe_init_ibm,
    mqtt_common_init_ibm
};

#elif defined(__AZURE__)
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

#define USERNAME_LEN 80
#define S_TOP_NAME "krs/cmd/stg/"
#define P_TOP_NAME "krs.tel.gts."
#define S_TOP_LEN sizeof(S_TOP_NAME) + 66
#define P_TOP_LEN sizeof(P_TOP_NAME) + 66

typedef struct _iot_args {
    arrow_gateway_t *gateway;
} iot_args;

static int mqtt_common_init_iot(
        mqtt_env_t *env,
        i_args *args) {
    iot_args *iot = (iot_args *)args->args;
    CREATE_CHUNK(username, USERNAME_LEN);

    int ret = snprintf(username,
                       USERNAME_LEN,
                       "/pegasus:%s",
                       P_VALUE(iot->gateway->hid));
    if ( ret < 0 ) return -1;
    username[ret] = 0x0;
    property_copy(&env->username, p_stack(username));
    DBG("qmtt.username %s", username);

    args->data->clientID.cstring = P_VALUE(iot->gateway->hid);
    args->data->username.cstring = username;
    args->data->password.cstring = (char*)get_api_key();
    property_copy(&env->addr, p_const("mqtt-a01.arrowconnect.io"));

    FREE_CHUNK(username);

    return 0;
}

static int mqtt_telemetry_init_iot(
        mqtt_env_t *env,
        i_args *args) {
  iot_args *iot = (iot_args *)args->args;
  CREATE_CHUNK(p_topic, P_TOP_LEN);

  int ret = snprintf(p_topic,
                     P_TOP_LEN,
                     "%s%s",
                     P_TOP_NAME,
                     P_VALUE(iot->gateway->hid));
  if ( ret < 0 ) return -1;
  p_topic[ret] = 0x0;
  property_copy(&env->p_topic, p_stack(p_topic));

  FREE_CHUNK(p_topic);
  return 0;
}

static int mqtt_subscribe_init_iot(
        mqtt_env_t *env,
        i_args *args) {
    iot_args *iot = (iot_args *)args->args;
    CREATE_CHUNK(s_topic, S_TOP_LEN);
    int ret = snprintf(s_topic,
                       S_TOP_LEN,
                       "%s%s",
                       S_TOP_NAME,
                       P_VALUE(iot->gateway->hid));
    if ( ret < 0 ) return -1;
    s_topic[ret] = 0x0;
    property_copy(&env->s_topic, p_stack(s_topic));
    DBG("sub %s",  s_topic);
    FREE_CHUNK(s_topic);
    return 0;
}

static __attribute_used__ mqtt_driver_t iot_driver = {
    mqtt_telemetry_init_iot,
    mqtt_subscribe_init_iot,
    mqtt_common_init_iot
};

static int mqtt_connect(mqtt_driver_t *driver,
                        int ps,
                        mqtt_env_t *env,
                        i_args *args ) {
    if ( env->init && publish == ps ) return 0;
    else _mqtt_init_common(env);
    int rc = -1;
    driver->common_init(env, args);
    if ( ps == publish )
        rc = driver->telemetry_init(env, args);
    else
        rc = driver->commands_init(env, args);
    if ( rc < 0 ) {
        DBG("IBM telemetry setting fail");
        return rc;
    }
    return _mqtt_env_connect(env, args->data);
}

int mqtt_telemetry_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.keepAliveInterval = 10;
    data.cleansession = MQTT_CLEAN_SESSION;
    mqtt_driver_t *drv = NULL;
#if defined(__IBM__)
    drv = &ibm_driver;
    ibm_args ibmargs = { device, config };
    i_args args = { &data, &ibmargs };
#else
    drv = &iot_driver;
    iot_args iotargs = { gateway };
    i_args args = { &data, &iotargs };
#endif
    return mqtt_connect(drv, publish, &__mqtt_telemetry, &args);
}

#if !defined(NO_EVENTS)
int mqtt_subscribe_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.keepAliveInterval = 10;
    data.cleansession = MQTT_CLEAN_SESSION;

    if ( !__mqtt_commands ) {
#if !defined(__IBM__) && !defined(__AZURE__)
        DBG("---------------comd-----------");
        __mqtt_commands = &__mqtt_telemetry;
#else
        __mqtt_commands = (mqtt_env_t*)malloc(sizeof(mqtt_env_t));
        memset(__mqtt_commands, 0x0, sizeof(mqtt_env_t));
#endif
    }
    iot_args iotargs = { gateway };
    i_args args = { &data, &iotargs };

    return mqtt_connect(&iot_driver,
                        subscribe,
                        __mqtt_commands,
                        &args);
}
#endif

void mqtt_disconnect(void) {
    _mqtt_env_disconnect(&__mqtt_telemetry);
}

#if !defined(NO_EVENTS)
int mqtt_subscribe(void) {
    DBG("Subscribing to %s", P_VALUE(__mqtt_commands->s_topic));
    int rc = MQTTSubscribe(&__mqtt_commands->client,
                           P_VALUE(__mqtt_commands->s_topic),
                           QOS2,
                           messageArrived);
    if ( rc < 0 ) {
        DBG("Subscribe failed %d\n", rc);
    }
    return rc;
}

int mqtt_yield(int timeout_ms) {
  return MQTTYield(&__mqtt_commands->client, timeout_ms);
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
    char *payload = telemetry_serialize(device, d);
    msg.payload = payload;
    msg.payloadlen = strlen(payload);

    int ret = MQTTPublish(&__mqtt_telemetry.client,
                          P_VALUE(__mqtt_telemetry.p_topic),
                          &msg);
    free(payload);
    return ret;
}

int mqtt_is_connect() {
    return __mqtt_telemetry.client.isconnected;
}
