/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <MQTTClient.h>
#include <mqtt/client/client.h>
#include <debug.h>
#include "konexios/transaction.h"

typedef struct mqtthead {
    MQTTHeader head;
    uint32_t rem_len;
} mqtt_head_t;

typedef int(*_cycle_callback)(MQTTClient* c, mqtt_head_t *msg, TimerInterval* timer);

extern int decodePacketLength(MQTTClient *c, int *value, int timeout);
extern int keepalive(MQTTClient *c);
extern int getNextPacketId(MQTTClient *c);
extern int waitfor(MQTTClient *c, int packet_type, TimerInterval *timer);

static int cycle_connack(MQTTClient* c, mqtt_head_t *msg, TimerInterval* timer) {
	//DBG(" "); //debug for factory ota fail
    int rc = MQTT_SUCCESS;
    if ( (rc = c->ipstack->mqttread(c->ipstack,
                                    c->readbuf,
                                    msg->rem_len,
                                    TimerLeftMS(timer)) != (int)msg->rem_len) ) {
        return FAILURE;
    }
    return MQTT_SUCCESS;
}

int cycle_r(MQTTClient* c, TimerInterval* timer);

int waitfor_r(MQTTClient *c, int packet_type, TimerInterval *timer) {
  int rc = FAILURE;
  while ( !TimerIsExpired(timer) && rc != packet_type && rc >= 0) {
    rc = cycle_r(c, timer);
  }

  return rc;
}

/**
 * @brief
 * @b Purpose:	Handle an MQTT publish packet from the broker and pass to the
 * \n           appropriate app handler as registered with konexios_command_handler_add( )
 * \n			e.g. feed_cmd(), schedule_add(), schedule_clear()...
 *
 * @param[in]	c: pointer to an MQTT client, info for receive buffer and channel.
 * @param[in]	m:  pointer to the MQTT header previously decoded for this incoming packet.
 * @param[in]	timer:  pointer to a timer initialized with the maximum time allowed to receive the packet
 *
 * @return MQTT_SUCCESS on
 */
