/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *   Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *   Ian Craggs - fix for #96 - check rem_len in readPacket
 *   Ian Craggs - add ability to set message handler separately #6
 *******************************************************************************/
#include "MQTTClient.h"
#include "arrow/mqtt.h"

// This is where out_msg_t is defined
#include <arrow/mqtt_out_msg.h>

#if defined(__arm__)
//#include "trace.h" //factory ota failure debug
#define trace(...)
#endif

#if defined(__USE_STD__)
  #include <stdio.h>
  #include <string.h>
#endif

extern int cycle_r(MQTTClient* c, TimerInterval* timer);
extern mqtt_env_t *get_telemetry_env();

static void NewMessageData(MessageData *md, MQTTString *aTopicName, MQTTMessage *aMessage) {
  md->topicName = aTopicName;
  md->message = aMessage;
}


int getNextPacketId(MQTTClient *c) {
  return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}


/**
 * @brief
 * @b Purpose:	Send an outbound message
 *
 * @param[in]	c: pointer to MQTTClient with write-to destination set
 * @param[in]	buf: message buffer
 * @param[in]	length: length of message
 * @param[in]	timer:	pointer to timer governing max. transmission time allowed
 *
 * @return MQTT_SUCCESS on successful transmission, else FAILURE
 */
int sendPacket(MQTTClient *c, unsigned char* buf, int length, TimerInterval *timer) {
  int rc = FAILURE,
      sent = 0;

  while (sent < length && !TimerIsExpired(timer)) {
    rc = c->ipstack->mqttwrite(c->ipstack, &buf[sent], length, TimerLeftMS(timer));
    if (rc <= 0) { // there was an error writing the data
        rc = FAILURE;
        break;
    }
    sent += rc;
  }
  if (sent == length) {
    TimerCountdown(&c->last_sent, c->keepAliveInterval); // record the fact that we have successfully sent the packet
    rc = MQTT_SUCCESS;
  } else {
    rc = FAILURE;
  }
  return rc;
}


void MQTTClientInit(MQTTClient *c, Network *network, unsigned int command_timeout_ms,
                    unsigned char *sendbuf, size_t sendbuf_size, unsigned char *readbuf, size_t readbuf_size) {
  int i;
  c->ipstack = network;

  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    c->messageHandlers[i].topicFilter = 0;
  }
  c->command_timeout_ms = command_timeout_ms;
  c->buf = sendbuf;
  c->buf_size = sendbuf_size;
  c->readbuf = readbuf;
  c->readbuf_size = readbuf_size;
  c->isconnected = 0;
  c->cleansession = 0;
  c->reject = 0;
  c->ping_outstanding = 0;
  c->delivery_cb = 0;
  c->defaultMessageHandler = NULL;
  c->queueHandler = NULL;
  c->next_packetid = 1;
  TimerInit(&c->last_sent);
  TimerInit(&c->last_received);
#if defined(MQTT_TASK)
  MutexInit(&c->mutex);
#endif
}

/**
 * @brief
 * @b Purpose:	Send an MQTT Ack packet on the wire
 *
 * @param	msg: pointer to an outbound message with a properly built MQTT ack in the buffer
 *
 * @return MQTT_SUCCESS on successful transmission, else FAILURE
 */
int MQTTSendAck(out_msg_t *out_msg)
{
    mqtt_env_t *tmp;
	TimerInterval timer;
	int rc = FAILURE;

    if( mqtt_is_telemetry_connect() ) {
        tmp = get_telemetry_env();
        if (tmp) {
     	    TimerInit(&timer);
     	    TimerCountdownMS(&timer, tmp->client.command_timeout_ms);

     		rc = sendPacket(&tmp->client, out_msg->mqtt_ack.resp, out_msg->mqtt_ack.len, &timer); // send the packet
        }
    }

    return rc;
}

/**
 * @brief
 * @b Purpose:	Inspect a received MQTT packet to determine its length.
 *
 * @param[in]	c: pointer to an MQTT client, info for receive buffer and channel.
 * @param[out]  value: Length in bytes of inbound packet. 0 on failure.
 * @param[in]	timeout:  maximum time in msec to wait while reading incoming stream.
 *
 * @return number of bytes read on inbound packet to determine length, else 0 on failure.
 */
