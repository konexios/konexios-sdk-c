/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <stdint.h>

// This is where out_msg_t is defined
#include <arrow/mqtt_out_msg.h>

#include "arrow/mqtt.h"
#include <config.h>
#include <mqtt/client/client.h>
#include <json/telemetry.h>
#include <data/property.h>
#include <arrow/events.h>
#include <debug.h>
#include <arrow/credentials.h>

#define MQTT_DBG(...)

#define USE_STATIC
#include <data/chunk.h>

// Determine if we clear topic handlers when we close
// the mqtt client, and if we send the 'clean session flag'
// to the broker
#if defined(NO_EVENTS)
  #define MQTT_CLEAN_SESSION 1
#else
  #define MQTT_CLEAN_SESSION 0
#endif

// Pointer to a linked list that holds all
// the mqtt_env_t 'channels'
static mqtt_env_t *__mqtt_channels = NULL;

static arrow_mqtt_delivery_callback_t base_event_callbacks = {
    p_static_null,
    process_event_init,
    process_event,
    process_event_finish,
    {NULL}
};

#if defined(HTTP_VIA_MQTT) && ACN_API_ENABLED
static arrow_mqtt_delivery_callback_t http_event_callbacks = {
    p_static_null,
    process_http_init,
    process_http,
    process_http_finish,
    {NULL}
};
#endif

#if defined(STATIC_MQTT_ENV)
  static uint8_t telemetry_recvbuf[MQTT_RECVBUF_LEN + 1];
  static uint8_t telemetry_buf[MQTT_BUF_LEN + 1];
  static mqtt_env_t static_telemetry_channel;
  #if !defined(NO_EVENTS)
    #if defined(MQTT_TWO_CHANNEL)
// Azure and IBM require a second receive buffer?
      static uint8_t command_recvbuf[MQTT_RECVBUF_LEN + 1];
      static uint8_t command_buf[MQTT_BUF_LEN + 1];
      static mqtt_env_t static_command_channel;
      static mqtt_env_t *static_command_channel_ptr = &static_command_channel;
    #else
      static mqtt_env_t *static_command_channel_ptr = &static_telemetry_channel;
      static uint8_t *command_recvbuf = telemetry_recvbuf;
      static uint8_t *command_buf = telemetry_buf;
    #endif
  #endif
#endif

// Define the external structures for the MQTT networks
extern mqtt_driver_t iot_driver;
#if defined(__IBM__)
  extern mqtt_driver_t ibm_driver;
#endif
#if defined(__AZURE__)
  extern mqtt_driver_t azure_driver;
#endif


// Private functions
// ---------------------------------------------------------------------------

// default the header data
static void data_prep(MQTTPacket_connectData *data) {
  MQTTPacket_connectData d = MQTTPacket_connectData_initializer;
  *data = d;
  data->cleansession = MQTT_CLEAN_SESSION;
  data->keepAliveInterval = 10;
}

// Initialize a 'channel'
static int _mqtt_init_common(mqtt_env_t *env) {
  property_init(&env->p_topic);
  property_init(&env->s_topic);
#if defined(HTTP_VIA_MQTT)
  property_init(&env->p_api_topic);
  property_init(&env->s_api_topic);
#endif
  property_init(&env->username);
  property_init(&env->addr);

    // Initialize the data for this 'channel'
  data_prep(&env->data);

    // If not static, malloc a receive buffer for
    // the channel
#if !defined(STATIC_MQTT_ENV)
    env->buf.size = MQTT_BUF_LEN;
    env->buf.buf = (unsigned char*)malloc(env->buf.size+1);
    env->readbuf.size = MQTT_RECVBUF_LEN;
    env->readbuf.buf = (unsigned char*)malloc(env->readbuf.size+1);
#endif
  env->timeout = DEFAULT_MQTT_TIMEOUT;
  env->port = arrow_mqtt_host()->port;
  env->init = 0;
  arrow_linked_list_init(env);
  return 0;
}