static int cycle_publish(MQTTClient* c, mqtt_head_t *m, TimerInterval* timer) {
    int rc = MQTT_SUCCESS;
    int rest_buffer = c->readbuf_size;
    MQTTMessage msg = { 0, 0, 0, 0, 0, 0 };
    msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
    msg.dup = m->head.bits.dup;
    msg.qos = (enum QoS) m->head.bits.qos;
    msg.retained = m->head.bits.retain;

    int total_len = m->rem_len;

    if ( c->reject ) {
        unsigned char dummy[16];
        while ( total_len ) {
            int chunk = KONEXIOS_MIN(total_len, (int)sizeof(dummy));
            int r = c->ipstack->mqttread(c->ipstack,
                                         dummy,
                                         chunk,
                                         TimerLeftMS(timer));
            if ( r > 0 ) total_len -= r;
            else break;
        }
        DBG("ERROR --> Rejecting Packet.");
        return FAILURE;
    }

    int len = c->ipstack->mqttread(c->ipstack,
                                   c->readbuf,
                                   2,
                                   TimerLeftMS(timer));
    if ( len <= 0 ) {
        DBG("ERROR --> Rejecting Packet.");
        return FAILURE;
    }
    total_len -= len;

    MQTTString topicName = MQTTString_initializer;
    unsigned char* curdata = c->readbuf;

    topicName.lenstring.len = readInt(&curdata);

    if ( topicName.lenstring.len >= total_len ) {
        // too short buffer or broken packet
        DBG("ERROR --> Too short buffer or broken packet.");
        return FAILURE;
    }

    len = c->ipstack->mqttread(c->ipstack,
                                   c->readbuf,
                                   topicName.lenstring.len,
                                   TimerLeftMS(timer));
    if ( len != topicName.lenstring.len ) {
        DBG("ERROR --> Failed to read the topic length.");
        return FAILURE;
    }
    total_len -= len;
    rest_buffer -= len;

    topicName.lenstring.data = (char*)c->readbuf;

    if ( rest_buffer <= 0 ) {
        DBG("ERROR --> read buffer ran out of space!");
        return FAILURE;
    }

    if (msg.qos > 0) {
        uint8_t tmp[2];
        len = c->ipstack->mqttread(c->ipstack,
                                           tmp,
                                           2,
                                           TimerLeftMS(timer));
        if ( len != 2 ) {
            DBG("ERROR --> Failed to read message id when processing inbound mqtt.");
            return FAILURE;
        }

        total_len -= len;
        curdata = tmp;
        msg.id = readInt(&curdata);
    }

    if(msg.dup == 0)
    {
    	DBG("PUBLISH ID %d:, retain %d", msg.id, msg.retained);
    }
    else
    {
    	DBG("*** RE-PUBLISH ID %d: dup %d, retain %d ***", msg.id, msg.dup, msg.retained);
    }

    msg.payloadlen = total_len;
    msg.payload = c->readbuf + topicName.lenstring.len;

    int shift = 0;

    int msg_err = konexios_mqtt_client_delivery_message_init(c, &topicName, &msg);

    while( total_len ) {
        int chunk = total_len < rest_buffer ? total_len : rest_buffer;
        len = c->ipstack->mqttread(c->ipstack,
                                       (unsigned char*)msg.payload,
                                       chunk,
                                       TimerLeftMS(timer));
        if ( len <= 0 ) {
            DBG("ERROR --> Failed to read mqtt payload.  Disconnecting and Returning ERROR.");
            return FAILURE;
        } else {
            total_len -= len;
            shift += len;
            msg.payloadlen = len;
            if ( !msg_err ) {
                msg_err = konexios_mqtt_client_delivery_message_process(c, &topicName, &msg);
            }
        }
    }

    if ( !msg_err ) {
        msg_err = konexios_mqtt_client_delivery_message_done(c, &topicName, &msg);
    } else {
        DBG("ERROR --> Arrow delivery message done error.  Will not send ACK!");
    }

    // don't send ack if error
    if ( !msg_err ) {
    	rc = MQTTBuildAck(c, &msg, PUBLISH, timer);
    }
    return rc;
}

static int cycle_puback(MQTTClient* c, mqtt_head_t *m, TimerInterval* timer) {
    int rc = MQTT_SUCCESS;
    int total_len = m->rem_len;

    //DBG("RX %d : PUBACK with QoS %d and %d rem_len", m->head.bits.type, m->head.bits.qos, total_len);

    uint8_t buf[2] = {0};
    unsigned char* msgIdP = buf;
    if (total_len > 0 && total_len < 3) {
    	int len = c->ipstack->mqttread(c->ipstack,
                                   buf,
                                   total_len,
                                   TimerLeftMS(timer));
		if ( len <= 0 )
		{
			rc = FAILURE;
		}
		else
		{
		    int receivedId = readInt(&msgIdP);
		    transaction_match(receivedId, PUBLISH);

			DBG("PUBACK msgId: %d", receivedId);
		}
    }
    else {
    	DBG("\trem_len %d invalid", total_len);
    	rc = FAILURE;
    }
    return rc;
}

static int cycle_subscribe(MQTTClient* c, mqtt_head_t *m, TimerInterval* timer) {
    int rc = MQTT_SUCCESS;

    int total_len = m->rem_len;

    unsigned char *ptr = c->readbuf;
//    writeInt(&ptr, total_len);

    int len = c->ipstack->mqttread(c->ipstack,
                                   ptr,
                                   total_len,
                                   TimerLeftMS(timer));
    if ( len <= 0 ) {
        return FAILURE;
    }
    return rc;
}

static int cycle_ping(MQTTClient* c, mqtt_head_t *m, TimerInterval* timer) {
    SSP_PARAMETER_NOT_USED(m);
    SSP_PARAMETER_NOT_USED(timer);
    c->ping_outstanding = 0;
    return MQTT_SUCCESS;
}