int decodePacketLength(MQTTClient *c, int *value, int timeout) {
  unsigned char i = 0x0;
  int multiplier = 1;
  int len = 0;
  const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

  *value = 0;
  do {
    int rc = MQTTPACKET_READ_ERROR;
    rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
//    trace(TRACE_DEBUG, "read bytes:%d", rc); //gs
    if (rc != 1) {
 //     trace(TRACE_DEBUG, "exit _read returned: %d", rc); //gs
      *value = 0; //gs
      len = 0; //gs
      goto exit;
    }
    if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
      rc = MQTTPACKET_READ_ERROR; /* bad data */
//      trace(TRACE_DEBUG, "error, len > MAX_NO_OF_REMAINING_LENGTH_BYTES"); //gs
      *value = 0; //gs
      len = 0; //gs
      goto exit;
    }
    *value += (i & 127) * multiplier;
    multiplier *= 128;
  } while ((i & 128) != 0);
exit:
  return len;
}


static int readPacket(MQTTClient *c, TimerInterval *timer) {
  MQTTHeader header = {0};
  int len = 0;
  int rem_len = 0;

  /* 1. read the header byte.  This has the packet type in it */
  int rc = c->ipstack->mqttread(c->ipstack, c->readbuf, 1, TimerLeftMS(timer));
  if (rc != 1) {
    goto exit;
  }

  len = 1;
  /* 2. read the remaining length.  This is variable in itself */
  decodePacketLength(c, &rem_len, TimerLeftMS(timer));
  if ( rem_len <= 0 ) {
    // didn't receive decode byte
    rc = 0;
    goto exit;
  }
  len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */
  if ((size_t)rem_len > (c->readbuf_size - len)) {
    while ( rem_len ) {
      int chunk = rem_len < ((int)c->readbuf_size - len - 1) ? rem_len : ((int)c->readbuf_size - len - 1);
      int r = c->ipstack->mqttread(c->ipstack, c->readbuf + len, chunk, TimerLeftMS(timer));
      if ( r > 0 ) {
        rem_len -= r;
      } else {
        break;
      }
    }
    rc = BUFFER_OVERFLOW;
    goto exit;
  }

  /* 3. read the rest of the buffer using a callback to supply the rest of the data */
  if (rem_len > 0 && (rc = c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, TimerLeftMS(timer)) != rem_len)) {
    rc = 0;
    goto exit;
  }

  header.byte = c->readbuf[0];
  rc = header.bits.type;
  if (c->keepAliveInterval > 0) {
    TimerCountdown(&c->last_received, c->keepAliveInterval);  // record the fact that we have successfully received a packet
  }
exit:
  return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char *topicFilter, MQTTString *topicName) {
  char *curf = topicFilter;
  char *curn = topicName->lenstring.data;
  char *curn_end = curn + topicName->lenstring.len;

  while (*curf && curn < curn_end) {
    if (*curn == '/' && *curf != '/') {
      break;
    }
    if (*curf != '+' && *curf != '#' && *curf != *curn) {
      break;
    }
    if (*curf == '+') {
      // skip until we meet the next separator, or end of string
      char *nextpos = curn + 1;
      while (nextpos < curn_end && *nextpos != '/') {
        nextpos = ++curn + 1;
      }
    } else if (*curf == '#') {
      curn = curn_end - 1;  // skip until end of string
    }
    curf++;
    curn++;
  };

  return (curn == curn_end) && (*curf == '\0');
}


int deliverMessage(MQTTClient *c, MQTTString *topicName, MQTTMessage *message) {
  int i;
  int rc = FAILURE;

  // we have to find the right message handler - indexed by topic
  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char *)c->messageHandlers[i].topicFilter) ||
        isTopicMatched((char *)c->messageHandlers[i].topicFilter, topicName))) {
      if (c->messageHandlers[i].fp != NULL) {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->messageHandlers[i].fp(&md);
        rc = MQTT_SUCCESS;
      }
    }
  }

  if (rc == FAILURE && c->defaultMessageHandler != NULL) {
    MessageData md;
    NewMessageData(&md, topicName, message);
    c->defaultMessageHandler(&md);
    rc = MQTT_SUCCESS;
  }

  return rc;
}