// Free a 'channel'
static int _mqtt_deinit_common(mqtt_env_t *env) {
#if defined(MQTT_TASK)
  arrow_mutex_deinit(env->client.mutex);
#endif
  property_free(&env->p_topic);
  property_free(&env->s_topic);
#if defined(HTTP_VIA_MQTT)
  property_free(&env->p_api_topic);
  property_free(&env->s_api_topic);
#endif
  property_free(&env->username);
  property_free(&env->addr);
#if !defined(STATIC_MQTT_ENV)
  free(env->buf.buf);
  free(env->readbuf.buf);
#endif
  env->buf.size = 0;
  env->readbuf.size = 0;
  return 0;
}

// init mask functions
static int _mqtt_env_is_init(mqtt_env_t *env, uint32_t init_mask) {
  int ret = 0;
  if ( env->init & init_mask ) {
    ret = 1;
  }
  return ret;
}

static void _mqtt_env_set_init(mqtt_env_t *env, uint32_t init_mask) {
  env->init |= init_mask;
}

static void _mqtt_env_unset_init(mqtt_env_t *env, uint32_t init_mask) {
  env->init &= ~init_mask;
}

// Get a new socket and connect to the server
static int _mqtt_env_connect(mqtt_env_t *env)
{
  int ret = -1;

  if (!P_VALUE(env->addr)) {
	  DBG("addr NULL");
	  return -1;
  }

  NetworkInit(&env->net);
  ret = NetworkConnect(&env->net,
                       P_VALUE(env->addr),
                       env->port,
                       env->timeout);
  DBG("MQTT Connecting %s %d", P_VALUE(env->addr), env->port);
  if ( ret < 0 ) {
    DBG("MQTT Connecting fail %s %d [%d]",
        P_VALUE(env->addr),
        env->port,
        ret);
    return -1;
  }

    // Initialize the client
  MQTTClientInit(&env->client,
                 &env->net,
                 env->timeout,
                 env->buf.buf, env->buf.size,
                 env->readbuf.buf, env->readbuf.size);

    // Connect to the broker
  ret = MQTTConnect(&env->client, &env->data);
  if ( ret != MQTT_SUCCESS ) {
    DBG("MQTT Connect fail %d", ret);
    NetworkDisconnect(&env->net);
    return -1;
  }

    // Mark client as initialized
  _mqtt_env_set_init(env, MQTT_CLIENT_INIT);


  return 0;
}

// Close the network connection and close the
// socket
static int _mqtt_env_close(mqtt_env_t *env) {
  MQTTDisconnect(&env->client);
  NetworkDisconnect(&env->net);
  _mqtt_env_unset_init(env, MQTT_CLIENT_INIT);
  _mqtt_env_unset_init(env, MQTT_COMMANDS_INIT);
  return 0;
}

// Close the connection and de-init the variables
static int _mqtt_env_free(mqtt_env_t *env) {
  if ( _mqtt_env_is_init(env, MQTT_CLIENT_INIT) ) {
    _mqtt_env_close(env);
  }
  _mqtt_deinit_common(env);
  _mqtt_env_unset_init(env, 0xffffffff);
  return 0;
}

static int _mqtt_env_pause(mqtt_env_t *env, int pause) {
    env->client.reject = pause ? 1 : 0;
    return 0;
}


// Checks mask == num. Used for searching
// a linked list for a channel 'type'
static int mqttchannelseq( mqtt_env_t *ch, uint32_t num ) {
  if ( ch->mask == num ) {
    return 0;
  }
  return -1;
}

// Get the type of connection??
static uint32_t get_telemetry_mask() {
  int mqttmask = ACN_num;
#if defined(__IBM__)
  mqttmask = IBM_num;
#elif defined(__AZURE__)
  mqttmask = Azure_num;
#endif
  return mqttmask;
}

