/**
*  @file        transaction.h
*  @version	$Revision: 1.0 $
*  @date	$Date: 17/04/2019 $
*  @author 	Keith Fowler
*  @brief 	mqtt round-trip request - response support
*
*  @copyright	Copyright (C) 2019
* 		\n Petnet Inc.
* 		\n All Rights Reserved
*
*/

#ifndef TRANSACTION_H
#define TRANSACTION_H

// @section include file references
//#include <json/models.h>
#include <konexios/mqtt_out_msg.h>

// @section enumerations
#define MQTT_MAX_RETRIES    2       // maximum no. of retries
#define MQTT_ACK_WAIT       10000   // maximum wait time (in ms) for response


// @section structures


// @section function prototypes
void transaction_init(void);
void transaction_start(out_msg_t *msg, int mqttType, int qos);
int transaction_pending(void);
int transaction_match(int packetId, int packetType);


#endif //TRANSACTION_H