/**
 * @brief
 * @b Purpose:	Build up an MQTT acknowledge packet.
 *
 * @param[in]	c: pointer to an MQTT client, info for transmission buffer and channel.
 * @param[in]	msg: pointer to message for packet id.
 * @param[in]	packet_type: the packet type to acknowledge.
 * @param[in]	timer: pointer to a delivery interval timer.
 *
 * @return FAILURE if Ack could not be sent, else MQTT_SUCCESS.
 */
int MQTTBuildAck(MQTTClient *c, MQTTMessage *msg, int packet_type, TimerInterval *timer) {
   int len=0;
   int rc = MQTT_SUCCESS;
   int acktype=0;
   out_msg_t out_msg;

   switch (packet_type) {
     default:
        break;
     case CONNECT:
        // if we are server?
       acktype = CONNACK;
       break;
     case CONNACK:
     case PUBACK:
     case SUBACK:
     case UNSUBACK:
     case PUBCOMP:
     case PINGRESP:
        break;
     case PUBLISH:
        if (msg->qos == QOS1) {
        	acktype = PUBACK;
    	}
    	else if (msg->qos == QOS2) {
    		acktype = PUBREC;
    	}
    	else {
    		// nothing to send
     	}
       break;
     case PUBREC:
    	 acktype = PUBREL;
    	 break;
     case PUBREL:
        acktype = PUBCOMP;
        break;
     case SUBSCRIBE:
         acktype = SUBACK;
         break;
     case UNSUBSCRIBE:
         acktype = UNSUBACK;
         break;
     case PINGREQ:
        acktype = PINGRESP;
        break;
   }

   if (acktype != 0) {
      out_msg.type = OUT_MQTT_ACK;
      len = MQTTSerialize_ack(out_msg.mqtt_ack.resp, MQTT_ACK_LENGTH, acktype, 0, msg->id);

      if (len <= 0) {
         rc = FAILURE;
      } else {
         out_msg.mqtt_ack.len = len;
         if (c->queueHandler != NULL) {
        	 // send Acks with high priority
            c->queueHandler(&out_msg, 1);
         } else {
            // fall-back to send immediate if queue function is not set.
            rc = sendPacket(c, out_msg.mqtt_ack.resp, out_msg.mqtt_ack.len, timer);
         }
         
         trace(TRACE_DEBUG,"sent ACK type: %d", acktype);
      }
   }

   return rc;
}

/**
 * @brief
 * @b Purpose:	Check if we need to signal client on-line.
 * 				Send a ping message as needed.
 *
 * @param[in]	c: pointer to an MQTT client, info for transmission buffer and channel.
 *
 * @return FAILURE if Ping response not received, else MQTT_SUCCESS.
 */
int keepalive(MQTTClient *c) {
  int rc = MQTT_SUCCESS;

  if (c->keepAliveInterval == 0) {
    goto exit;
  }

  if (TimerIsExpired(&c->last_sent) || TimerIsExpired(&c->last_received)) {
    if (c->ping_outstanding) {
      rc = FAILURE;  /* PINGRESP not received in keepalive interval */
    } else {
    	MQTTDeliverPing(c);
    }
  }

exit:
  return rc;
}


/**
 * @brief
 * @b Purpose:	Build and send an MQTT ping message
 *
 * @param[in]	c: pointer to an MQTT client, info for transmission buffer and channel.
 *
 * @return FAILURE if Ping could not be sent, else MQTT_SUCCESS.
 */
int MQTTDeliverPing(MQTTClient *c) {
   int rc = MQTT_SUCCESS;
   out_msg_t out_msg;

   out_msg.mqtt_ack.len = MQTTSerialize_pingreq(out_msg.mqtt_ack.resp, MQTT_ACK_LENGTH);
   if (out_msg.mqtt_ack.len > 0) {
      if (c->queueHandler != NULL) {
         c->queueHandler(&out_msg, 0);
      } else {
         TimerInterval timer;

         // fall-back to send immediate if queue function is not set.
         TimerInit(&timer);
         TimerCountdownMS(&timer, 1000);
         rc = sendPacket(c, out_msg.mqtt_ack.resp, out_msg.mqtt_ack.len, &timer);
      }

      c->ping_outstanding = 1;
   } else {
      rc = FAILURE;
   }

   return rc;
}