// Get the current 'channel'
mqtt_env_t *get_telemetry_env()
{
    int mqttmask;
  mqtt_env_t *tmp = NULL;

    // Get the type
    mqttmask = get_telemetry_mask();

    // Find the channel in the linked list that
    // matches this mask
  linked_list_find_node(tmp,
                        __mqtt_channels,
                        mqtt_env_t,
                        mqttchannelseq,
                        mqttmask );

    // If we don't have an 'active' channel, make a new
    // one and add it to the linked list
  if ( !tmp ) {
#if defined(STATIC_MQTT_ENV)
    tmp = &static_telemetry_channel;
#else
    tmp = (mqtt_env_t *)calloc(1, sizeof(mqtt_env_t));
#endif

        // Initialize the 'channel'
    _mqtt_init_common(tmp);

        // Why isn't this in mqtt_init_common()??????
#if defined(STATIC_MQTT_ENV)
    tmp->buf.size = sizeof(telemetry_buf) - 1;
    tmp->buf.buf = (unsigned char *)telemetry_buf;
    tmp->readbuf.size = sizeof(telemetry_recvbuf) - 1;
    tmp->readbuf.buf = (unsigned char *)telemetry_recvbuf;
#endif

        // Mark this 'channel' with the type it is
        // Why isn't this in mqtt_init_common()??????
    tmp->mask = mqttmask;

        // Add to the linked list
    arrow_linked_list_add_node_last(__mqtt_channels,
                                    mqtt_env_t,
                                    tmp);
  }

    // return the pointer to the 'channel'
  return tmp;
}

// get the current 'driver' structure, depending on what
// kind of MQTT we are talking to.
static mqtt_driver_t *get_telemetry_driver(uint32_t mqttmask) {
  mqtt_driver_t *drv = NULL;

  switch(mqttmask) {
    case ACN_num:
      drv = &iot_driver;
      break;
#if defined(__IBM__)
    case IBM_num:
      drv = &ibm_driver;
      break;
#endif
#if defined(__AZURE__)
    case Azure_num:
      drv = &azure_driver;
      break;
#endif
    default:
      drv = NULL;
  }
  return drv;
}

// Public functions
// ---------------------------------------------------------------------------

// Arrow MQTT logic
int mqtt_telemetry_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {

    // Get the 'channel' and 'driver' struct, all initialized
    // and ready to go
  int mqttmask = get_telemetry_mask();
  mqtt_driver_t *drv = get_telemetry_driver(mqttmask);

    // Group the arguments into an i_args struct to be passed
    // to more init function
  i_args args = { device, gateway, config };
  mqtt_env_t *tmp = get_telemetry_env();
  if ( !tmp ) {
    DBG("Telemetry memory error");
    return -1;
  }

    // NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE
    /*
    // These are the init fucntions for the IOT driver
    // found in acn.c
    mqtt_driver_t iot_driver = {
        mqtt_telemetry_init_iot,    <---- telemetry_init
        mqtt_subscribe_init_iot,    <---- commands_init
        mqtt_common_init_iot        <---- common_init
    };
    */
    // NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE:NOTE


    // If not initialized, do the common_init() (Setup username and connection info)

  if ( ! _mqtt_env_is_init(tmp, MQTT_COMMON_INIT) ) {
    drv->common_init(tmp, &args);
    _mqtt_env_set_init(tmp, MQTT_COMMON_INIT);
  }

    // If the 'telemetry' is not initialized, set up
    // the publish topic
  if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
    int ret = drv->telemetry_init(tmp, &args);
    if ( ret < 0 ) {
      DBG("MQTT telemetry setting fail");
      return ret;
    }
#if defined(HTTP_VIA_MQTT)
    ret = drv->api_publish_init(tmp, &args);
    if ( ret < 0 ) {
      DBG("MQTT API publish setting fail");
      return ret;
    }
#endif

    _mqtt_env_set_init(tmp, MQTT_TELEMETRY_INIT);
  }
    // Do the actual connection to the server

  if ( !_mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
    int ret = _mqtt_env_connect(tmp);
    if ( ret < 0 ) {
      return ret;
    }
  }
  return 0;
}

// Mark the 'channel' as not initialized
int mqtt_telemetry_disconnect(void)
{
  mqtt_env_t *tmp = get_telemetry_env();
  if ( !tmp ) {
    return -1;
  }
    // If we are not subscribed, closed the 'channel'
  if ( ! _mqtt_env_is_init(tmp,  MQTT_SUBSCRIBE_INIT) ) {
    _mqtt_env_close(tmp);
  } else {
    _mqtt_env_unset_init(tmp, MQTT_TELEMETRY_INIT);
  }
  return 0;
}