_cycle_callback __cycle_collection[] = {
    NULL, //reserved
    NULL,
    cycle_connack,
    cycle_publish,
	cycle_puback,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
    NULL,
	NULL,
    cycle_ping,
	NULL,
	//NULL	//reserved
};

/**
 * @brief
 * @b Purpose:	Monitor the receive buffer for inbound MQTT messages and pass a
 * \n           valid packet to the appropriate receive handler in cycle_collection[]
 *
 * @param[in]	c: pointer to an MQTT client, info for receive buffer and channel.
 * @param[in]	timeout:  maximum time in msec to wait while reading incoming stream.
 *
 * @return the type of MQTT packet received on a valid packet, else an appropriate failure code (-ve)
 */
int cycle_r(MQTTClient* c, TimerInterval* timer) {
    mqtt_head_t header = {{0}, 0};
    int len = 0;
    int rem_len = 0;
    int rc = FAILURE;
    uint32_t pack_type = 0;
    uint8_t tmp[16];
//    DBG("xxxxxxxxxxx Entry. xxxxxxxxxxx")
    // Read MQTT packet Control Header (1 Byte)
    rc = c->ipstack->mqttread(c->ipstack, tmp, 1, TimerLeftMS(timer));
    if (rc <= 0)
    {
        if(rc == -1) {
        	DBG("MQTT read empty (%d)", rc);
        }else {
        	DBG("MQTT read rc: %d", rc);
        }
        rc = FAILURE;
        goto exit;
    }
    // Received an mqtt header byte. Reload receive timer.
    TimerCountdownMS(timer, MQTT_RECEIVE_TIMEOUT);

    header.head.byte = tmp[0];
    pack_type = (uint32_t)header.head.bits.type;
    DBG("mqtt recv header message type %d", (int)pack_type);
    if ( pack_type < 1 || pack_type >= sizeof(__cycle_collection)/sizeof(_cycle_callback) ) {
        DBG("Control Header Data Error, pack_type = %lu", (long unsigned int)pack_type);
        if(pack_type < CONNECT) {
            DBG("pack_type Forbidden/Reserved");
        }
        else {
            DBG("pack_type > #of handlers in __cycle_collection");
        }
//        rc = FAILURE;
//        goto exit;
    }

    // Read MQTT packet Packet Length (1-4 Bytes)
    len = decodePacketLength(c, &rem_len, TimerLeftMS(timer));

    // If the decodePacket returned an error or the packet length is too large
    if ( len <= 0 || rem_len > MQTT_CLIENT_MAX_MSG_LEN ){
        DBG("Packet Length error.");
        rc = FAILURE_REQUIRES_RESTART; // SP
        goto exit;
    }

    // If the MQTT packet type is not supported
    if ( __cycle_collection[pack_type] == NULL ) {
    	// Read and ignore/dump the packet contents
        DBG("Control Header type is not supported. Dump the packet.");
    	while ( rem_len ) {
            int chunk = rem_len < (int)sizeof(tmp) ? rem_len : (int)sizeof(tmp);
            int r = c->ipstack->mqttread(c->ipstack, tmp, chunk, TimerLeftMS(timer));
            if ( r > 0 ) rem_len -= r;
            else break;
        }
        rc = pack_type;
//        rc = FAILURE; // pack_type;  SP
        goto exit;
    }

    // Pass execution to the Control Header-specific read function.
    header.rem_len = rem_len;
    rc = __cycle_collection[pack_type](c, &header, timer);
    if ( rc == MQTT_SUCCESS ) {
        TimerCountdown(&c->last_received, c->keepAliveInterval);
    }
    else{
        DBG("MQTT Packet Payload failure");
        rc = FAILURE_REQUIRES_RESTART;
        goto exit;
    }

    if (keepalive(c) != MQTT_SUCCESS) {
        //check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
        DBG("Keepalive failure.  ");
        rc = FAILURE;
    }

exit:
    if (rc == MQTT_SUCCESS)
        rc = pack_type;

//    DBG("xxxxxxxxxxx Exit. rc = %d xxxxxxxxxxx", rc);
    return rc;
}