void MQTTCleanSession(MQTTClient *c) {
  int i = 0;

  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    c->messageHandlers[i].topicFilter = NULL;
  }
}


void MQTTCloseSession(MQTTClient *c) {
  c->ping_outstanding = 0;
  c->isconnected = 0;
  if (c->cleansession) {
    MQTTCleanSession(c);
  }
}

int cycle(MQTTClient *c, TimerInterval *timer) {
  int rc = MQTT_SUCCESS;
  MQTTMessage msg;

  int packet_type = readPacket(c, timer);     /* read the socket, see what work is due */
  switch (packet_type) {
    default:
      /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
      rc = packet_type;
      goto exit;
    case 0: /* timed out reading packet */
      break;
    case CONNACK:
    case PUBACK:
    case SUBACK:
    case UNSUBACK:
      break;
    case PUBLISH: {
      MQTTString topicName;
      int intQoS;
      msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
      if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
                                  (unsigned char **)&msg.payload, (int *)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1) {
        goto exit;
      }
      msg.qos = (enum QoS)intQoS;
      deliverMessage(c, &topicName, &msg);

      rc = MQTTBuildAck(c, &msg, packet_type, timer);
      if (rc == FAILURE) {
        goto exit;  // there was a problem
      }
      break;
    }
    case PUBREC:
    case PUBREL: {
      unsigned short mypacketid;
      unsigned char dup, type;
      int result;

      result = MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size);
      if (result != 1) {
    	  rc = FAILURE;
    	  goto exit;
      }

      msg.id = mypacketid;
      result = MQTTBuildAck(c, &msg, packet_type, timer);
      if (result != MQTT_SUCCESS) {
    	  rc = FAILURE;
    	  goto exit;
      }
      break;
    }

    case PUBCOMP:
      break;
    case PINGRESP:
      c->ping_outstanding = 0;
      break;
  }

  if (keepalive(c) != MQTT_SUCCESS) {
    //check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
    rc = FAILURE;
  }

exit:
  if (rc == MQTT_SUCCESS) {
    rc = packet_type;
  }
  // WORKAROUND We don't want to close connection if there are no messages
//    else if (c->isconnected)
//        MQTTCloseSession(c);
  return rc;
}


int MQTTYield(MQTTClient* c, int timeout_ms)
{
    int rc = MQTT_SUCCESS;
    TimerInterval timer;

  TimerInit(&timer);
  TimerCountdownMS(&timer, timeout_ms);

  do {
    if ( (rc = cycle_r(c, &timer)) < 0) {
      rc = FAILURE;
      break;
    } else {
      return rc;
    }
  } while (!TimerIsExpired(&timer));

  return rc;
}


void MQTTRun(void *parm) {
  TimerInterval timer;
  MQTTClient *c = (MQTTClient *)parm;

  TimerInit(&timer);

  while (1) {
#if defined(MQTT_TASK)
    MutexLock(&c->mutex);
#endif
    TimerCountdownMS(&timer, 500); /* Don't wait too long if no traffic is incoming */
    cycle(c, &timer);
#if defined(MQTT_TASK)
    MutexUnlock(&c->mutex);
#endif
  }
}


#if defined(MQTT_TASK)
int MQTTStartTask(MQTTClient *client) {
  return ThreadStart(&client->thread, &MQTTRun, client);
}
#endif

int waitfor(MQTTClient *c, int packet_type, TimerInterval *timer) {
  int rc = FAILURE;

  do {
    if (TimerIsExpired(timer)) {
      break;  // we timed out
    }
    rc = cycle_r(c, timer);
  } while (rc != packet_type && rc >= 0);

  return rc;
}