// Stop the 'publish' topic logic
int mqtt_telemetry_terminate(void) {
  mqtt_env_t *tmp = get_telemetry_env();
  if ( !tmp ) {
    return -1;
  }
  if ( ! _mqtt_env_is_init(tmp,  MQTT_SUBSCRIBE_INIT) ) {
    arrow_linked_list_del_node(__mqtt_channels, mqtt_env_t, tmp);
    _mqtt_env_free(tmp);
#if !defined(STATIC_MQTT_ENV)
    free(tmp);
#endif
  } else {

        // If we are subscribed, just mark publish as not initialized???
    _mqtt_env_unset_init(tmp, MQTT_TELEMETRY_INIT);
  }
  return 0;
}

typedef struct mqtt_json_machine_ {
    JsonNode *mqtt_pub_pay;
    json_encode_machine_t em;
} mqtt_json_machine_t;

int mqtt_json_part_init(void *d) {
    mqtt_json_machine_t *mq = (mqtt_json_machine_t *)d;
    int len = json_size(mq->mqtt_pub_pay);
    if ( json_encode_init(&mq->em, mq->mqtt_pub_pay) < 0 ) {
        return -1;
    }
    return len;
}

int mqtt_json_part(void *d, char *ptr, int len) {
    mqtt_json_machine_t *mq = (mqtt_json_machine_t *)d;
    int r = json_encode_part(&mq->em, ptr, len);
    return r;
}

int mqtt_json_fin(void *d) {
    mqtt_json_machine_t *mq = (mqtt_json_machine_t *)d;
    int r = json_encode_fin(&mq->em);
    return r;
}

mqtt_payload_drive_t mqtt_json_drive = {
    mqtt_json_part_init,
    mqtt_json_part,
    mqtt_json_fin,
    NULL
};


/**
 * @brief
 * @b Purpose:	Client to broker PUBLISH. Send JSON data to broker
 * \n           and setup transaction if required by QOS level.
 *
 * @param[in]	device: Feeder specific identifiers
 * @param[in]	d: outbound data (of type out_msg_t) to be serialized.
 *
 *
 * @return MQTT_SUCCESS on good send, else FAILURE.
 */
// Send the data to the publish topic
int mqtt_publish(arrow_device_t *device, void *d) {
    static mqtt_json_machine_t mqtt_json_payload;
    out_msg_t *data = (out_msg_t*)d;
    MQTTMessage msg = {MQTT_QOS, MQTT_RETAINED, MQTT_ORIGINAL, 0, NULL, 0};
    int ret = FAILURE;

    mqtt_env_t *tmp = get_telemetry_env();
    if ( tmp ) {
        if (data->packetId > 0) {
        	//this is a retransmission.
        	msg.id = data->packetId;
        	msg.dup = 1;
        }

        mqtt_json_payload.mqtt_pub_pay = telemetry_serialize_json(device, d);
        mqtt_json_drive.data = (void*)&mqtt_json_payload;
        ret = MQTTPublish_part(&tmp->client,
                               P_VALUE(tmp->p_topic),
                               &msg,
                               &mqtt_json_drive);
        json_delete(mqtt_json_payload.mqtt_pub_pay);

        // retain the packetId for retries.
        data->packetId = msg.id;
    }
    return ret;
}

#if defined(HTTP_VIA_MQTT)
int mqtt_api_publish(JsonNode *data) {
    static mqtt_json_machine_t mqtt_json_payload;
  MQTTMessage msg = {MQTT_QOS, MQTT_RETAINED, MQTT_ORIGINAL, 0, NULL, 0};
  int ret = -1;
  mqtt_env_t *tmp = get_telemetry_env();
  if ( tmp ) {
    if ( !data ) {
      return -1;
    }
    mqtt_json_payload.mqtt_pub_pay = data;
    mqtt_json_drive.data = (void*)&mqtt_json_payload;
    msg.payload = (void *)data;
    msg.payloadlen = json_size(data);
    MQTT_DBG("[%d][%s]", msg.payloadlen, msg.payload);
    ret = MQTTPublish_part(&tmp->client,
                      P_VALUE(tmp->p_api_topic),
                      &msg,
                           &mqtt_json_drive);
  }
  return ret;
}
#endif

