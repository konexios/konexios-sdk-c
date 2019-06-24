/**
*  @file        transaction.c
*  @version	$Revision: 1.0 $
*  @date	$Date: 17/04/2019 $
*  @author 	Keith Fowler
*  @brief 	Transaction message manager for tracking round trip MQTT request - response
*  \n       and handling timeouts, retries and failures.
*  \n       Currently this only supports a single pending transaction. Multiple transactions
*  \n       would require a list of timers and activeMessages to track each round trip.
*
*  @copyright	Copyright (C) 2019
* 		\n Petnet Inc.
* 		\n All Rights Reserved
*
*/
#include <stdbool.h>
#include <stdint.h>
#include <arrow/mqtt_out_msg.h>

#include "transaction.h"
#include "MQTTClient.h"
#include "arrow/mqtt.h"
//#include "trace.h"
#include "debug.h"

#define trace(...)

#ifndef pdFALSE
#define pdFALSE     false
#endif
#ifndef pdTRUE
#define pdTRUE      true
#endif

#ifdef FREERTOS_TIMERS
static TimerHandle_t    ack_handle = NULL;
#endif
static out_msg_t 	    activeMsg;
static int              pendingType = RESERVED;

/**
 * @brief
 * @b Purpose:	Handle the timeout of the transaction timer.
 * \n           This will be called if we have not received a match for the pending transaction
 * \n           within MQTT_ACK_WAIT msec.
 * \n           Check both the retry count and the pending MQTT packet type to validate
 * \n           that a resend (to the back of the outbound queue) should be attempted.
 *
 * @param[in]	arg: not used.
 *
 * @return void
 */
void transaction_timeout(void* arg) {

	if ( (activeMsg.retryCount++ < MQTT_MAX_RETRIES) && pendingType != RESERVED) {
		mqtt_env_t *tmp;

		if( mqtt_is_telemetry_connect() ) {
			tmp = get_telemetry_env();
			if ( (tmp != NULL) && (tmp->client.queueHandler != NULL) ) {
				// re-send message to back of queue
				tmp->client.queueHandler(&activeMsg, 0);
			}
		}
	}
	else {
		pendingType = RESERVED;
		trace(TRACE_CRITICAL, "No Ack to message type: %d", activeMsg.type);
	}

	DBG("Transaction Timeout Id: %d\n", activeMsg.packetId);
}

/**
 * @brief
 * @b Purpose:	Initialize the (single) MQTT transaction timer.
 *
 * @return void
 */
void transaction_init(void) {
#ifdef FREERTOS_TIMERS
    if (ack_handle == NULL) {
	    TickType_t period = pdMS_TO_TICKS(MQTT_ACK_WAIT);
	    ack_handle = xTimerCreate("ackTimer", period, pdFalse, NULL, transaction_timeout);

    }
    else {
		xTimerStop(ack_handle, 10);
    }
#endif
	pendingType = RESERVED;
}

/**
 * @brief
 * @b Purpose:	Setup a new MQTT transaction. Typically called when a NEW (not duplicate)
 * \n           MQTT message is sent and we are expecting an ack or response of some sort.
 * \n           The transaction timer will be started, which will expire if a matching Ack
 * \n           is not received within MQTT_ACK_WAIT msec.
 *
 * @param[in]	msg: the encoded message which was sent
 * @param[in]	mqttType: packet type in range of CONNECT ... DISCONNECT
 * @param[in]	qos:  packet quality of service, QOS0 thru QOS2
 *
 * @return void
 */
void transaction_start(out_msg_t *msg, int mqttType, int qos) {
	if (qos > QOS0) {
		activeMsg = *msg;
		pendingType = mqttType;
#ifdef FREERTOS_TIMERS
		if (xTimerReset(ack_handle, pdMS_TO_TICKS(MQTT_ACK_WAIT)) != pdTRUE)
		{
			DBG("Couldn't start transaction timer\n");
		}
#endif

	}
}

/**
 * @brief
 * @b Purpose:	Check if there is an active transaction in progress.
 *
 * @return TRUE if the transaction timer is running, else FALSE.
 */
int transaction_pending(void) {
#ifdef FREERTOS_TIMERS
	return xTimerIsTimerActive(ack_handle);
#else
	return false;
#endif
}

/**
 * @brief
 * @b Purpose:	Check the received packet details against the currently active transaction.
 *
 * @param[in]	packetId: the decoded (on the wire) MQTT packet Id.
 * @param[in]	packetType: the decoded (on the wire) MQTT packet type.

 * @return TRUE if the parameters match the current pending transaction, else FALSE
 */
int transaction_match(int packetId, int packetType) {
	if (packetId == activeMsg.packetId) {
#ifdef FREERTOS_TIMERS
		// The parameters match, so cancel and clear the active transaction.
		if (xTimerStop(ack_handle, 10) != pdTRUE)
		{
			DBG("Couldn't stop transition timer\n");
		}
#endif

		pendingType = RESERVED;
		return pdTRUE;
	}
	else {
		DBG("Mismatch Id: %d\n", packetId);
	}

	return pdFALSE;
}