int MQTTConnectWithResults(MQTTClient *c, MQTTPacket_connectData *options, MQTTConnackData *data) {
  TimerInterval connect_timer;
  int rc = FAILURE;
  MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
  int len = 0;

#if defined(MQTT_TASK)
  MutexLock(&c->mutex);
#endif
  if (c->isconnected) { /* don't send connect packet again if we are already connected */
    goto exit;
  }

  TimerInit(&connect_timer);
  TimerCountdownMS(&connect_timer, c->command_timeout_ms);

  if (options == 0) {
    options = &default_options;  /* set default options if none were supplied */
  }

  c->keepAliveInterval = options->keepAliveInterval;
  c->cleansession = options->cleansession;
  TimerCountdown(&c->last_received, c->keepAliveInterval);
  if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0) {
    goto exit;
  }
  if ((rc = sendPacket(c, c->buf, len, &connect_timer)) != MQTT_SUCCESS) { // send the connect packet
    goto exit;  // there was a problem
  }

  // this will be a blocking call, wait for the connack
  if (waitfor(c, CONNACK, &connect_timer) == CONNACK) {
    data->rc = 0;
    data->sessionPresent = 0;
    if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf, c->readbuf_size) == 1) {
      rc = data->rc;
    } else {
      rc = FAILURE;
    }
  } else {
    rc = FAILURE;
  }

exit:
  if (rc == MQTT_SUCCESS) {
    c->isconnected = 1;
    c->ping_outstanding = 0;
  }

#if defined(MQTT_TASK)
  MutexUnlock(&c->mutex);
#endif

  return rc;
}


int MQTTConnect(MQTTClient *c, MQTTPacket_connectData *options) {
  MQTTConnackData data;
  return MQTTConnectWithResults(c, options, &data);
}


int MQTTSetMessageHandler(MQTTClient *c, const char *topicFilter, messageHandler messageHandler) {
  int rc = FAILURE;
  int i = -1;

  /* first check for an existing matching slot */
  for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
    if (c->messageHandlers[i].topicFilter != NULL && strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0) {
      if (messageHandler == NULL) { /* remove existing */
        c->messageHandlers[i].topicFilter = NULL;
        c->messageHandlers[i].fp = NULL;
      }
      rc = MQTT_SUCCESS; /* return i when adding new subscription */
      break;
    }
  }
  /* if no existing, look for empty slot (unless we are removing) */
  if (messageHandler != NULL) {
    if (rc == FAILURE) {
      for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if (c->messageHandlers[i].topicFilter == NULL) {
          rc = MQTT_SUCCESS;
          break;
        }
      }
    }
    if (i < MAX_MESSAGE_HANDLERS) {
      c->messageHandlers[i].topicFilter = topicFilter;
      c->messageHandlers[i].fp = messageHandler;
    }
  }
  return rc;
}


int MQTTSubscribeWithResults(MQTTClient *c, const char *topicFilter, enum QoS qos,
                             messageHandler messageHandler, MQTTSubackData *data) {
  int rc = FAILURE;
  TimerInterval timer;
  int len = 0;
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicFilter;

#if defined(MQTT_TASK)
  MutexLock(&c->mutex);
#endif
  if (!c->isconnected) {
    goto exit;
  }

  TimerInit(&timer);
  TimerCountdownMS(&timer, c->command_timeout_ms);

  len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int *)&qos);
  if (len <= 0) {
    goto exit;
  }
  if ((rc = sendPacket(c, c->buf, len, &timer)) != MQTT_SUCCESS) { // send the subscribe packet
    goto exit;  // there was a problem
  }

  if (waitfor(c, SUBACK, &timer) == SUBACK) {    // wait for suback
    int count = 0;
    unsigned short mypacketid;
    data->grantedQoS = QOS0;
    if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int *)&data->grantedQoS, c->readbuf, c->readbuf_size) == 1) {
      if (data->grantedQoS != 0x80) {
        rc = MQTTSetMessageHandler(c, topicFilter, messageHandler);
      }
    }
  } else {
    rc = FAILURE;
  }

exit:
//    if (rc == FAILURE)
//        MQTTCloseSession(c);
#if defined(MQTT_TASK)
  MutexUnlock(&c->mutex);
#endif
  return rc;
}