int mqtt_is_telemetry_connect(void) {
  mqtt_env_t *tmp = get_telemetry_env();
  if ( tmp ) {
    if ( _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT ) &&
         _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
      return 1;
    }
  }
  return 0;
}

// Close all channels
void mqtt_disconnect(void) {
  mqtt_env_t *curr = NULL;
  arrow_linked_list_for_each ( curr, __mqtt_channels, mqtt_env_t ) {
    if ( _mqtt_env_is_init(curr, MQTT_CLIENT_INIT) ) {
      _mqtt_env_close(curr);
    }
  }
}

// Free all channels
void mqtt_terminate(void) {
  mqtt_env_t *curr = NULL;
  arrow_linked_list_for_each_safe ( curr, __mqtt_channels, mqtt_env_t ) {
    if ( curr->init ) {
      _mqtt_env_free(curr);
    }
#if !defined(STATIC_MQTT_ENV)
    free(curr);
#endif
  }
    // Mark the linked list as NULL
  __mqtt_channels = NULL;
}

void mqtt_pause(int pause) {
    mqtt_env_t *curr = NULL;
    arrow_linked_list_for_each ( curr, __mqtt_channels, mqtt_env_t ) {
        _mqtt_env_pause(curr, pause);
    }
}

#if !defined(NO_EVENTS)
static mqtt_env_t *get_event_env() {
  mqtt_env_t *tmp = NULL;
  linked_list_find_node(tmp,
                        __mqtt_channels,
                        mqtt_env_t,
                        mqttchannelseq,
                        ACN_num );
  if ( !tmp ) {
#if defined(STATIC_MQTT_ENV)
    tmp = static_command_channel_ptr;
#else
    tmp = (mqtt_env_t *)calloc(1, sizeof(mqtt_env_t));
#endif
    _mqtt_init_common(tmp);
#if defined(STATIC_MQTT_ENV)
    tmp->buf.size = MQTT_BUF_LEN;
    tmp->buf.buf = (unsigned char *)command_buf;
    tmp->readbuf.size = MQTT_RECVBUF_LEN;
    tmp->readbuf.buf = (unsigned char *)command_recvbuf;
#endif
    tmp->mask = ACN_num;
    arrow_linked_list_add_node_last(__mqtt_channels,
                                    mqtt_env_t,
                                    tmp);
  }
  return tmp;
}


// Setup and subscribe to the receive topic
int mqtt_subscribe_connect(arrow_gateway_t *gateway,
                           arrow_device_t *device,
                           arrow_gateway_config_t *config) {

    // Get the 'driver' and the 'channel' for the subscription
  mqtt_driver_t *drv = &iot_driver;
  i_args args = { device, gateway, config };
  mqtt_env_t *tmp = get_event_env();
  if ( !tmp ) {
    return -1;
  }
  if ( ! _mqtt_env_is_init(tmp, MQTT_COMMON_INIT) ) {
    drv->common_init(tmp, &args);
    _mqtt_env_set_init(tmp, MQTT_COMMON_INIT);
  }

    // Set up the receive topic.  In this design there is only one
    // subscribe topic.  What a pile.
  if ( ! _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) ) {
    int ret = drv->commands_init(tmp, &args);
    if ( ret < 0 ) {
      DBG("MQTT subscribe setting fail");
      return ret;
    }

#if (ACN_API_ENABLED)//defined(HTTP_VIA_MQTT)
    ret = drv->api_subscribe_init(tmp, &args);
    if ( ret < 0 ) {
      DBG("MQTT API subscribe setting fail");
      return ret;
    }
#endif
    _mqtt_env_set_init(tmp, MQTT_SUBSCRIBE_INIT);
  }
  property_weak_copy(&base_event_callbacks.topic, tmp->s_topic);
  if ( arrow_mqtt_client_delivery_message_reg(&tmp->client, &base_event_callbacks) < 0 ) {
      DBG("MQTT handler fail");
  }