int MQTTPublish_part(MQTTClient* c,
                     const char* topicName,
                     MQTTMessage* message,
                     mqtt_payload_drive_t *drive) {
    int rc = FAILURE;
    TimerInterval timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    extern int MQTTSerialize_publishLength(int qos, MQTTString topicName, int payloadlen);

#if defined(MQTT_TASK)
      MutexLock(&c->mutex);
#endif
      if (!c->isconnected) {
            goto exit;
      }

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if (message->dup == MQTT_ORIGINAL) {
    	//Sending the first attempt. Get next id if needed.
        if (message->qos == QOS1 || message->qos == QOS2) {
            message->id = getNextPacketId(c);
        }
    }

    int payloadlen = drive->init(drive->data);
    if ( payloadlen <= 0 ) {
        goto exit;
    }
    int rem_len = MQTTSerialize_publishLength(message->qos, topic, payloadlen);

    unsigned char *ptr = c->buf;

    // send header
    MQTTHeader header;
    header.bits.type = PUBLISH;
    header.bits.dup = message->dup;
    header.bits.qos = message->qos;
    header.bits.retain = message->retained;
    writeChar(&ptr, header.byte);

    ptr += MQTTPacket_encode(ptr, rem_len);

    writeMQTTString(&ptr, topic);

    if (message->qos > 0) {
        writeInt(&ptr, message->id);
    }

    int len = ptr - c->buf;

    if ((rc = sendPacket(c, c->buf, len, &timer)) != MQTT_SUCCESS) { // send the subscribe packet
        goto exit; // there was a problem
    }

    ptr = c->buf;
    // send packet body
    int total = payloadlen;
    len = c->buf_size;
    while( total && !TimerIsExpired(&timer) ) {
        int chunk = total < len ? total : len;
        chunk = drive->part(drive->data, (char*)ptr, chunk);
        if ( chunk <= 0 ) {
//        	DBG("chunk failure.")
            rc = FAILURE;
            goto exit;
        }
        if ((rc = sendPacket(c, c->buf, chunk, &timer)) != MQTT_SUCCESS){ // send the subscribe packet
        	DBG("sendPacket failure.")
            goto exit;
        }
        total -= chunk;
    }
    if ( !total ) {
        drive->fin(drive->data);
        rc = MQTT_SUCCESS;
    } else {
//    	DBG("total failure.")
        rc = FAILURE;
        goto exit;
    }

exit:
    return rc;
}

int konexios_mqtt_client_subscribe(MQTTClient *c,
                                enum QoS qos,
                                konexios_mqtt_delivery_callback_t *cb) {
    MQTTSubackData subdata;
    int rc = FAILURE;
    TimerInterval timer;
    int len = 0;
    unsigned char *ptr = c->buf;

    if (!c->isconnected) {
      goto exit;
    }

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    MQTTHeader header = {0};
    int rem_len = 5 + property_size(&cb->topic);
    header.byte = 0;
    header.bits.type = SUBSCRIBE;
    header.bits.dup = 0;
    header.bits.qos = 1;
    writeChar(&ptr, header.byte); /* write header */
    ptr += MQTTPacket_encode(ptr, rem_len);
    writeInt(&ptr, getNextPacketId(c));

    writeCString(&ptr, P_VALUE(cb->topic));
    writeChar(&ptr, qos);

    len = ptr - c->buf;
    if (len <= 0) {
      goto exit;
    }
    if ((rc = sendPacket(c, c->buf, len, &timer)) != MQTT_SUCCESS) { // send the subscribe packet
      goto exit;  // there was a problem
    }

    if (waitfor(c, SUBACK, &timer) == SUBACK) {    // wait for suback
      int count = 0;
      unsigned short mypacketid;
      subdata.grantedQoS = QOS0;
      if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int *)&subdata.grantedQoS, c->readbuf, c->readbuf_size) == 1) {
        if (subdata.grantedQoS != 0x80) {
          rc = konexios_mqtt_client_delivery_message_reg(c, cb);
        }
      }
    } else {
      rc = FAILURE;
    }
  exit:
    return rc;
}