int MQTTSubscribe(MQTTClient *c, const char *topicFilter, enum QoS qos,
                  messageHandler messageHandler) {
  MQTTSubackData data;
  return MQTTSubscribeWithResults(c, topicFilter, qos, messageHandler, &data);
}


int MQTTUnsubscribe(MQTTClient *c, const char *topicFilter) {
  int rc = FAILURE;
  TimerInterval timer;
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicFilter;
  int len = 0;

#if defined(MQTT_TASK)
  MutexLock(&c->mutex);
#endif
  if (!c->isconnected) {
    goto exit;
  }

  TimerInit(&timer);
  TimerCountdownMS(&timer, c->command_timeout_ms);

  if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0) {
    goto exit;
  }
  if ((rc = sendPacket(c, c->buf, len, &timer)) != MQTT_SUCCESS) { // send the subscribe packet
    goto exit;  // there was a problem
  }

  if (waitfor(c, UNSUBACK, &timer) == UNSUBACK) {
    unsigned short mypacketid;  // should be the same as the packetid above
    if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1) {
      /* remove the subscription message handler associated with this topic, if there is one */
      MQTTSetMessageHandler(c, topicFilter, NULL);
    }
  } else {
    rc = FAILURE;
  }

exit:
  if (rc == FAILURE) {
    MQTTCloseSession(c);
  }
#if defined(MQTT_TASK)
  MutexUnlock(&c->mutex);
#endif
  return rc;
}

int MQTTPublish(MQTTClient *c, const char *topicName, MQTTMessage *message) {
  int rc = FAILURE;
  TimerInterval timer;
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicName;
  int len = 0;

#if defined(MQTT_TASK)
  MutexLock(&c->mutex);
#endif
  if (!c->isconnected) {
    goto exit;
  }

  TimerInit(&timer);
  TimerCountdownMS(&timer, c->command_timeout_ms);

  if (message->qos == QOS1 || message->qos == QOS2) {
    message->id = getNextPacketId(c);
  }

  len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
                              topic, (unsigned char *)message->payload, message->payloadlen);
  if (len <= 0) {
    goto exit;
  }
  if ((rc = sendPacket(c, c->buf, len, &timer)) != MQTT_SUCCESS) { // send the subscribe packet
    goto exit;  // there was a problem
  }

exit:
  if (rc == FAILURE) {
    MQTTCloseSession(c);
  }
#if defined(MQTT_TASK)
  MutexUnlock(&c->mutex);
#endif
  return rc;
}


/**
 * @brief
 * @b Purpose:	Build and send an MQTT disconnect message
 *
 * @param[in]	c: pointer to an MQTT client, info for transmission buffer and channel.
 *
 * @return FAILURE if Disconnect could not be sent, else MQTT_SUCCESS.
 */
int MQTTDisconnect(MQTTClient *c) {
  int rc = FAILURE;
  TimerInterval timer;     // we might wait for incomplete incoming publishes to complete
  int len = 0;
  unsigned char buf[MQTT_DISCONNECT_LENGTH];

#if defined(MQTT_TASK)
  MutexLock(&c->mutex);
#endif
  TimerInit(&timer);
  TimerCountdownMS(&timer, c->command_timeout_ms);

  len = MQTTSerialize_disconnect(buf, MQTT_DISCONNECT_LENGTH);
  if (len > 0) {
    rc = sendPacket(c, buf, len, &timer);  // send the disconnect packet
  }
  MQTTCloseSession(c);

#if defined(MQTT_TASK)
  MutexUnlock(&c->mutex);
#endif
  return rc;
}

DLLExport int MQTTIsConnected(MQTTClient *client) {
  return client->isconnected;
}

/**
 * @brief
 * @b Purpose:	Register an external function to queue MQTT responses
 * \n           (generally, a means of queueing ACKs generated by the SDK)
 *
 * @param[in]	queueHandler: pointer to application function to queue outbound messages
 *
 * @return Nil.
 */
void MQTTSetQueueCallback(void (*queueHandler))
{
    mqtt_env_t *tmp;

    if( mqtt_is_telemetry_connect() ) {
        tmp = get_telemetry_env();
        if (tmp) {
        	tmp->client.queueHandler = queueHandler;
        }
    }
}