#if (ACN_API_ENABLED) //defined(HTTP_VIA_MQTT)
  property_weak_copy(&http_event_callbacks.topic, tmp->s_api_topic);
  if ( arrow_mqtt_client_delivery_message_reg(&tmp->client, &http_event_callbacks) < 0 ) {
      DBG("MQTT API handler fail");
  }
#endif

  if ( ! _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
    int ret = _mqtt_env_connect(tmp);
    if ( ret < 0 ) {
      return -1;
    }
  }

    // Done
  return 0;
}


// Close the subscribe logic
int mqtt_subscribe_disconnect(void) {
  mqtt_env_t *tmp = get_event_env();
  if ( !tmp ) {
    DBG("There is no sub channel");
    return -1;
  }
  if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
    _mqtt_env_close(tmp);
  } else {
    _mqtt_env_unset_init(tmp, MQTT_SUBSCRIBE_INIT);
  }
  return 0;
}

// Close all channels that are only receiving
int mqtt_subscribe_terminate(void) {
  mqtt_env_t *tmp = get_event_env();
  if ( !tmp ) {
    return -1;
  }
  if ( ! _mqtt_env_is_init(tmp, MQTT_TELEMETRY_INIT) ) {
    arrow_linked_list_del_node(__mqtt_channels, mqtt_env_t, tmp);
    _mqtt_env_free(tmp);

#if !defined(STATIC_MQTT_ENV)
    free(tmp);
#endif
  } else {
    _mqtt_env_unset_init(tmp, MQTT_SUBSCRIBE_INIT);
  }
  return 0;
}

// Send the the actual subscribe packets
int mqtt_subscribe(void) {
  mqtt_env_t *tmp = get_event_env();
  if ( tmp &&
       _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) &&
       !_mqtt_env_is_init(tmp, MQTT_COMMANDS_INIT) ) {

	  char *st = "(null)";
	  if (P_VALUE(tmp->s_topic)) {
		  st = P_VALUE(tmp->s_topic);
	  }
    DBG("Subscribing to %s", st);
    int rc = arrow_mqtt_client_subscribe(&tmp->client,
                           QOS2,
                           &base_event_callbacks);
    if ( rc < 0 ) {
      DBG("Subscribe failed %d\n", rc);
      return rc;
    }
#if (ACN_API_ENABLED)//defined(HTTP_VIA_MQTT)
    DBG("Subscribing API to %s", P_VALUE(tmp->s_api_topic));
    rc = arrow_mqtt_client_subscribe(&tmp->client,
                               QOS2,
                               &http_event_callbacks);
    if ( rc < 0 ) {
      DBG("Subscribe API failed %d\n", rc);
    }
#endif
    _mqtt_env_set_init(tmp, MQTT_COMMANDS_INIT);
  }
  return 0;
}

// Check to see if we are connected to the server?
int mqtt_is_subscribe_connect(void) {
  mqtt_env_t *tmp = get_event_env();
  if ( tmp ) {
    if ( _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
      return 1;
    }
  }
  return 0;
}
#endif


int mqtt_yield(int timeout_ms) {
#if !defined(NO_EVENTS)
  int ret = -1;
  mqtt_env_t *tmp = get_event_env();
  if ( tmp &&
       _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) &&
       _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
    ret = MQTTYield(&tmp->client, timeout_ms);
    return ret;
  }
  return -1;
#else
  msleep(timeout_ms);
  return 0;
#endif
}

#if !defined(NO_EVENTS)
int mqtt_receive(int timeout_ms) {
  TimerInterval timer;
  TimerInit(&timer);
  TimerCountdownMS(&timer, (unsigned int)timeout_ms);

  int ret = -1;
  mqtt_env_t *tmp = get_event_env();
  if ( tmp &&
       _mqtt_env_is_init(tmp, MQTT_SUBSCRIBE_INIT) &&
       _mqtt_env_is_init(tmp, MQTT_CLIENT_INIT) ) {
    do {
      ret = MQTTYield(&tmp->client, TimerLeftMS(&timer));
    } while (!TimerIsExpired(&timer));
    return ret;
  }
  return -1;
}
#endif

// EOF
